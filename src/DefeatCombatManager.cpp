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
        if (!defActor->isIgnored()) {
            defActor->requestExtraData(
                defActor, [&] {}, 10s);
        }
    }

    void DefeatCombatManager::onActorEnterBleedout(RE::Actor* actor) {
        if (!_defeatActorManager->validForVictrimRole(actor)) {
            return;
        }

        auto player = _defeatActorManager->getPlayer(actor);

        auto isPlayer = player->isSame(actor);
        DefeatActorType defeatActor = nullptr;

        if (isPlayer) {
            if (!_defeatManager->getConfig()->Config.OnOffPlayerVictim->get()) {
                SKSE::log::trace("onActorEnterBleedout PlayerVictim disabled - skipped");
                return;
            }
        } else {
            if (!_defeatManager->getConfig()->Config.OnOffNVN->get()) {
                SKSE::log::trace("onActorEnterBleedout NvN disabled - skipped");
                return;
            }
        }
        DefeatActorType target = _defeatActorManager->getDefeatActor(actor);

        if (target->getState() == DefeatActorStates::KNONKDOWN_STATE ||
            target->getState() == DefeatActorStates::KNONKOUT_STATE ||
            target->getState() == DefeatActorStates::STANDING_STRUGGLE_STATE) {
            SKSE::log::trace("onActorEnterBleedout target in Knockdown state - skipped");
            return;
        }

        auto mcmConfig = _defeatManager->getConfig();

        HitResult result = HitResult::SKIP;

        if (isPlayer) {
            result = HitResult::SKIP;
        } else {
            if (target->isIgnored()) {
                SKSE::log::trace("onActorEnterBleedout <{:08X}:{}> rejected by Ignore Conditions", actor->GetFormID(),
                                 actor->GetName());
                return;
            }
            if (!_defeatActorManager->isInCombat(target)) {
                SKSE::log::trace("onActorEnterBleedout - <{:08X}> not in combat - skipped", target->getTESFormId());
                return;
            }
            if (_defeatActorManager->isCommandedActor(target)) {
                SKSE::log::trace("onActorEnterBleedout - <{:08X}> is Commanded Actor - skipped",
                                 target->getTESFormId());
                return;
            }
            if (target->isCreature()) {
                SKSE::log::trace("onActorEnterBleedout - <{:08X}> is creature - skipped", target->getTESFormId());
                return;
            }
            if (!target->isKDAllowed()) {
                SKSE::log::trace("onActorEnterBleedout - KD Not Allowed - skipped");
                return;
            }
            if (mcmConfig->Config.NVNKDtype->get() < 2) {
                SKSE::log::trace("onActorEnterBleedout - NVNKDtype disabled for HIT - skipped");
                return;
            }
            if (_defeatActorManager->hasKeywordString(target, _defeatManager->Forms.KeywordId.DefeatActive)) {
                SKSE::log::trace("onActorEnterBleedout - target is DefeatActive - skipped");
                return;
            }

            if (target->isFollower() && !mcmConfig->Config.AllowCvic->get()) {
                SKSE::log::trace("onActorEnterBleedout - Cvic not allowed - skipped");
                return;
            }

            auto aggressor = _defeatActorManager->getSuitableAggressor(target);
            if (aggressor) {
                if (target->isFollower()) {
                    if (mcmConfig->Config.AllowCvic->get()) {
                        SKSE::log::trace("onActorEnterBleedout Follower <{:08X}> Knockdown by NPC <{:08X}>",
                                         target->getTESFormId(), aggressor->getTESFormId());
                        result = HitResult::KNOCKDOWN;
                    }
                } else {
                    if (aggressor->isFollower()) {
                        if (mcmConfig->Config.AllowCagg->get()) {
                            SKSE::log::trace("onActorEnterBleedout NPC <{:08X}> Knockdown by Follower <{:08X}>",
                                             target->getTESFormId(), aggressor->getTESFormId());
                            result = HitResult::KNOCKDOWN;
                        }
                    } else {
                        SKSE::log::trace("onActorEnterBleedout NPC <{:08X}> Knockdown by NPC <{:08X}>",
                                         target->getTESFormId(), aggressor->getTESFormId());
                        result = HitResult::KNOCKDOWN;
                    }
                }
            } else {
                if (target->isFollower()) {
                    SKSE::log::trace("onActorEnterBleedout Follower <{:08X}> Knockdown without Agressor",
                                     target->getTESFormId());
                    result = HitResult::KNOCKDOWN;
                }
            }

            if (result != HitResult::SKIP) {
                target->setHitImmunityFor(10000ms);
                _defeatActorManager->npcKnockDownEvent(target, aggressor, result, true);
            }
        }
    }

    void DefeatCombatManager::onHitHandler(RawHitEvent event) {
        if (event.target == nullptr || event.aggressor == nullptr) {
            return;
        }

        auto aggr_actor = event.aggressor->As<RE::Actor>();
        auto target_actor = event.target->As<RE::Actor>();

        if (!_defeatActorManager->validForAggressorRole(aggr_actor) ||
            !_defeatActorManager->validForVictrimRole(target_actor)) {
            return;
        }

        auto player = _defeatActorManager->getPlayer(target_actor);

        auto isPlayerTarget = player->isSame(target_actor);
        auto isPlayerAggressor = player->isSame(aggr_actor);
        DefeatActorType target = nullptr;
        DefeatActorType source = nullptr;

        if (isPlayerTarget) {
            // npc -> player
            if (!_defeatManager->getConfig()->Config.OnOffPlayerVictim->get()) {
                SKSE::log::trace("onHitHandler PlayerVictim disabled - skipped");
                return;
            }
            if (!_defeatActorManager->validPlayerForVictimRole(target_actor)) {
                SKSE::log::trace("onHitHandler PlayerVictim not valid Player for victim role - skipped");
                return;
            }
            target = player;
        } else {
            if (!isPlayerAggressor) {
                // npc -> npc
                if (!_defeatManager->getConfig()->Config.OnOffNVN->get()) {
                    SKSE::log::trace("onHitHandler NvN disabled - skipped");
                    return;
                }
                target = _defeatActorManager->getDefeatActor(target_actor);
            } else {
                // player -> npc
                if (!_defeatManager->getConfig()->Config.OnOffPlayerAggressor->get()) {
                    SKSE::log::trace("onHitHandler PvN disabled - skipped");
                    return;
                }
                return;
            }
        }

        if (target->hasHitImmunity()) {
            SKSE::log::trace("onHitHandler Hit Immunity - skipped");
            return;
        }

        if (target->getState() != DefeatActorStates::ACTIVE) {
            SKSE::log::trace("onHitHandler target not in Active state - skipped");
            return;
        }

        source = _defeatActorManager->getDefeatActor(aggr_actor);
        if (source->isIgnored()) {
            SKSE::log::trace("onHitHandler Hit from <{:08X}:{}> rejected by Ignore Conditions",
                             event.aggressor->GetFormID(), event.aggressor->GetName());
            return;
        }
        if (target->isIgnored()) {
            SKSE::log::trace("onHitHandler Hit to <{:08X}:{}> rejected by Ignore Conditions", event.target->GetFormID(),
                             event.target->GetName());
            return;
        }

        if (target->registerAndCheckHitGuard(source, event.source, event.projectile)) {
            SKSE::log::trace("onHitHandler Hit from <{:08X}:{}> rejected by Hit Spam Guard",
                             event.aggressor->GetFormID(), event.aggressor->GetName());
            return;
        }

        auto hitEvent = createHitEvent(target, source, event);

        if (isPlayerTarget) {
            onPlayerHitHandler(hitEvent, player, source);
        } else {
            onNvNHitHandler(hitEvent, target, source);
        }
    }

    void DefeatCombatManager::onPlayerHitHandler(HitEvent event, DefeatPlayerActorType targetActor,
                                                 DefeatActorType sourceActor) {
        SKSE::log::trace("onPlayerHitHandler");

        if (sourceActor->isFollower()) {
            SKSE::log::trace("onPlayerHitHandler Aggressor is Follower - skipped");
            return;
        }

        targetActor->setLastHitAggressor(sourceActor);

        targetActor->requestExtraData(
            targetActor, [&] {}, 10s);
        sourceActor->requestExtraData(
            sourceActor, [&] {}, 10s);

        this->calculatePlayerHit(event);
    }

    void DefeatCombatManager::onNvNHitHandler(HitEvent event, DefeatActorType defActor, DefeatActorType source) {
        auto mcmConfig = _defeatManager->getConfig();

        if (!_defeatActorManager->isInCombat(defActor)) {
            SKSE::log::trace("onNvNHitHandler - <{:08X}> not in combat - skipped", defActor->getTESFormId());
            return;
        }

        if (_defeatActorManager->isCommandedActor(defActor)) {
            SKSE::log::trace("onNvNHitHandler - <{:08X}> is Commanded Actor - skipped", defActor->getTESFormId());
            return;
        }

        if (defActor->isCreature()) {
            SKSE::log::trace("onNvNHitHandler - <{:08X}> is creature - skipped", defActor->getTESFormId());
            return;
        }

        if (!_defeatActorManager->hasCombatTarget(source, defActor)) {
            SKSE::log::trace("onNvNHitHandler - <{:08X}> not in target <{:08X}> - skipped", defActor->getTESFormId(),
                             source->getTESFormId());
            return;
        }
        if (!event.target->isKDAllowed()) {
            SKSE::log::trace("onNvNHitHandler - KD Not Allowed - skipped");
            return;
        }
        if (_defeatActorManager->hasKeywordString(source, _defeatManager->Forms.KeywordId.DefeatActive) &&
            !_defeatActorManager->hasKeywordString(source, _defeatManager->Forms.KeywordId.DefeatAggPlayer)) {
            SKSE::log::trace("onNvNHitHandler - Aggressor is DefeatActive and not DefeatAggPlayer - skipped");
            return;
        }
        if (_defeatActorManager->getDistanceBetween(event.target, event.aggressor) > mcmConfig->KD_FAR_MAX_DISTANCE) {
            SKSE::log::trace("onNvNHitHandler - Distance is too big - skipped");
            return;
        }
        if (mcmConfig->Config.NVNKDtype->get() > 2) {
            SKSE::log::trace("onNvNHitHandler - NVNKDtype disabled for HIT - skipped");
            return;
        }

        defActor->setLastHitAggressor(source);
        HitResult result = HitResult::SKIP;

        if (defActor->isFollower()) {
            if (mcmConfig->Config.AllowCvic->get() && randomChanse(mcmConfig->Config.COHFollower->get())) {
                const auto health =
                    _defeatActorManager->getActorValuePercentage(defActor, RE::ActorValue::kHealth) * 100;
                if (health <= mcmConfig->Config.ThresholdFollower->get()) {
                    SKSE::log::trace("onNvNHitHandler Follower Knockdown");
                    result = HitResult::KNOCKDOWN;
                }
            }
        } else {
            if (mcmConfig->Config.EveryoneNVN->get() ||
                _defeatActorManager->hasSexCombinationWithAggressor(defActor, source)) {
                if (source->isFollower()) {
                    if (mcmConfig->Config.AllowCagg->get() && randomChanse(mcmConfig->Config.ChanceOnHitNPC->get())) {
                        const auto health =
                            _defeatActorManager->getActorValuePercentage(defActor, RE::ActorValue::kHealth) * 100;
                        if (health <= mcmConfig->Config.ThresholdNPCvsNPC->get()) {
                            SKSE::log::trace("onNvNHitHandler NPC Knockdown by Follower");
                            result = HitResult::KNOCKDOWN;
                        }
                    }
                } else {
                    if (mcmConfig->Config.AllowNPC->get() && randomChanse(mcmConfig->Config.ChanceOnHitNPC->get())) {
                        const auto health =
                            _defeatActorManager->getActorValuePercentage(defActor, RE::ActorValue::kHealth) * 100;
                        if (health <= mcmConfig->Config.ThresholdNPCvsNPC->get()) {
                            SKSE::log::trace("onNvNHitHandler NPC Knockdown by NPC");
                            result = HitResult::KNOCKDOWN;
                        }
                    }
                }
            }
        }

        if (result != HitResult::SKIP) {
            defActor->setHitImmunityFor(10000ms);
            _defeatActorManager->npcKnockDownEvent(event.target, event.aggressor, result);
        }
    }

    void DefeatCombatManager::calculatePlayerHit(HitEventType event) {
        SKSE::log::trace("calculatePlayerHit for <{:08X}> from <{:08X}>", event.target->getTESFormId(),
                         event.aggressor->getTESFormId());

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