#include "DefeatCombatManager.h"

namespace SexLabDefeat {

    DefeatCombatManager::DefeatCombatManager(IDefeatActorManager* defeatActorManager, IDefeatManager* defeatManager) {
        _defeatActorManager = defeatActorManager;
        _defeatManager = defeatManager;
    }

    DefeatCombatManager::~DefeatCombatManager() {
    }

    void DefeatCombatManager::onActorEnteredToCombatState(RE::Actor* target_actor) {
        auto defActor = _defeatActorManager->getDefeatActor(target_actor);
        defActor->requestExtraData(target_actor, [&] {}, 10s);
    }

    void DefeatCombatManager::onHitHandler(RawHitEvent event) {
        auto target_actor = event.target->As<RE::Actor>();
        if (target_actor != nullptr) {
            auto player = _defeatActorManager->getPlayer(target_actor);
            if (player->isSame(target_actor)) {
                onPlayerHitHandler(event, player);
            }
        }
    }

    void DefeatCombatManager::onPlayerHitHandler(RawHitEvent event, DefeatPlayerActorType targetActor) {
        SKSE::log::trace("onPlayerHitHandler");
        if (targetActor->hasHitImmunity()) {
            SKSE::log::trace("onPlayerHitHandler Hit Immunity - skipped");
            return;
        }
        if (targetActor->getState() != DefeatActorStates::ACTIVE) {
            return;
        }
        if (event.aggressor == nullptr) {
            return;
        }

        auto aggr_actor = event.aggressor->As<RE::Actor>();
        auto target_actor = event.target->As<RE::Actor>();

        if (aggr_actor && _defeatActorManager->validForAggressorRole(aggr_actor) &&
            _defeatActorManager->validPlayerForVictimRole(target_actor)) {

            auto aggrActor = _defeatActorManager->getDefeatActor(aggr_actor);
            targetActor->setLastHitAggressor(aggrActor);

            if (targetActor->registerAndCheckHitGuard(aggrActor, event.source, event.projectile)) {
                SKSE::log::trace("onPlayerHitHandler Hit from <{:08X}:{}> rejected by Hit Spam Guard",
                                    event.aggressor->GetFormID(), event.aggressor->GetName());
                return;
            }

            auto hitEvent = createHitEvent(targetActor, aggrActor, event);
            auto target_actor_handle = target_actor->GetHandle();
            auto aggr_actor_handle = aggr_actor->GetHandle();

            targetActor->requestExtraData(
                target_actor, [&] {}, 10s);
            aggrActor->requestExtraData(
                aggr_actor, [&] {}, 10s);

            this->calculatePlayerHit(hitEvent);
        }
    }

    void DefeatCombatManager::calculatePlayerHit(HitEventType event) {
        SKSE::log::trace("calculatePlayerHit for <{:08X}> from <{:08X}>", event.target->getTESFormId(),
                         event.aggressor->getTESFormId());
        if (event.target == nullptr || event.aggressor == nullptr) {
            return;
        }
        if (_defeatActorManager->checkAggressor(event.target, event.aggressor)) {
            auto result = KDWay(event);
            if (result != HitResult::SKIP) {
                SKSE::log::trace("calculatePlayerHit: {}", result);
                if (_defeatManager->SoftDependency.LRGPatch) {
                    event.target->resetDynamicDefeat();
                    auto widget = _defeatManager->getWidget();
                    if (widget != nullptr) {
                        if (!widget->stopDynamicWidget()) {
                            SKSE::log::error("Error on stop Dynamic Widget");
                        }
                    }
                }
                event.target->setState(DefeatActorStates::DISACTIVE);

                _defeatActorManager->playerKnockDownEvent(event.target, event.aggressor, result);
            } else {
                // SKSE::log::trace("calculatePlayerHit: SKIP");
            }
        } else {
            SKSE::log::trace("calculatePlayerHit: Aggressor not valid. creature: {}", event.aggressor->isCreature());
        }
    }

