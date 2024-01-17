#include "Defeat.h"

namespace SexLabDefeat {
    extern template class DeferredExpiringValue<ActorExtraData>;
    extern template float DefeatConfig::getConfig<float>(std::string configKey, float _def) const;

    DefeatCombatManager::DefeatCombatManager(DefeatActorManager* defeatActorManager,
                                                           DefeatManager* defeatManager) {
        _defeatActorManager = defeatActorManager;
        _defeatManager = defeatManager;
        _hitGuardExpiration = std::chrono::milliseconds(_defeatManager->getConfig()->HIT_SPAM_GUARD_EXPIRATION_MS);
        hitSpamGuardSpinLock = new SpinLock();
        OnSLDefeatPlayerKnockDownEventName = new RE::BSFixedString("OnSLDefeatPlayerKnockDown");
    }

    DefeatCombatManager::~DefeatCombatManager() {
        hitSpamGuardSpinLock->spinLock();
        delete hitSpamGuardSpinLock;
        delete OnSLDefeatPlayerKnockDownEventName;
    }

    void DefeatCombatManager::onHitHandler(RawHitEvent event) {
        auto target_actor = event.target->As<RE::Actor>();
        if (target_actor && _defeatActorManager->getPlayer() != nullptr &&
            (_defeatActorManager->getPlayer()->isSame(target_actor))) {
            onPlayerHitHandler(event, _defeatActorManager->getPlayer());
        }
    }

    void DefeatCombatManager::onPlayerHitHandler(RawHitEvent event, DefeatActorType defActor) {
        SKSE::log::trace("onPlayerHitHandler");
        if (defActor.get()->hasHitImmunity()) {
            SKSE::log::trace("onPlayerHitHandler Hit Immunity - skipped");
            return;
        }
        if (event.aggressor != nullptr) {
            auto aggr_actor = event.aggressor->As<RE::Actor>();
            auto target_actor = event.target->As<RE::Actor>();
            auto targetActor = _defeatActorManager->getActor(target_actor);

            if (targetActor->getState() != DefeatActor::States::ACTIVE) {
                return;
            }

            if (aggr_actor && _defeatActorManager->validForAggressorRole(aggr_actor) &&
                _defeatActorManager->validPlayerForVictimRole(target_actor)) {
                if (registerAndCheckHitGuard(event.aggressor, event.source, event.projectile)) {
                    SKSE::log::trace("onPlayerHitHandler Hit from <{:08X}:{}> rejected by Hit Spam Guard",
                                     event.aggressor->GetFormID(), event.aggressor->GetName());
                    return;
                }

                auto aggrActor = _defeatActorManager->getActor(aggr_actor);
                defActor.get()->setLastHitAggressor(aggrActor);
                auto hitEvent = _defeatManager->createHitEvent(target_actor, aggr_actor, event);
                aggrActor.get()->extraData->getCallback([this, hitEvent] { this->calculatePlayerHit(hitEvent); });
            }
        }
    }

