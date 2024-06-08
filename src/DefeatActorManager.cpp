#include "DefeatActorManager.h"
#include "Defeat.h"

namespace SexLabDefeat {

    void DefeatActorManager::reset() {
        SexLabDefeat::UniqueSpinLock lock(*this);
        _actorMap.clear();

        auto actor = RE::PlayerCharacter::GetSingleton();
        std::shared_ptr<DefeatPlayerActorImpl> defeatActor =
            std::make_shared<DefeatPlayerActorImpl>(actor->GetFormID(), this);
        _actorMap.emplace(actor->GetFormID(), defeatActor);
        _player = defeatActor;
    }
    DefeatPlayerActorType DefeatActorManager::getPlayer(RE::Actor* actor) {
        if (actor == nullptr) {
            actor = RE::PlayerCharacter::GetSingleton();
        }
        auto defPlayerImpl = getPlayerImpl();
        SexLabDefeat::UniqueSpinLock lock(*(defPlayerImpl.get()));
        return std::make_shared<DefeatPlayerActor>(defPlayerImpl->_data, actor, defPlayerImpl);
    }

    std::shared_ptr<DefeatActorImpl> DefeatActorManager::getDefeatActorImpl(RE::Actor* actor) {
        assert(actor != nullptr);
        auto formID = actor->GetFormID();
        return getDefeatActorImpl(formID);
    }

    std::shared_ptr<DefeatActorImpl> DefeatActorManager::getDefeatActorImpl(RE::FormID formID) {
        SexLabDefeat::UniqueSpinLock lock(*this);
        auto val = _actorMap.find(formID);
        std::shared_ptr<DefeatActorImpl> defeatActorImpl;
        if (val == _actorMap.end()) {
            defeatActorImpl = std::make_shared<DefeatActorImpl>(formID, this);
            _actorMap.emplace(formID, defeatActorImpl);
        } else {
            defeatActorImpl = val->second;
        }
        return defeatActorImpl;
    }

    DefeatActorType DefeatActorManager::getDefeatActor(RE::FormID formID) {
        if (formID) {
            auto actor = RE::TESForm::LookupByID<RE::Actor>(formID);
            if (actor) {
                auto defImpl = getDefeatActorImpl(actor);
                SexLabDefeat::UniqueSpinLock lock(*defImpl);
                return std::make_shared<DefeatActor>(defImpl->_data, actor, defImpl);
            }
        }
        return nullptr;
    }

    DefeatActorType DefeatActorManager::getDefeatActor(RE::Actor* actor) {
        assert(actor != nullptr);
        auto defImpl = getDefeatActorImpl(actor);
        SexLabDefeat::UniqueSpinLock lock(*defImpl);
        return std::make_shared<DefeatActor>(defImpl->_data, actor, defImpl);
    }
        
    bool IDefeatActorManager::isDefeatAllowedByAgressor(DefeatActorType target, DefeatActorType aggressor) {
        if (target->isPlayer()) {
            return aggressor->isDefeatAllowed2PC();
        }
        return aggressor->isDefeatAllowed2NvN();
    };

    bool IDefeatActorManager::IsSexualAssaulAllowedByAggressor(DefeatActorType target, DefeatActorType aggressor) {
        return hasSexInterestByAggressor(target, aggressor) && hasSexCombinationWithAggressor(target, aggressor);
    };

    bool DefeatActorManager::hasSexInterestByAggressor(DefeatActorType target, DefeatActorType aggressor) {
        if (aggressor->isSatisfied()) {
            SKSE::log::trace("CheckAggressor: isSatisfied");
            return false;
        }
        if (target->isPlayer()) {
            return randomChanse(_defeatManager->getConfig()->Config.PvicRaped->get());
        }
        if (target->isFollower()) {
            return randomChanse(_defeatManager->getConfig()->Config.NVNRapedFollower->get());
        }
        return randomChanse(_defeatManager->getConfig()->Config.NVNRaped->get());
    }