    HitResult DefeatCombatManager::KDWay(HitEventType event) {
        if (!event.target->isKDAllowed()) {
            SKSE::log::trace("KDWay - KD Not Allowed");
            return HitResult::SKIP;
        }
        if (_defeatActorManager->getDistanceBetween(event.target, event.aggressor) >
            _defeatManager->getConfig()->KD_FAR_MAX_DISTANCE) {

            SKSE::log::trace("KDWay - Distance is too big");
            return HitResult::SKIP;
        }

        if (_defeatManager->SoftDependency.LRGPatch &&
            _defeatManager->Forms.LRGPatch.DynDefIgnoredWeaponList != nullptr &&
            _defeatManager->Forms.LRGPatch.DynDefIgnoredWeaponList->HasForm(event.source)) {

            SKSE::log::trace("KDWay - DynDef Ignored Weapon List");
            return HitResult::SKIP;
        }

        auto mcmConfig = _defeatManager->getConfig();
        auto result = HitResult::SKIP;

        if (mcmConfig->Config.KDWayThreshold->get()) {
            if ((result = KDWayWound(event)) != HitResult::SKIP) {
                SKSE::log::info("KDWay - KDWayWound");
                return result;
            }
        }
        if (mcmConfig->Config.KDWayStamina->get()) {
            if ((result = KDWayExhaustion(event)) != HitResult::SKIP) {
                SKSE::log::info("KDWay - KDWayExhaustion");
                return result;
            }
        }
        if (_defeatManager->SoftDependency.LRGPatch && mcmConfig->Config.LRGPatch.KDWayVulnerability->get()) {
            if ((result = KDWayVulnerability(event)) != HitResult::SKIP) {
                SKSE::log::info("KDWay - KDWayVulnerability");
                return result;
            }
        }
        if (_defeatManager->SoftDependency.LRGPatch && mcmConfig->Config.LRGPatch.KDWayDynamic->get()) {
            if ((result = KDWayDynamic(event)) != HitResult::SKIP) {
                SKSE::log::info("KDWay - KDWayDynamic");
                return result;
            }
        }
        if (event.isPowerAttack && mcmConfig->Config.KDWayPowerAtk->get()) {
            if ((result = KDWayPowerAtk(event)) != HitResult::SKIP) {
                SKSE::log::info("KDWay - KDWayPowerAtk");
                return result;
            }
        }

        return HitResult::SKIP;
    }