    bool DefeatCombatManager::registerAndCheckHitGuard(RE::TESObjectREFR* actor, RE::FormID source,
                                                       RE::FormID projectile) {
        std::array<HitSpamKey, 2> toCheck{{{0, 0}, {0, 0}}};

        if (source != 0) {
            toCheck[0].actor = actor->GetFormID();
            toCheck[0].source = source;
        }
        if (projectile != 0) {
            toCheck[1].actor = actor->GetFormID();
            toCheck[1].source = projectile;
        }
        auto now = std::chrono::high_resolution_clock::now();

        hitSpamGuardSpinLock->spinLock();

        for (const auto& key : toCheck) {
            if (key.source == 0) {
                continue;
            }
            if (auto search = hitSpamGuard.find(key); search != hitSpamGuard.end()) {
                if ((search->second + _hitGuardExpiration) > now) {
                    hitSpamGuardSpinLock->spinUnlock();
                    return true;
                } else {
                    hitSpamGuardSpinLock->spinUnlock();
                    search->second = now;
                    return false;
                }
            }
            hitSpamGuard.insert({key, now});
        }
        hitSpamGuardSpinLock->spinUnlock();
        return false;
    }
    void DefeatCombatManager::calculatePlayerHit(HitEventType event) {
        SKSE::log::trace("calculatePlayerHit for <{:08X}> from <{:08X}>", event->target->getActorFormId(),
                         event->aggressor->getActorFormId());

        if (event->target->CheckAggressor(event->aggressor)) {
            auto result = KDWay(event);
            if (result != HitResult::SKIP) {
                SKSE::log::trace("calculatePlayerHit: {}", result);
                event->target->setState(DefeatActor::States::DISACTIVE);

                auto vm = RE::SkyrimVM::GetSingleton();
                if (vm) {
                    const auto handle = vm->handlePolicy.GetHandleForObject(
                        static_cast<RE::VMTypeID>(RE::FormType::Reference), event->target->getActor());
                    if (handle && handle != vm->handlePolicy.EmptyHandle()) {
                        RE::BSFixedString eventStr = "KNONKDOWN";
                        if (result == HitResult::KNONKOUT) {
                            eventStr = "KNONKOUT";
                        } else if (result == HitResult::STANDING_STRUGGLE) {
                            eventStr = "STANDING_STRUGGLE";
                        }

                        auto eventArgs = RE::MakeFunctionArguments((RE::TESObjectREFR*)event->aggressor->getActor(),
                                                                   std::move(eventStr));

                        vm->SendAndRelayEvent(handle, OnSLDefeatPlayerKnockDownEventName, eventArgs, nullptr);
                    }
                }
            } else {
                // SKSE::log::trace("calculatePlayerHit: SKIP");
            }
        } else {
            SKSE::log::trace("calculatePlayerHit: Aggressor not valid");
        }
    }

    HitResult DefeatCombatManager::KDWay(HitEventType event) {
        if (!event->target->isKDAllowed()) {
            SKSE::log::trace("KDWay - KD Not Allowed");
            return HitResult::SKIP;
        }
        if (event->target->getDistanceTo(event->aggressor) > _defeatManager->getConfig()->KD_FAR_MAX_DISTANCE) {
            SKSE::log::trace("KDWay - Distance is too big");
            return HitResult::SKIP;
        }

        auto mcmConfig = _defeatManager->getConfig();
        auto result = HitResult::SKIP;

        if (mcmConfig->getConfig<bool>("KDWayThreshold", true)) {
            if ((result = KDWayWound(event)) != HitResult::SKIP) {
                return result;
            }
        }
        if (mcmConfig->getConfig<bool>("KDWayStamina", true)) {
            if ((result = KDWayExhaustion(event)) != HitResult::SKIP) {
                return result;
            }
        }
        if (mcmConfig->getConfig<bool>("KDWayVulnerability", true)) {
            if ((result = KDWayVulnerability(event)) != HitResult::SKIP) {
                return result;
            }
        }
        if (mcmConfig->getConfig<bool>("KDWayDynamic", true)) {
            if ((result = KDWayDynamic(event)) != HitResult::SKIP) {
                return result;
            }
        }
        if (event->isPowerAttack && mcmConfig->getConfig<bool>("KDWayPowerAtk", true)) {
            if ((result = KDWayPowerAtk(event)) != HitResult::SKIP) {
                return result;
            }
        }

        return HitResult::SKIP;
    }