    bool DefeatActorManager::hasSexCombinationWithAggressor(DefeatActorType target, DefeatActorType aggressor) {
        auto mcmConfig = _defeatManager->getConfig();
        if (target->isPlayer()) {
            if (!aggressor->isCreature()) {
                bool aggressorFemale = aggressor->isFemale();
                if (!mcmConfig->Config.SexualityPvic->get()) {
                    return (!aggressorFemale && mcmConfig->Config.MaleHunterPvic->get()) ||
                           (aggressorFemale && mcmConfig->Config.FemaleHunterPvic->get());
                } else {
                    bool victimFemale = target->isFemale();
                    if (!aggressorFemale && victimFemale) {  // Male on Female
                        return mcmConfig->Config.MaleHunterPvic->get() &&
                               (aggressor->IsStraight() || aggressor->IsBisexual());

                    } else if (aggressorFemale && victimFemale) {  // Female on Female
                        return mcmConfig->Config.FemaleHunterPvic->get() &&
                               (aggressor->IsGay() || aggressor->IsBisexual());

                    } else if (aggressorFemale && !victimFemale) {  // Female on Male
                        return mcmConfig->Config.FemaleHunterPvic->get() &&
                               (aggressor->IsStraight() || aggressor->IsBisexual());

                    } else if (!aggressorFemale && !victimFemale) {  // Male on Male
                        return mcmConfig->Config.MaleHunterPvic->get() &&
                               (aggressor->IsGay() || aggressor->IsBisexual());
                    }
                }
            } else {
                if (notInFlyingState(aggressor)) {
                    if (!mcmConfig->Config.SexLab.UseCreatureGender->get()) {
                        return true;
                    }
                    auto gender = aggressor->getSexLabGender();  // 3 - Female creatures, returns 2 male if
                                                                 // SexLabConfig.UseCreatureGender is disabled
                    if (gender == 2 && mcmConfig->Config.HuntCrea->get()) {
                        return true;
                    } else if (gender == 3 && mcmConfig->Config.HuntFCrea->get()) {
                        return true;
                    }
                }
            }
        } else {
            bool victimFemale = target->isFemale();
            if (!aggressor->isCreature()) {
                bool aggressorFemale = aggressor->isFemale();
                if (!mcmConfig->Config.SexualityNVN->get()) {
                    return (!aggressorFemale && victimFemale && mcmConfig->Config.MaleOnGal->get()) ||
                           (aggressorFemale && victimFemale && mcmConfig->Config.GalOnGal->get()) ||
                           (!aggressorFemale && !victimFemale && mcmConfig->Config.MaleOnMale->get()) ||
                           (aggressorFemale && !victimFemale && mcmConfig->Config.GalOnMale->get());
                } else {
                    if (!aggressorFemale && victimFemale) {  // Male on Female
                        return mcmConfig->Config.MaleOnGal->get() &&
                               (aggressor->IsStraight() || aggressor->IsBisexual());

                    } else if (aggressorFemale && victimFemale) {  // Female on Female
                        return mcmConfig->Config.GalOnGal->get() && (aggressor->IsGay() || aggressor->IsBisexual());

                    } else if (aggressorFemale && !victimFemale) {  // Female on Male
                        return mcmConfig->Config.GalOnMale->get() &&
                               (aggressor->IsStraight() || aggressor->IsBisexual());

                    } else if (!aggressorFemale && !victimFemale) {  // Male on Male
                        return mcmConfig->Config.MaleOnMale->get() && (aggressor->IsGay() || aggressor->IsBisexual());
                    }
                }
            } else {
                if (notInFlyingState(aggressor)) {
                    if (!mcmConfig->Config.SexLab.UseCreatureGender->get()) {
                        return true;
                    }
                    auto aggressorFemale = aggressor->getSexLabGender() == 3;
                    if (!aggressorFemale && victimFemale && mcmConfig->Config.CreaOnFemale->get()) {  // Male on Female
                        return true;

                    } else if (aggressorFemale && victimFemale &&
                               mcmConfig->Config.CreaFemaleOnFemale->get()) {  // Female on Female
                        return true;

                    } else if (aggressorFemale && !victimFemale &&
                               mcmConfig->Config.CreaFemaleOnMale->get()) {  // Female on Male
                        return true;

                    } else if (!aggressorFemale && !victimFemale &&
                               mcmConfig->Config.CreaOnMale->get()) {  // Male on Male
                        return true;
                    }
                    // Other genders
                    if ((victimFemale && mcmConfig->Config.CreaOnFemale->get()) ||
                        (!victimFemale && mcmConfig->Config.CreaOnMale->get())) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool DefeatActorManager::checkAggressor(DefeatActorType target, DefeatActorType aggressor) {
        if (aggressor->isIgnoreActorOnHit()) {
            SKSE::log::trace("CheckAggressor: false - isIgnoreActorOnHit");
            return false;
        }

        if (target->isSurrender() || _defeatManager->getConfig()->Config.EveryonePvic->get()) {
            return true;
        } else {
            if (!aggressor->isSexLabAllowed() || !isDefeatAllowedByAgressor(target, aggressor)) {
                return false;
            }
            if (aggressor->isCreature() && !_defeatManager->getConfig()->Config.HuntCrea->get()) {
                return true;
            } else {
                // sexuality
                return hasSexInterestByAggressor(target, aggressor);
            }
        }
    };

    void DefeatActorManager::playerKnockDownEvent(DefeatActorType target, DefeatActorType aggressor, HitResult event) {
        PapyrusInterface::CallbackPtr callback(new DefeatRequestCallback(DefeatRequestCallback::ActorType::PLAYER));

        RE::BSFixedString eventStr = "KNOCKDOWN";
        if (event == HitResult::KNOCKOUT) {
            eventStr = "KNOCKOUT";
        } else if (event == HitResult::STANDING_STRUGGLE) {
            eventStr = "STANDING_STRUGGLE";
        }
        SKSE::log::trace("playerKnockDownEvent <{}>", eventStr);

        if (PapyrusInterface::DispatchStaticCall("defeat_skse_api", "playerKnockDownEvent", callback,
                                                 aggressor->getTESActor(), std::move(eventStr))) {
            return;
        }
        SKSE::log::error("Failed to dispatch static call [defeat_skse_api::npcKnockDownEvent].");

/* auto vm = RE::SkyrimVM::GetSingleton();
        if (vm) {
            const auto handle = vm->handlePolicy.GetHandleForObject(static_cast<RE::VMTypeID>(RE::FormType::Reference),
                                                                    target->getTESActor());
            if (handle && handle != vm->handlePolicy.EmptyHandle()) {
                RE::BSFixedString eventStr = "KNOCKDOWN";
                if (event == HitResult::KNOCKOUT) {
                    eventStr = "KNOCKOUT";
                } else if (event == HitResult::STANDING_STRUGGLE) {
                    eventStr = "STANDING_STRUGGLE";
                }
                SKSE::log::trace("playerKnockDownEvent <{}>", eventStr);
                 auto eventArgs =
                    RE::MakeFunctionArguments((RE::TESObjectREFR*)aggressor->getTESActor(), std::move(eventStr));

                vm->SendAndRelayEvent(
                    handle,
                    &(_defeatManager->getConfig()->Config.PapyrusFunctionNames.OnSLDefeatPlayerKnockDownEventName),
                    eventArgs, nullptr);
            }
        }*/
    }
    void DefeatActorManager::sexLabSceneInterrupt(DefeatActorType target, DefeatActorType aggressor, bool isHit) {
        if (aggressor) {
            sexLabSceneInterrupt(target->getTESActor(), aggressor->getTESActor(), isHit);
        } else {
            sexLabSceneInterrupt(target->getTESActor(), nullptr, isHit);
        }
    }

    void DefeatActorManager::sexLabSceneInterrupt(RE::Actor* target, RE::Actor* aggressor, bool isHit) {
        auto now = clock::now();

        _sexLabInterruptExpirationsLock.spinLock();
        auto val = _sexLabInterruptExpirations.find(target->GetFormID());
        if (val == _sexLabInterruptExpirations.end() || (now > val->second)) {
            _sexLabInterruptExpirations.insert_or_assign(target->GetFormID(), now + 5000ms);
            _sexLabInterruptExpirationsLock.spinUnlock();

            if (target->IsPlayerRef()) {
                if (!isHit) {
                    return;
                }
            }
            auto mcmConfig = _defeatManager->getConfig();
            if (mcmConfig->Config.OnOffPlayerVictim->get() && !mcmConfig->Config.OnOffNVN->get()) {
            } else {
                if (isHit) {
                    if (!mcmConfig->Config.HitInterrupt->get()) {
                        return;
                    }
                } else {
                    if (!mcmConfig->Config.CombatInterrupt->get()) {
                        return;
                    }
                }

                PapyrusInterface::CallbackPtr callback(
                    new PapyrusInterface::EmptyRequestCallback("sexLabSceneInterrupt"));

                if (aggressor) {
                    SKSE::log::trace("sexLabSceneInterrupt: <{:08X}:{}> by <{:08X}:{}>", target->GetFormID(),
                                     target->GetName(), aggressor->GetFormID(), aggressor->GetName());
                } else {
                    SKSE::log::trace("sexLabSceneInterrupt: <{:08X}:{}>", target->GetFormID(),
                                     target->GetName());
                }

                if (PapyrusInterface::DispatchStaticCall("defeat_skse_api", "sexLabSceneInterrupt", callback,
                                                            std::move(target), std::move(aggressor))) {
                    return;
                }
                SKSE::log::error("Failed to dispatch static call [defeat_skse_api::npcKnockDownEvent].");

            }
        } else {
            _sexLabInterruptExpirationsLock.spinUnlock();
            SKSE::log::trace("sexLabSceneInterrupt: <{:08X}:{}> not allowed", target->GetFormID(), target->GetName());
        }
    }

    

    static OnBSAnimationGraphEventHandler* instanse;

    static OnBSAnimationGraphEventHandler* getInstance(IDefeatManager* defeatManager) {
        if (!instanse) {
            instanse = new OnBSAnimationGraphEventHandler(defeatManager);
        }
        return instanse;
    }

    void DefeatActorManager::npcKnockDownEvent(DefeatActorType target, DefeatActorType aggressor, HitResult event,
                                               bool isBleedout, bool isAssault) {
        auto npcType = DefeatRequestCallback::ActorType::NPC;
        if (target->isFollower()) {
            npcType = DefeatRequestCallback::ActorType::FOLLOWER;
        }
        PapyrusInterface::CallbackPtr callback(new DefeatRequestCallback(npcType));

        RE::BSFixedString eventStr = "KNOCKDOWN";
        if (event == HitResult::KNOCKOUT) {
            eventStr = "KNOCKOUT";
        } else if (event == HitResult::STANDING_STRUGGLE) {
            eventStr = "STANDING_STRUGGLE";
        }
        RE::Actor* aggressorActor = nullptr;
        RE::Actor* targetActor = target->getTESActor();
        if (aggressor) {
            aggressorActor = aggressor->getTESActor();
            SKSE::log::trace("npcKnockDownEvent - {}: <{:08X}> from <{:08X}>", eventStr, target->getTESFormId(),
                             aggressor->getTESFormId());
        } else {
            SKSE::log::trace("npcKnockDownEvent - {}: <{:08X}>", eventStr, target->getTESFormId());
        }

        // Attempt to fix auto bleedout recovery
        if (isBleedout) {
            SKSE::log::trace("Set NoBleedoutRecovery for <{:08X}>.", target->getTESFormId());
            target->getTESActor()->GetActorRuntimeData().boolFlags.set(RE::Actor::BOOL_FLAGS::kNoBleedoutRecovery);
        }

        //target->getTESActor()->AddAnimationGraphEventSink(getInstance(_defeatManager));
        if (PapyrusInterface::DispatchStaticCall("defeat_skse_api", "npcKnockDownEvent", callback,
                                                 std::move(targetActor),
                                                 std::move(aggressorActor), std::move(eventStr),
                                                 std::move(isBleedout), std::move(isAssault))) {
            //target->getTESActor()->GetActorRuntimeData().boolFlags.set(RE::Actor::BOOL_FLAGS::kNoBleedoutRecovery);
            return;
        }
        SKSE::log::error("Failed to dispatch static call [defeat_skse_api::npcKnockDownEvent].");
    }

    bool DefeatActorManager::isIgnored(RE::Actor* actor) {
        for (RE::TESFaction* faction : _defeatManager->Forms.Ignore.Factions) {
            if (actor->IsInFaction(faction)) {
                return true;
            }
        };
        for (std::string_view keyword : _defeatManager->Forms.Ignore.Keywords) {
            if (actor->HasKeywordString(keyword)) {
                return true;
            }
        };
        return false;
    }

    bool IDefeatActorManager::validForAggressorRole(DefeatActorType actor) {
        return validForAggressorRole(actor->getTESActor());
    }

    bool IDefeatActorManager::validForAggressorRole(RE::Actor* actor) {
        if (actor == nullptr || actor->IsGhost() || !actor->Is3DLoaded()) {
            return false;
        }
        return true;
    }

    bool IDefeatActorManager::validForVictrimRole(RE::Actor* actor) {
        if (actor == nullptr || actor->IsGhost() || !actor->Is3DLoaded()) {
            return false;
        }
        return true;
    }

    bool DefeatActorManager::validPlayerForVictimRole(RE::Actor* actor) {
        if (actor == nullptr || actor->IsOnMount() || actor->HasKeywordString(_defeatManager->Forms.KeywordId.DefeatActive)) {
            return false;
        }
        if (!actor->HasKeywordString(_defeatManager->Forms.KeywordId.ActorTypeNPC)) {
            return _defeatManager->getConfig()->Config.BeastImmunity->get();
        }
        return true;
    }

    DefeatConfig* DefeatActorManager::getConfig() { return _defeatManager->getConfig(); }

    DefeatForms DefeatActorManager::getForms() { return _defeatManager->Forms; }

    SoftDependencyType DefeatActorManager::getSoftDependency() { return _defeatManager->SoftDependency; }

    float IDefeatActorManager::getDistanceBetween(DefeatActorType source, DefeatActorType target) {
        auto a1 = source->getTESActor()->GetPosition();
        auto a2 = target->getTESActor()->GetPosition();
        return a1.GetDistance(a2);
    }
    void IDefeatActorManager::forEachActorsInRange(RE::Actor* target, float a_range,
                                                  std::function<bool(RE::Actor* a_actor)> a_callback) {
        const auto position = target->GetPosition();

        for (auto actorHandle : RE::ProcessLists::GetSingleton()->highActorHandles) {
            if (auto actorPtr = actorHandle.get()) {
                if (auto actor = actorPtr.get()) {
                    if (a_range == 0 || position.GetDistance(actor->GetPosition()) <= a_range) {
                        auto loc_refBase = actor->GetActorBase();
                        if (actor && actor != target && !actor->IsDisabled() && actor->Is3DLoaded() &&
                            !actor->IsPlayerRef() && !actor->IsDead() &&
                            (actor->Is(RE::FormType::NPC) || (loc_refBase && loc_refBase->Is(RE::FormType::NPC))) &&
                            !actor->IsGhost() && !actor->IsOnMount()) {
                            if (!a_callback(actor)) {
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    DefeatActorType DefeatActorManager::getSuitableAggressor(DefeatActorType actor) {
        auto EveryoneNVN = _defeatManager->getConfig()->Config.EveryoneNVN->get();

        auto lastAggressor = actor->getLastHitAggressor();

        if (lastAggressor) {
            if (getDistanceBetween(actor, lastAggressor) > 2000 ||
                !(EveryoneNVN || this->hasSexCombinationWithAggressor(actor, lastAggressor)) ||
                !validForAggressorRole(lastAggressor) || 
                    (hasKeywordString(lastAggressor, _defeatManager->Forms.KeywordId.DefeatActive) &&
                    !hasKeywordString(lastAggressor, _defeatManager->Forms.KeywordId.DefeatAggPlayer))
                ) {

                lastAggressor = nullptr;
            }
        }

        if (!lastAggressor) {
            forEachActorsInRange(actor->getTESActor(), 2000, [&](RE::Actor* a_actor) {
                if (a_actor->IsInCombat() &&
                    (!a_actor->HasKeywordString(_defeatManager->Forms.KeywordId.DefeatActive) ||
                     a_actor->HasKeywordString(_defeatManager->Forms.KeywordId.DefeatAggPlayer)) &&
                    a_actor->IsHostileToActor(actor->getTESActor())
                    ) {
                    // TODO: additional checks
                    // TODO: maybe last in target/hit agressors
                    DefeatActorType aggrActor = getDefeatActor(a_actor);
                    if (validForAggressorRole(aggrActor) &&
                        (EveryoneNVN ||
                        this->hasSexCombinationWithAggressor(actor, aggrActor))) {
                        lastAggressor = aggrActor;
                        SKSE::log::trace("getSuitableAggressor in range for <{:08X}> is <{:08X}>",
                                         actor->getTESFormId(), aggrActor->getTESFormId());
                        return false;
                    }
                }
                return true;
            });
        }
        return lastAggressor;
    }
    std::list<DefeatActorType> DefeatActorManager::getSuitableFollowers(DefeatActorType actor) {
        auto ret = std::list<DefeatActorType>();
        forEachActorsInRange(actor->getTESActor(), 0, [&](RE::Actor* a_actor) {
            if (a_actor->IsPlayerTeammate() ||
                a_actor->IsInFaction(_defeatManager->Forms.Faction.CurrentFollowerFaction) ||
                a_actor->IsInFaction(_defeatManager->Forms.Faction.CurrentHireling)) {

                auto defeatActor = getDefeatActor(a_actor);
                if (defeatActor) {
                    ret.push_back(defeatActor);
                    SKSE::log::trace("getSuitableFollowers: <{:08X}>", a_actor->GetFormID(),
                                     a_actor->GetName());
                }
            }
            return true;
        });
        return ret;
    }
    std::list<DefeatActorType> DefeatActorManager::getSuitableAggressors(DefeatActorType actor) {
        auto ret = std::list<DefeatActorType>();
        return ret;
    }
    float IDefeatActorManager::getHeadingAngleBetween(DefeatActorType source, DefeatActorType target) {
        auto a1 = target->getTESActor()->GetPosition();
        return source->getTESActor()->GetHeadingAngle(a1, false);
    }
    float IDefeatActorManager::getActorValuePercentage(DefeatActorType source, RE::ActorValue av) {
        auto actor = source->getTESActor();
        auto actorAV = actor->AsActorValueOwner();

        auto temporary = actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, av);
        auto total = actorAV->GetPermanentActorValue(av);
        auto current = actorAV->GetActorValue(av);

        return total > 0 ? current / (total + temporary) : 1.0;
    }
    RE::TESForm* IDefeatActorManager::getEquippedHitSourceByFormID(DefeatActorType source, RE::FormID hitSource) {
        auto actor = source->getTESActor();
        RE::TESForm* form = nullptr;
        auto equipped_right = actor->GetEquippedObject(false);
        if (equipped_right != nullptr && equipped_right->GetFormID() == hitSource) {
            form = equipped_right;
        } else {
            auto equipped_left = actor->GetEquippedObject(true);
            if (equipped_left != nullptr && equipped_left->GetFormID() == hitSource) {
                form = equipped_left;
            }
        }
        return form;
    }
    bool IDefeatActorManager::wornHasAnyKeyword(DefeatActorType source, std::list<std::string> kwds) {
        return wornHasAnyKeyword(*source, kwds);
    }

    bool IDefeatActorManager::wornHasAnyKeyword(DefeatActor& source, std::list<std::string> kwds) {
        auto actor = source.getTESActor();

        bool ret = false;

        auto visitor = SexLabDefeat::WornVisitor([&kwds, &ret](RE::InventoryEntryData* a_entry) {
            auto loc_object = a_entry->GetObject();
            RE::TESObjectARMO* loc_armor = nullptr;
            if (loc_object != nullptr && loc_object->IsArmor()) {
                loc_armor = static_cast<RE::TESObjectARMO*>(loc_object);
                if (loc_armor != nullptr) {
                    for (const std::string& kwd : kwds) {
                        SKSE::log::trace("wornHasAnyKeyword - {}:{}", loc_armor->GetFullName(), kwd);
                        if (loc_armor->HasKeywordString(kwd)) {
                            ret = true;
                            return RE::BSContainer::ForEachResult::kStop;
                        }
                    }
                }
            }
            return RE::BSContainer::ForEachResult::kContinue;
        });
        actor->GetInventoryChanges()->VisitWornItems(visitor);
        return ret;
    }
    bool IDefeatActorManager::hasKeywordString(DefeatActorType source, std::string_view kwd) {
        return hasKeywordString(*source, kwd);
    }
    bool IDefeatActorManager::hasKeywordString(DefeatActor& source, std::string_view kwd) {
        return source.getTESActor()->HasKeywordString(kwd);
    }
    bool IDefeatActorManager::isInFaction(DefeatActorType actor, RE::TESFaction* faction) {
        return isInFaction(*actor, faction);
    }
    bool IDefeatActorManager::isInFaction(DefeatActor& actor, RE::TESFaction* faction) { 
        return actor.getTESActor()->IsInFaction(faction);
    }
    bool IDefeatActorManager::hasCombatTarget(DefeatActorType source, DefeatActorType target) { 
        auto combatGroup = source->getTESActor()->GetCombatGroup();
        if (!combatGroup) {
            return false;
        }
        combatGroup->lock.LockForRead();
        bool ret = false;
        for (auto&& combatTarget : combatGroup->targets) {
            auto targetptr = combatTarget.targetHandle.get();
            if (targetptr) {
                auto targetActor = targetptr.get();
                if (targetActor != nullptr) {
                    if (targetActor == target->getTESActor()) {
                        ret = true;
                        break;
                    }
                }
            }
        }
        combatGroup->lock.UnlockForRead();
        return ret;
    }
    bool IDefeatActorManager::notInFlyingState(DefeatActorType source) {
        return notInFlyingState(*source);
    }
    bool IDefeatActorManager::notInFlyingState(DefeatActor& source) {
        return source.getTESActor()->AsActorState()->GetFlyState() == RE::FLY_STATE::kNone;
    }
    bool IDefeatActorManager::hasSpell(DefeatActorType source, RE::SpellItem* spell) { return hasSpell(*source, spell); }
    bool IDefeatActorManager::hasSpell(DefeatActor& source, RE::SpellItem* spell) {
        return spell != nullptr && source.getTESActor()->HasSpell(spell);
    }
    bool IDefeatActorManager::hasMagicEffect(DefeatActorType source, RE::EffectSetting* effect) {
        return hasMagicEffect(*source, effect);
    }
    bool IDefeatActorManager::hasMagicEffect(DefeatActor& source, RE::EffectSetting* effect) {
        return effect != nullptr && source.getTESActor()->GetMagicTarget()->HasMagicEffect(effect);
    }
    bool IDefeatActorManager::isInKillMove(DefeatActorType source) { return isInKillMove(*source); }
    bool IDefeatActorManager::isInKillMove(DefeatActor& source) { return source.getTESActor()->IsInKillMove(); }
    bool IDefeatActorManager::isQuestEnabled(RE::TESQuest* quest) { return quest != nullptr && quest->IsEnabled(); }
    bool IDefeatActorManager::isInCombat(DefeatActorType source) { return source->getTESActor()->IsInCombat(); }
    bool IDefeatActorManager::isCommandedActor(DefeatActorType source) {
        return source->getTESActor()->IsCommandedActor();
    }
    bool IDefeatActorManager::isPlayerTeammate(DefeatActorType source) { return isPlayerTeammate(*source); }
    bool IDefeatActorManager::isPlayerTeammate(DefeatActor& source) {
        return source.getTESActor()->IsPlayerTeammate();
    }

    void DefeatRequestCallback::operator()(RE::BSScript::Variable a_result) {
        //auto process = RE::ProcessLists::GetSingleton();
        //process->runDetection = false;
        //process->ClearCachedFactionFightReactions();
        //process->runDetection = true;
    }

}