    HitResult DefeatCombatManager::KDWayWound(HitEventType event) {
        auto mcmConfig = _defeatManager->getConfig();
        if (event.isHitBlocked && mcmConfig->Config.KDHealthBlock->get()) {
            return HitResult::SKIP;
        }
        const auto health = _defeatActorManager->getActorValuePercentage(event.target, RE::ActorValue::kHealth) * 100;

        if (randomChanse(mcmConfig->Config.ChanceOnHitPvic->get()) &&
            (health <= mcmConfig->Config.ThresholdPvic->get()) &&
            (health >= mcmConfig->Config.ThresholdPvicMin->get())) {
            SKSE::log::trace("KDWayExhaustion - Health {}% ", static_cast<int>(health));

            HitResult result = HitResult::KNOCKDOWN;
            if (randomChanse(mcmConfig->Config.KnockOutHPvic->get()) &&
                !event.target->isTied()) {
                result = HitResult::KNOCKOUT;
            } else {
                if (mcmConfig->Config.bResistQTE->get() &&
                    randomChanse(mcmConfig->Config.SStruggleHealthPvic->get())) {
                    result = HitResult::STANDING_STRUGGLE;
                };
            }
            return result;
        }

        return HitResult::SKIP;
    };
    HitResult DefeatCombatManager::KDWayExhaustion(HitEventType event) {
        auto mcmConfig = _defeatManager->getConfig();
        if (event.isHitBlocked && mcmConfig->Config.KDStaminaBlock->get()) {
            return HitResult::SKIP;
        }
        const auto stamina = _defeatActorManager->getActorValuePercentage(event.target, RE::ActorValue::kStamina) * 100;

        SKSE::log::trace("KDWayExhaustion - Heading angle {} ",
                         static_cast<int>(_defeatActorManager->getHeadingAngleBetween(event.target, event.aggressor)));

        if (KDOnlyBack(mcmConfig->Config.KDWayStaminaOB->get(), event)) {
            if (randomChanse(mcmConfig->Config.ChanceOnHitPvicS->get()) &&
                (stamina <= mcmConfig->Config.ExhaustionPvic->get())) {
                SKSE::log::trace("KDWayExhaustion - Stamina {}% ", static_cast<int>(stamina));

                HitResult result = HitResult::KNOCKDOWN;
                if (randomChanse(mcmConfig->Config.KnockOutSPvic->get()) &&
                    !event.target->isTied()) {
                    result = HitResult::KNOCKOUT;
                } else {
                    if (mcmConfig->Config.bResistQTE->get() &&
                        randomChanse(mcmConfig->Config.SStruggleExhaustionPvic->get())) {
                        result = HitResult::STANDING_STRUGGLE;
                    };
                }
                return result;
            }
        }

        return HitResult::SKIP;
    };
    HitResult DefeatCombatManager::KDWayVulnerability(HitEventType event) {
        auto mcmConfig = _defeatManager->getConfig();
        if (event.isHitBlocked && mcmConfig->Config.LRGPatch.KDVulnerabilityBlock->get()) {
            return HitResult::SKIP;
        }

        if (KDOnlyBack(mcmConfig->Config.LRGPatch.KDWayVulnerabilityOB->get(), event)) {

            auto vulnerability = event.target->getVulnerability();

            if (randomChanse(mcmConfig->Config.LRGPatch.ChanceOnHitPvicVulnerability->get()) &&
                (vulnerability >= mcmConfig->Config.LRGPatch.VulnerabilityPvic->get())) {
                SKSE::log::trace("KDWayVulnerability - Vulnerability {}% ", static_cast<int>(vulnerability));

                HitResult result = HitResult::SKIP;
                if (randomChanse(mcmConfig->Config.LRGPatch.KnockOutVulnerabilityPvic->get()) &&
                    !event.target->isTied()) {
                    result = HitResult::KNOCKOUT;
                } else {
                    auto form = _defeatActorManager->getEquippedHitSourceByFormID(event.aggressor, event.source);
                    if (form != nullptr) {
                        auto formType = form->GetFormType();
                        if (formType == RE::FormType::Weapon || formType == RE::FormType::Spell) {
                            if (mcmConfig->Config.bResistQTE->get() &&
                                randomChanse(
                                    mcmConfig->Config.LRGPatch.SStruggleVulnerabilityPvic->get())) {
                                result = HitResult::STANDING_STRUGGLE;
                            };
                        }
                    }
                }
                return result;
            }
        }

        return HitResult::SKIP;
    };
    HitResult DefeatCombatManager::KDWayDynamic(HitEventType event) {
        auto DefeatAmount = KDWayDynamicCalculation(event);
        auto widget = _defeatManager->getWidget();
        auto mcmConfig = _defeatManager->getConfig();

        // event->target->getDynamicDefeatSpinLock()->spinLock();
        event.target->incrementDynamicDefeat(DefeatAmount);
        auto totalDynamicDefeat = event.target->getDynamicDefeat();

        if (widget->getState() != IDefeatWidget::State::DYNAMIC_WIDGET && totalDynamicDefeat > 0) {
            if (!widget->startDynamicWidget()) {
                SKSE::log::error("Error on start Dynamic Widget");
            }
            shedulePlayerDeplateDynamicDefeat();
        }
        if (totalDynamicDefeat >= 1) {
            if (!widget->stopDynamicWidget()) {
                SKSE::log::error("Error on stop Dynamic Widget");
            }
            event.target->resetDynamicDefeat();

            HitResult result = HitResult::KNOCKDOWN;
            if (randomChanse(mcmConfig->Config.LRGPatch.KnockOutDynamicPvic->get()) &&
                !event.target->isTied()) {
                result = HitResult::KNOCKOUT;
            } else {
                if (mcmConfig->Config.bResistQTE->get() &&
                    randomChanse(mcmConfig->Config.LRGPatch.SStruggleDynamicPvic->get())) {
                    result = HitResult::STANDING_STRUGGLE;
                };
            }
            // event->target->getDynamicDefeatSpinLock()->spinUnlock();
            return result;
        } else if (totalDynamicDefeat > 0) {
            if (!widget->setPercent(totalDynamicDefeat)) {
                SKSE::log::error("Error on set percent Dynamic Widget");
            }
            // event->target->getDynamicDefeatSpinLock()->spinUnlock();
        }

        return HitResult::SKIP;
    }