    HitResult DefeatCombatManager::KDWayWound(HitEventType event) {
        auto mcmConfig = _defeatManager->getConfig();
        if (event->isHitBlocked && mcmConfig->getConfig<bool>("KDHealthBlock", false)) {
            return HitResult::SKIP;
        }
        const auto health = event->target->getActorValuePercentage(RE::ActorValue::kHealth) * 100;

        if (_defeatManager->randomChanse(mcmConfig->getConfig<float>("ChanceOnHitPvic", 100.0f)) &&
            (health <= mcmConfig->getConfig<float>("ThresholdPvic", 20.0f)) &&
            (health >= mcmConfig->getConfig<float>("ThresholdPvicMin", 5.0f))) {
            SKSE::log::trace("KDWayExhaustion - Health {}% ", static_cast<int>(health));

            HitResult result = HitResult::KNONKDOWN;
            if (_defeatManager->randomChanse(mcmConfig->getConfig<float>("KnockOutHPvic", 0.0f)) &&
                !event->target->isTied()) {
                result = HitResult::KNONKOUT;
            } else {
                if (mcmConfig->getConfig<bool>("bResistQTE", true) &&
                    _defeatManager->randomChanse(mcmConfig->getConfig<float>("SStruggleHealthPvic", 0.0f))) {
                    result = HitResult::STANDING_STRUGGLE;
                };
            }
            return result;
        }

        return HitResult::SKIP;
    };
    HitResult DefeatCombatManager::KDWayExhaustion(HitEventType event) {
        auto mcmConfig = _defeatManager->getConfig();
        if (event->isHitBlocked && mcmConfig->getConfig<bool>("KDStaminaBlock", false)) {
            return HitResult::SKIP;
        }
        const auto stamina = event->target->getActorValuePercentage(RE::ActorValue::kStamina) * 100;

        SKSE::log::trace("KDWayExhaustion - Heading angle {} ",
                         static_cast<int>(event->target->getHeadingAngle(event->aggressor)));

        if (KDOnlyBack(mcmConfig->getConfig<bool>("KDWayStaminaOB", true), event)) {
            if (_defeatManager->randomChanse(mcmConfig->getConfig<float>("ChanceOnHitPvicS", 50.0f)) &&
                (stamina <= mcmConfig->getConfig<float>("ExhaustionPvic", 50.0f))) {
                SKSE::log::trace("KDWayExhaustion - Stamina {}% ", static_cast<int>(stamina));

                HitResult result = HitResult::KNONKDOWN;
                if (_defeatManager->randomChanse(mcmConfig->getConfig<float>("KnockOutSPvic", 0.0f)) &&
                    !event->target->isTied()) {
                    result = HitResult::KNONKOUT;
                } else {
                    if (mcmConfig->getConfig<bool>("bResistQTE", true) &&
                        _defeatManager->randomChanse(mcmConfig->getConfig<float>("SStruggleExhaustionPvic", 0.0f))) {
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
        if (event->isHitBlocked && mcmConfig->getConfig<bool>("KDVulnerabilityBlock", false)) {
            return HitResult::SKIP;
        }

        if (KDOnlyBack(mcmConfig->getConfig<bool>("KDWayVulnerabilityOB", false), event)) {
            // TODO: DFW_GetVulnerability
            auto vulnerability = event->target->getVulnerability();

            if (_defeatManager->randomChanse(mcmConfig->getConfig<float>("ChanceOnHitPvicVulnerability", 50.0f)) &&
                (vulnerability >= mcmConfig->getConfig<float>("VulnerabilityPvic", 50.0f))) {
                SKSE::log::trace("KDWayVulnerability - Vulnerability {}% ", static_cast<int>(vulnerability));

                HitResult result = HitResult::SKIP;
                if (_defeatManager->randomChanse(mcmConfig->getConfig<float>("KnockOutVulnerabilityPvic", 0.0f)) &&
                    !event->target->isTied()) {
                    result = HitResult::KNONKOUT;
                } else {
                    auto form = event->aggressor->getEquippedSource(event->source);
                    if (form != nullptr) {
                        auto formType = form->GetFormType();
                        if (formType == RE::FormType::Weapon || formType == RE::FormType::Spell) {
                            if (mcmConfig->getConfig<bool>("bResistQTE", true) &&
                                _defeatManager->randomChanse(
                                    mcmConfig->getConfig<float>("SStruggleVulnerabilityPvic", 0.0f))) {
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
        event->target->incrementDynamicDefeat(DefeatAmount);
        auto totalDynamicDefeat = event->target->getDynamicDefeat();

        if (widget->getState() != DefeatWidget::State::DynamicWidget && totalDynamicDefeat > 0) {
            widget->startDynamicWidget();
            shedulePlayerDeplateDynamicDefeat();
        }
        if (totalDynamicDefeat >= 1) {
            widget->stopDynamicWidget();
            event->target->resetDynamicDefeat();

            HitResult result = HitResult::KNONKDOWN;
            if (_defeatManager->randomChanse(mcmConfig->getConfig<float>("KnockOutDynamicPvic", 0.0f)) &&
                !event->target->isTied()) {
                result = HitResult::KNONKOUT;
            } else {
                if (mcmConfig->getConfig<bool>("bResistQTE", true) &&
                    _defeatManager->randomChanse(mcmConfig->getConfig<float>("SStruggleDynamicPvic", 0.0f))) {
                    result = HitResult::STANDING_STRUGGLE;
                };
            }
            // event->target->getDynamicDefeatSpinLock()->spinUnlock();
            return result;
        } else if (totalDynamicDefeat > 0) {
            widget->setPercent(totalDynamicDefeat);
            // event->target->getDynamicDefeatSpinLock()->spinUnlock();
        }

        return HitResult::SKIP;
    }

    void DefeatCombatManager::shedulePlayerDeplateDynamicDefeat() {
        auto player = this->getDefeatManager()->getActorManager()->getPlayer();
        if (player->isSheduledDeplateDynamicDefeat) {
            return;
        }
        player->isSheduledDeplateDynamicDefeat = true;

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
                if (this->getDefeatManager()->getGameState() == SexLabDefeat::DefeatManager::GameState::IN_GAME &&
                    re_ui != nullptr && !re_ui->GameIsPaused() && !this->_playerDeplateDynamicDefeatStopThread) {
                    auto totalDynamicDefeat = player->getDynamicDefeat();
                    if (totalDynamicDefeat > 0) {
                        auto actor = player->getActor();
                        if (actor != nullptr) {
                            auto mcmConfig = _defeatManager->getConfig();
                            if (actor->IsInCombat()) {
                                player->decrementDynamicDefeat(
                                    mcmConfig->getConfig<float>("DynamicDefeatDepleteOverTime", 1.0f) / 100);
                            } else {
                                player->decrementDynamicDefeat(
                                    mcmConfig->getConfig<float>("DynamicDefeatDepleteOverTime", 1.0f) / (100 / 5));
                            }
                            totalDynamicDefeat = _defeatManager->getActorManager()->getPlayer()->getDynamicDefeat();
                        }
                    }

                    SKSE::GetTaskInterface()->AddUITask([this] {
                        auto player = this->getDefeatManager()->getActorManager()->getPlayer();
                        auto totalDynamicDefeat = player->getDynamicDefeat();
                        auto widget = _defeatManager->getWidget();
                        if (widget != nullptr) {
                            if (totalDynamicDefeat <= 0 && widget->getState() == DefeatWidget::State::DynamicWidget) {
                                widget->stopDynamicWidget(true);
                            } else if (totalDynamicDefeat != widget->getLastPercent()) {
                                widget->setPercent(totalDynamicDefeat, true);
                                SKSE::log::trace("PlayerDeplateDynamicDefeat widget deplated to {}%",
                                                 static_cast<int>(totalDynamicDefeat * 100));
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
                    player->isSheduledDeplateDynamicDefeat = false;
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
        float DefeatBaseDamage = mcmConfig->getConfig<float>("DynamicDefeatOnHitBase", 10.0f) / 100;
        float DefeatVulnerabilityMult = 0;
        float DefeatPowerAttackMult = 0;
        float DefeatLowStaminaMult = 0;
        float DefeatLowHeathMult = 0;
        float DefeatBackHit = 0;
        float DefeatBlockReduction = 1.0;

        auto form = event->aggressor->getEquippedSource(event->source);
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
                    DefeatBaseDamage = mcmConfig->getConfig<float>("DynamicDefeatOnHitOneHand", 10.0f) / 100;
                    SKSE::log::trace("CalculateWidget - WeaponType: One-Handed");
                    break;

                case RE::WEAPON_TYPE::kTwoHandSword:
                case RE::WEAPON_TYPE::kTwoHandAxe:
                    DefeatBaseDamage = mcmConfig->getConfig<float>("DynamicDefeatOnHitTwoHand", 10.0f) / 100;
                    SKSE::log::trace("CalculateWidget - WeaponType: Two-Handed");
                    break;

                case RE::WEAPON_TYPE::kBow:
                case RE::WEAPON_TYPE::kCrossbow:
                    DefeatBaseDamage = mcmConfig->getConfig<float>("DynamicDefeatOnHitBow", 10.0f) / 100;
                    SKSE::log::trace("CalculateWidget - WeaponType: Bow");
                    break;

                case RE::WEAPON_TYPE::kStaff:
                    break;
            }
        } else if (formType == RE::FormType::Spell) {
            DefeatBaseDamage = mcmConfig->getConfig<float>("DynamicDefeatOnHitSpell", 10.0f) / 100;
            SKSE::log::trace("CalculateWidget - WeaponType: Spell");
        }

        if (false) {
            SKSE::log::trace("CalculateWidget - DeviousFrameworkON");
            /* TODO: DeviousFramework
            If RessConfig.DeviousFrameworkON && McmConfig.DynamicDefeatUseDFWVulnerability
                DefeatLog("[Defeat] - CalculateWidget - DeviousFrameworkON")
                if McmConfig.DynamicDefeatVulnerabilityMult > 1.0
                    float PlayerVulnerability = DefeatUtil2.DFW_GetVulnerability(Player)
                    if PlayerVulnerability > 0
                        DefeatVulnerabilityMult = (PlayerVulnerability / 100) *
            (McmConfig.DynamicDefeatVulnerabilityMult - 1) endif endif
            */
        } else {
            SKSE::log::trace("CalculateWidget - Vulnerability");
            auto DynamicDefeatVulnerabilityMult = mcmConfig->getConfig<float>("DynamicDefeatVulnerabilityMult", 2.0f);
            if (DynamicDefeatVulnerabilityMult > 1) {
                auto vulnerability = event->target->getVulnerability();
                DefeatVulnerabilityMult = (vulnerability / 100) * (DynamicDefeatVulnerabilityMult - 1);
            }
        }

        if (event->isPowerAttack) {
            DefeatPowerAttackMult = mcmConfig->getConfig<float>("DynamicDefeatPowerAttackMult", 2.0f) - 1;
        }

        auto DynamicDefeatLowStaminaMult = mcmConfig->getConfig<float>("DynamicDefeatLowStaminaMult", 2.0f);
        auto DynamicDefeatLowHealthMult = mcmConfig->getConfig<float>("DynamicDefeatLowHealthMult", 2.0f);
        auto DynamicDefeatBackHitMult = mcmConfig->getConfig<float>("DynamicDefeatBackHitMult", 2.0f);
        auto DynamicDefeatBlockReduction = mcmConfig->getConfig<float>("DynamicDefeatBlockReduction", 50.0f);

        if (DynamicDefeatLowStaminaMult > 1.0) {
            const auto stamina = event->target->getActorValuePercentage(RE::ActorValue::kStamina) * 100;
            if (stamina <= mcmConfig->getConfig<float>("DynamicDefeatLowStaminaThreshold", 50.0f)) {
                DefeatLowStaminaMult = DynamicDefeatLowStaminaMult - 1;
            }
        }

        if (DynamicDefeatLowHealthMult > 1.0) {
            const auto health = event->target->getActorValuePercentage(RE::ActorValue::kHealth) * 100;
            if (health <= mcmConfig->getConfig<float>("DynamicDefeatLowHealthThreshold", 50.0f)) {
                DefeatLowHeathMult = DynamicDefeatLowHealthMult - 1;
            }
        }

        if (DynamicDefeatBackHitMult > 1.0 && KDOnlyBack(true, event)) {
            DefeatBackHit = DynamicDefeatBackHitMult - 1;
        }

        if (DynamicDefeatBlockReduction > 0.0 && event->isHitBlocked) {
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
        if (event->isHitBlocked && mcmConfig->getConfig<bool>("KDWayPowerAtk", true)) {
            return HitResult::SKIP;
        }

        auto form = event->aggressor->getEquippedSource(event->source);
        if (form != nullptr && form->GetFormType() == RE::FormType::Weapon) {
            auto weap = static_cast<RE::TESObjectWEAP*>(form);
            auto stagger = weap->GetStagger() * 100;
            if (_defeatManager->randomChanse(mcmConfig->getConfig<float>("KDWayPowerAtkCOH", 20.0f)) &&
                (stagger >= mcmConfig->getConfig<float>("PowerAtkStagger", 100.0f))) {
                SKSE::log::trace("KDWayPowerAtk - Stagger {}% ", static_cast<int>(stagger));

                HitResult result = HitResult::SKIP;
                if (_defeatManager->randomChanse(mcmConfig->getConfig<float>("KnockOutPPvic", 0.0f)) &&
                    !event->target->isTied()) {
                    result = HitResult::KNONKOUT;
                } else {
                    if (mcmConfig->getConfig<bool>("bResistQTE", true) &&
                        _defeatManager->randomChanse(mcmConfig->getConfig<float>("SStrugglePowerPvic", 0.0f))) {
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
            auto angle = event->target->getHeadingAngle(event->aggressor);
            if (angle < 110.0 && angle > -110.0) {
                return false;
            }
        }
        return true;
    };
}