    void DefeatCombatManager::shedulePlayerDeplateDynamicDefeat() {
        auto player = this->getDefeatManager()->getActorManager()->getPlayer();
        if (!player->sheduleDeplateDynamicDefeat()) {
            return;
        }

        std::thread worker([this] {
            SKSE::log::trace("deplateDynamicDefeat thread started");
            this->_playerDeplateDynamicDefeatStopThread = false;
            while (!this->_playerDeplateDynamicDefeatStopThread) {
                std::this_thread::sleep_for(2s);
                SKSE::log::trace("deplateDynamicDefeat thread working...");

                auto re_ui = RE::UI::GetSingleton();
                auto player = this->getDefeatManager()->getActorManager()->getPlayer();
                if (re_ui == nullptr) {
                    SKSE::log::critical("deplateDynamicDefeat: RE::UI nullptr");
                }
                if (this->getDefeatManager()->getGameState() == SexLabDefeat::IDefeatManager::GameState::IN_GAME &&
                    re_ui != nullptr && !re_ui->GameIsPaused() && !this->_playerDeplateDynamicDefeatStopThread) {
                    auto totalDynamicDefeat = player->getDynamicDefeat();
                    if (totalDynamicDefeat > 0) {
                        auto mcmConfig = _defeatManager->getConfig();
                        if (_defeatActorManager->isInCombat(player)) {
                            player->decrementDynamicDefeat(
                                mcmConfig->Config.LRGPatch.DynamicDefeatDepleteOverTime->get() /
                                                            100);
                        } else {
                            player->decrementDynamicDefeat(
                                mcmConfig->Config.LRGPatch.DynamicDefeatDepleteOverTime->get() /
                                                            (100.0 / 5));
                        }
                        totalDynamicDefeat = _defeatManager->getActorManager()->getPlayer()->getDynamicDefeat();
                    }

                    SKSE::GetTaskInterface()->AddUITask([this] {
                        auto player = this->getDefeatManager()->getActorManager()->getPlayer();
                        auto totalDynamicDefeat = player->getDynamicDefeat();
                        auto widget = _defeatManager->getWidget();
                        if (widget != nullptr) {
                            if (totalDynamicDefeat <= 0 && widget->getState() == IDefeatWidget::State::DYNAMIC_WIDGET) {
                                if (!widget->stopDynamicWidget(true)) {
                                    SKSE::log::error("Error on stop Dynamic Widget");
                                }
                            } else if (totalDynamicDefeat != widget->getLastPercent()) {
                                if (!widget->setPercent(totalDynamicDefeat, true)) {
                                    SKSE::log::error("Error on set percent Dynamic Widget");
                                } else {
                                    SKSE::log::trace("PlayerDeplateDynamicDefeat widget deplated to {}%",
                                                     static_cast<int>(totalDynamicDefeat * 100));
                                }
                            }
                        }
                    });
                    if (totalDynamicDefeat <= 0) {
                        SKSE::log::trace(
                            "deplateDynamicDefeat widget fully deplated. PlayerDeplateDynamicDefeat stopped");
                        this->_playerDeplateDynamicDefeatStopThread = true;
                    } else {
                        SKSE::log::trace("PlayerDeplateDynamicDefeat deplated to {}%",
                                         static_cast<int>(totalDynamicDefeat * 100));
                    }
                }
                if (this->_playerDeplateDynamicDefeatStopThread) {
                    player->stopDeplateDynamicDefeat();
                }
            }
            SKSE::log::trace("deplateDynamicDefeat thread stopped");
        });
        worker.detach();
    }
    void DefeatCombatManager::interruptPlayerDeplateDynamicDefeat() {
        this->_playerDeplateDynamicDefeatStopThread = true;
    }

    float DefeatCombatManager::KDWayDynamicCalculation(HitEventType event) {
        auto mcmConfig = _defeatManager->getConfig();

        float DefeatAmount = 0;
        float DefeatBaseDamage = mcmConfig->Config.LRGPatch.DynamicDefeatOnHitBase->get() / 100;
        float DefeatVulnerabilityMult = 0;
        float DefeatPowerAttackMult = 0;
        float DefeatLowStaminaMult = 0;
        float DefeatLowHeathMult = 0;
        float DefeatBackHit = 0;
        float DefeatBlockReduction = 1.0;

        auto form = _defeatActorManager->getEquippedHitSourceByFormID(event.aggressor, event.source);
        RE::FormType formType = RE::FormType::None;
        RE::TESObjectWEAP* weap = nullptr;
        if (form != nullptr) {
            formType = form->GetFormType();
            weap = static_cast<RE::TESObjectWEAP*>(form);
        }
        if (formType == RE::FormType::Weapon && weap != nullptr) {
            auto weapType = weap->GetWeaponType();
            switch (weapType) {
                case RE::WEAPON_TYPE::kHandToHandMelee:
                    break;

                case RE::WEAPON_TYPE::kOneHandSword:
                case RE::WEAPON_TYPE::kOneHandDagger:
                case RE::WEAPON_TYPE::kOneHandAxe:
                case RE::WEAPON_TYPE::kOneHandMace:
                    DefeatBaseDamage = mcmConfig->Config.LRGPatch.DynamicDefeatOnHitOneHand->get() / 100;
                    SKSE::log::trace("CalculateWidget - WeaponType: One-Handed");
                    break;

                case RE::WEAPON_TYPE::kTwoHandSword:
                case RE::WEAPON_TYPE::kTwoHandAxe:
                    DefeatBaseDamage = mcmConfig->Config.LRGPatch.DynamicDefeatOnHitTwoHand->get() / 100;
                    SKSE::log::trace("CalculateWidget - WeaponType: Two-Handed");
                    break;

                case RE::WEAPON_TYPE::kBow:
                case RE::WEAPON_TYPE::kCrossbow:
                    DefeatBaseDamage = mcmConfig->Config.LRGPatch.DynamicDefeatOnHitBow->get() / 100;
                    SKSE::log::trace("CalculateWidget - WeaponType: Bow");
                    break;

                case RE::WEAPON_TYPE::kStaff:
                    break;
            }
        } else if (formType == RE::FormType::Spell) {
            DefeatBaseDamage = mcmConfig->Config.LRGPatch.DynamicDefeatOnHitSpell->get() / 100;
            SKSE::log::trace("CalculateWidget - WeaponType: Spell");
        }

        SKSE::log::trace("CalculateWidget - Vulnerability");
        auto DynamicDefeatVulnerabilityMult = mcmConfig->Config.LRGPatch.DynamicDefeatVulnerabilityMult->get();
        if (DynamicDefeatVulnerabilityMult > 1) {
            auto vulnerability = event.target->getVulnerability();
            DefeatVulnerabilityMult = (vulnerability / 100) * (DynamicDefeatVulnerabilityMult - 1);
        }

        if (event.isPowerAttack) {
            DefeatPowerAttackMult = mcmConfig->Config.LRGPatch.DynamicDefeatPowerAttackMult->get() - 1;
        }

        auto DynamicDefeatLowStaminaMult = mcmConfig->Config.LRGPatch.DynamicDefeatLowStaminaMult->get();
        auto DynamicDefeatLowHealthMult = mcmConfig->Config.LRGPatch.DynamicDefeatLowHealthMult->get();
        auto DynamicDefeatBackHitMult = mcmConfig->Config.LRGPatch.DynamicDefeatBackHitMult->get();
        auto DynamicDefeatBlockReduction = mcmConfig->Config.LRGPatch.DynamicDefeatBlockReduction->get();

        if (DynamicDefeatLowStaminaMult > 1.0) {
            const auto stamina =
                _defeatActorManager->getActorValuePercentage(event.target, RE::ActorValue::kStamina) * 100;
            if (stamina <= mcmConfig->Config.LRGPatch.DynamicDefeatLowStaminaThreshold->get()) {
                DefeatLowStaminaMult = DynamicDefeatLowStaminaMult - 1;
            }
        }

        if (DynamicDefeatLowHealthMult > 1.0) {
            const auto health =
                _defeatActorManager->getActorValuePercentage(event.target, RE::ActorValue::kHealth) * 100;
            if (health <= mcmConfig->Config.LRGPatch.DynamicDefeatLowHealthThreshold->get()) {
                DefeatLowHeathMult = DynamicDefeatLowHealthMult - 1;
            }
        }

        if (DynamicDefeatBackHitMult > 1.0 && KDOnlyBack(true, event)) {
            DefeatBackHit = DynamicDefeatBackHitMult - 1;
        }

        if (DynamicDefeatBlockReduction > 0.0 && event.isHitBlocked) {
            DefeatBlockReduction = (1 - DynamicDefeatBlockReduction / 100);
        }

        DefeatAmount = DefeatBaseDamage *
                       (1 + DefeatVulnerabilityMult + DefeatPowerAttackMult + DefeatLowStaminaMult +
                        DefeatLowHeathMult + DefeatBackHit) *
                       DefeatBlockReduction;

        SKSE::log::info(
            "[Defeat] - CalculateWidget - DefeatAmount: {} BaseDamage: {} Vuln: {}% PowerAtt: {}% LowHeathMult: {}%"
            " LowStamMult: {}% DefeatBackHit: {}% DefeatBlockReduction: {}%",
            static_cast<int>(DefeatAmount * 100), static_cast<int>(DefeatBaseDamage * 100),
            static_cast<int>(DefeatVulnerabilityMult * 100), static_cast<int>(DefeatPowerAttackMult * 100),
            static_cast<int>(DefeatLowHeathMult * 100), static_cast<int>(DefeatLowStaminaMult * 100),
            static_cast<int>(DefeatBackHit * 100), static_cast<int>(DefeatBlockReduction * 100));

        return DefeatAmount;
    };

    HitResult DefeatCombatManager::KDWayPowerAtk(HitEventType event) {
        auto mcmConfig = _defeatManager->getConfig();
        if (event.isHitBlocked && mcmConfig->Config.KDWayPowerAtk->get()) {
            return HitResult::SKIP;
        }

        auto form = _defeatActorManager->getEquippedHitSourceByFormID(event.aggressor, event.source);
        if (form != nullptr && form->GetFormType() == RE::FormType::Weapon) {
            auto weap = static_cast<RE::TESObjectWEAP*>(form);
            auto stagger = weap->GetStagger() * 100;
            if (randomChanse(mcmConfig->Config.KDWayPowerAtkCOH->get()) &&
                (stagger >= mcmConfig->Config.PowerAtkStagger->get())) {
                SKSE::log::trace("KDWayPowerAtk - Stagger {}% ", static_cast<int>(stagger));

                HitResult result = HitResult::SKIP;
                if (randomChanse(mcmConfig->Config.KnockOutPPvic->get()) &&
                    !event.target->isTied()) {
                    result = HitResult::KNOCKOUT;
                } else {
                    if (mcmConfig->Config.bResistQTE->get() &&
                        randomChanse(mcmConfig->Config.SStrugglePowerPvic->get())) {
                        result = HitResult::STANDING_STRUGGLE;
                    };
                }
                return result;
            }
        }

        return HitResult::SKIP;
    }
    bool DefeatCombatManager::KDOnlyBack(bool opt, HitEventType event) {
        if (opt) {
            auto angle = _defeatActorManager->getHeadingAngleBetween(event.target, event.aggressor);
            if (angle < 110.0 && angle > -110.0) {
                return false;
            }
        }
        return true;
    };

    HitEventType DefeatCombatManager::createHitEvent(DefeatActorType target_actor, DefeatActorType aggr_actor,
                                               RawHitEvent rawHitEvent) {
        HitEventType event = {};

        event.target = target_actor;
        event.aggressor = aggr_actor;
        event.source = rawHitEvent.source;
        event.projectile = rawHitEvent.projectile;
        event.isPowerAttack = rawHitEvent.isPowerAttack;
        event.isSneakAttack = rawHitEvent.isSneakAttack;
        event.isBashAttack = rawHitEvent.isBashAttack;
        event.isHitBlocked = rawHitEvent.isHitBlocked;

        return event;
    }
}