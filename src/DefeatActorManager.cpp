#include "DefeatActorManager.h"

#include "DefeatActor.h"

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
    DefeatPlayerActorType DefeatActorManager::getPlayer() {
        auto defPlayerImpl = getPlayerImpl();
        SexLabDefeat::UniqueSpinLock lock(*(defPlayerImpl.get()));
        return std::make_shared<DefeatPlayerActor>(defPlayerImpl->_data, RE::PlayerCharacter::GetSingleton(),
                                                   defPlayerImpl);
    }

    std::shared_ptr<DefeatActorImpl> DefeatActorManager::getDefeatActorImpl(RE::Actor* actor) {
        return getDefeatActorImpl(actor->GetFormID());
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
    DefeatActorStates DefeatActorManager::getDefeatActorImplState(RE::FormID formID) {
        SexLabDefeat::UniqueSpinLock lock(*this);
        auto val = _actorMap.find(formID);
        DefeatActorStates state = DefeatActorStates::ACTIVE;
        if (val != _actorMap.end()) {
            val->second->spinLock();
            state = val->second->getState();
            val->second->spinUnlock();
        }
        return state;
    }

    DefeatActorType DefeatActorManager::getDefeatActor(RE::FormID formID) {
        if (formID) {
            auto actor = RE::TESForm::LookupByID<RE::Actor>(formID);
            if (actor) {
                return getDefeatActor(actor);
            }
        }
        return nullptr;
    }

    DefeatActorType DefeatActorManager::getDefeatActor(RE::Actor* actor) {
        if (!actor) {
            return nullptr;
        }
        auto defImpl = getDefeatActorImpl(actor);
        SexLabDefeat::UniqueSpinLock lock(*defImpl);
        if (defImpl->isPlayer()) {
            return std::make_shared<DefeatPlayerActor>(defImpl->_data, actor, defImpl);
        } else {
            return std::make_shared<DefeatActor>(defImpl->_data, actor, defImpl);
        }
    }

    DefeatActorType DefeatActorManager::getDefeatActor(IDefeatActorType actor) { return getDefeatActor(actor->getTESFormId()); }

    RE::Actor* DefeatActorManager::getTESActor(DefeatActor* actor) { return actor->getTESActor(); }
        
    bool DefeatActorManager::isDefeatAllowedByAgressor(DefeatActor* target, DefeatActor* aggressor) {
        if (target->isPlayer()) {
            return aggressor->isDefeatAllowed2PC();
        }
        return aggressor->isDefeatAllowed2NvN();
    };

    bool DefeatActorManager::IsSexualAssaulAllowedByAggressor(DefeatActor* target, DefeatActor* aggressor) {
        return hasSexInterestByAggressor(target, aggressor) && hasSexCombinationWithAggressor(target, aggressor);
    };

    bool DefeatActorManager::hasSexInterestByAggressor(DefeatActor* target, DefeatActor* aggressor) {
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

    bool DefeatActorManager::hasSexCombinationWithAggressor(DefeatActor* target, DefeatActor* aggressor) {
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

    bool DefeatActorManager::checkAggressor(DefeatActor* target, DefeatActor* aggressor) {
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

    void DefeatActorManager::playerKnockDownEvent(DefeatActor* target, DefeatActor* aggressor, HitResult event) {
        PapyrusInterface::CallbackPtr callback(new DefeatRequestCallback(DefeatRequestCallback::ActorType::PLAYER));

        RE::BSFixedString eventStr = "KNOCKDOWN";
        if (event == HitResult::KNOCKOUT) {
            eventStr = "KNOCKOUT";
        } else if (event == HitResult::STANDING_STRUGGLE) {
            eventStr = "STANDING_STRUGGLE";
        }
        SKSE::log::trace("playerKnockDownEvent <{}>", eventStr.c_str());

        if (PapyrusInterface::DispatchStaticCall("defeat_skse_api", "playerKnockDownEvent", callback,
                                                 aggressor->getTESActor(), std::move(eventStr))) {
            return;
        }
        SKSE::log::error("Failed to dispatch static call [defeat_skse_api::playerKnockDownEvent].");
    }
    void DefeatActorManager::sexLabSceneInterrupt(DefeatActor* target, DefeatActor* aggressor, bool isHit) {
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
                SKSE::log::error("Failed to dispatch static call [defeat_skse_api::sexLabSceneInterrupt].");

            }
        } else {
            _sexLabInterruptExpirationsLock.spinUnlock();
            SKSE::log::trace("sexLabSceneInterrupt: <{:08X}:{}> not allowed", target->GetFormID(), target->GetName());
        }
    }

    /*

    static OnBSAnimationGraphEventHandler* instanse;

    static OnBSAnimationGraphEventHandler* getInstance(IDefeatManager* defeatManager) {
        if (!instanse) {
            instanse = new OnBSAnimationGraphEventHandler(defeatManager);
        }
        return instanse;
    }
    */

    void DefeatActorManager::npcKnockDownEvent(DefeatActor* target, DefeatActor* aggressor, HitResult event,
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
            SKSE::log::trace("npcKnockDownEvent - {}: <{:08X}> from <{:08X}>", eventStr.c_str(), target->getTESFormId(),
                             aggressor->getTESFormId());
        } else {
            SKSE::log::trace("npcKnockDownEvent - {}: <{:08X}>", eventStr.c_str(), target->getTESFormId());
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

    bool DefeatActorManager::validForAggressorRole(DefeatActor* actor) const {
        auto tesActor = actor->getTESActor();
        if (tesActor == nullptr || tesActor->IsDead() || tesActor->IsGhost() || !tesActor->Is3DLoaded()) {
            return false;
        }
        return true;
    }

    bool DefeatActorManager::validForVictrimRole(DefeatActor* actor) const {
        auto tesActor = actor->getTESActor();
        if (tesActor == nullptr || tesActor->IsDead() || tesActor->IsGhost() || !tesActor->Is3DLoaded()) {
            return false;
        }
        return true;
    }

    bool DefeatActorManager::validPlayerForVictimRole(RE::Actor* actor) {
        if (actor == nullptr || !actor->IsInCombat() || actor->IsOnMount() ||
            actor->HasKeywordString(_defeatManager->Forms.KeywordId.DefeatActive)) {
            return false;
        }
        if (!actor->HasKeywordString(_defeatManager->Forms.KeywordId.ActorTypeNPC)) {
            return _defeatManager->getConfig()->Config.BeastImmunity->get();
        }
        return true;
    }

    DefeatConfig* DefeatActorManager::getConfig() { return _defeatManager->getConfig(); }

    DefeatForms DefeatActorManager::getForms() const { return _defeatManager->Forms; }

    SoftDependencyType DefeatActorManager::getSoftDependency() { return _defeatManager->SoftDependency; }

    float DefeatActorManager::getDistanceBetween(DefeatActor* source, DefeatActor* target) {
        auto a1 = source->getTESActor()->GetPosition();
        auto a2 = target->getTESActor()->GetPosition();
        return a1.GetDistance(a2);
    }
    void DefeatActorManager::forEachActorsInRange(RE::Actor* target, float a_range,
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
    std::list<DefeatActorType> DefeatActorManager::getNearestAggressors(DefeatActor* actor) {
        auto LastHitAggressors = actor->getLastHitAggressors();

        auto ret = getNearestActorsInRangeByFilter(actor, 2000, [&](RE::Actor* a_actor) {
            auto search = LastHitAggressors.find(a_actor->GetFormID());
            if (search != LastHitAggressors.end() &&
                /* TODO: must be obsolete */
                (!a_actor->HasKeywordString(_defeatManager->Forms.KeywordId.DefeatActive) ||
                 a_actor->HasKeywordString(_defeatManager->Forms.KeywordId.DefeatAggPlayer))
                /* ---------------------- */
                ) {
                return true;
            }
            return false;
        });
        return ret;
    }
    std::list<DefeatActorType> DefeatActorManager::getNearestAggressorsForGangBang(DefeatActor* actor) {
        auto list = getNearestActorsInRangeByFilter(actor, 3000, [&](RE::Actor* a_actor) {
            if (
                !a_actor->HasKeywordString(_defeatManager->Forms.KeywordId.DefeatActive) &&
                !a_actor->HasKeywordString(_defeatManager->Forms.KeywordId.SexLabActive)
            ) {
                return true;
            }
            return false;
        });
        std::list<DefeatActorType> ret{};
        std::set<RE::FormID> candidateAgressorsForFollowers{};
        std::list<DefeatActorType> followers{};
        auto isFollower = actor->isFollower();

        std::list<DefeatActorType>::iterator defActorIt = list.begin();
        while (defActorIt != list.end()) {
            if ((*defActorIt)->isFollower()) {
                followers.push_back((*defActorIt));
            }
            if (isFollower) {
                if ((*defActorIt)->isFollower()) {
                    SKSE::log::trace("getNearestAggressorsForGangBang {:08X} isFollower skip - {:08X}",
                                     actor->getTESFormId(), (*defActorIt)->getTESFormId());
                    auto followerLastHitAggressors = (*defActorIt)->getLastHitAggressors();
                    for (LastHitAggressorsType::const_iterator it = followerLastHitAggressors.begin();
                         it != followerLastHitAggressors.end(); ++it) {
                        candidateAgressorsForFollowers.insert(it->first);
                    }
                    defActorIt = list.erase(defActorIt);
                    continue;
                }
            }
            auto lastHitAggressors = actor->getLastHitAggressors();
            if (lastHitAggressors.find((*defActorIt)->getTESFormId()) != lastHitAggressors.end()) {
                if (!IsSexualAssaulAllowedByAggressor(actor, (*defActorIt).get())) {
                    SKSE::log::trace(
                        "getNearestAggressorsForGangBang {:08X} !IsSexualAssaulAllowedByAggressor skip - {:08X}",
                        actor->getTESFormId(), (*defActorIt)->getTESFormId());
                    defActorIt = list.erase(defActorIt);
                    continue;
                }
                ret.push_back((*defActorIt));
                SKSE::log::trace("getNearestAggressorsForGangBang {:08X} added by lastHit - {:08X}",
                                 actor->getTESFormId(), (*defActorIt)->getTESFormId());
                defActorIt = list.erase(defActorIt);
                continue;
            }
            ++defActorIt;
        }
        // Search in candidateAgressorsForFollowers
        if (ret.size() == 0 && candidateAgressorsForFollowers.size() > 0 && list.size() > 0) {
            defActorIt = list.begin();
            while (defActorIt != list.end()) {
                if (auto search = candidateAgressorsForFollowers.find((*defActorIt)->getTESFormId());
                    search != candidateAgressorsForFollowers.end()) {
                    if (!IsSexualAssaulAllowedByAggressor(actor, (*defActorIt).get())) {
                        SKSE::log::trace(
                            "getNearestAggressorsForGangBang {:08X} !IsSexualAssaulAllowedByAggressor for followers skip - {:08X}",
                            actor->getTESFormId(), (*defActorIt)->getTESFormId());
                        defActorIt = list.erase(defActorIt);
                        continue;
                    }
                    ret.push_back((*defActorIt));
                    defActorIt = list.erase(defActorIt);
                    continue;
                }
                ++defActorIt;
            }
        }
        // Search in player agressors
        if (ret.size() == 0 && isFollower && list.size() > 0) {
            auto playerLastHitAggressors = getPlayer()->getLastHitAggressors();
            if (playerLastHitAggressors.size() > 0) {
                defActorIt = list.begin();
                while (defActorIt != list.end()) {
                    if (auto search = playerLastHitAggressors.find((*defActorIt)->getTESFormId());
                        search != playerLastHitAggressors.end()) {
                        if (!IsSexualAssaulAllowedByAggressor(actor, (*defActorIt).get())) {
                            SKSE::log::trace(
                                "getNearestAggressorsForGangBang {:08X} !IsSexualAssaulAllowedByAggressor for "
                                "player skip - {:08X}",
                                actor->getTESFormId(), (*defActorIt)->getTESFormId());
                            defActorIt = list.erase(defActorIt);
                            continue;
                        }
                        ret.push_back((*defActorIt));
                        defActorIt = list.erase(defActorIt);
                        continue;
                    }
                    ++defActorIt;
                }
            }
        }
        // Search potential followers
        if (ret.size() == 0 && !isFollower) {
            bool hasHitToFollower = false;
            if (followers.size() > 0) {
                std::list<DefeatActorType>::iterator followerIt = followers.begin();
                while (followerIt != followers.end()) {
                    auto lastHitAggressors = (*followerIt)->getLastHitAggressors();
                    if (lastHitAggressors.find(actor->getTESFormId()) != lastHitAggressors.end()) {
                        hasHitToFollower = true;
                        if (!IsSexualAssaulAllowedByAggressor(actor, (*followerIt).get())) {
                            SKSE::log::trace(
                                "getNearestAggressorsForGangBang {:08X} !IsSexualAssaulAllowedByAggressor by "
                                "follower after hit skip - {:08X}",
                                actor->getTESFormId(), (*followerIt)->getTESFormId());
                            followerIt = followers.erase(followerIt);
                            continue;
                        }
                        ret.push_back((*followerIt));
                        followerIt = followers.erase(followerIt);
                        continue;
                    }
                    ++followerIt;
                }
            }
            if (ret.size() == 0 && hasHitToFollower && followers.size() > 0) {
                std::list<DefeatActorType>::iterator followerIt = followers.begin();
                while (followerIt != followers.end()) {
                    if (!IsSexualAssaulAllowedByAggressor(actor, (*followerIt).get())) {
                        SKSE::log::trace(
                            "getNearestAggressorsForGangBang {:08X} !IsSexualAssaulAllowedByAggressor by "
                            "follower skip - {:08X}",
                            actor->getTESFormId(), (*followerIt)->getTESFormId());
                        followerIt = followers.erase(followerIt);
                        continue;
                    }
                    ret.push_back((*followerIt));
                    followerIt = followers.erase(followerIt);
                }
            }
        }

        return ret;
    }
    std::list<DefeatActorType> DefeatActorManager::getNearestFollowers(DefeatActor* actor) {
        auto ret = getNearestActorsInRangeByFilter(actor, 0, [&](RE::Actor* a_actor) {
            if (a_actor->IsPlayerTeammate() ||
                a_actor->IsInFaction(_defeatManager->Forms.Faction.CurrentFollowerFaction) ||
                a_actor->IsInFaction(_defeatManager->Forms.Faction.CurrentHireling)) {
                return true;
            }
            return false;
        });
        return ret;
    }
    std::list<DefeatActorType> DefeatActorManager::getNearestActorsInRangeByFilter(
        DefeatActor* actor, float a_range, std::function<bool(RE::Actor* a_actor)> a_callback) {
        auto ret = std::list<DefeatActorType>();
        forEachActorsInRange(actor->getTESActor(), a_range, [&](RE::Actor* a_actor) {
            if (a_callback(a_actor)) {
                auto defeatActor = getDefeatActor(a_actor);
                if (defeatActor) {
                    ret.push_back(defeatActor);
                }
            }
            return true;
        });
        return ret;
    }
    float DefeatActorManager::getHeadingAngleBetween(DefeatActor* source, DefeatActor* target) {
        auto a1 = target->getTESActor()->GetPosition();
        return source->getTESActor()->GetHeadingAngle(a1, false);
    }
    float DefeatActorManager::getActorValuePercentage(DefeatActor* source, RE::ActorValue av) {
        auto actor = source->getTESActor();
        auto actorAV = actor->AsActorValueOwner();

        auto temporary = actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, av);
        auto total = actorAV->GetPermanentActorValue(av);
        auto current = actorAV->GetActorValue(av);

        return total > 0 ? current / (total + temporary) : 1.0;
    }
    RE::TESForm* DefeatActorManager::getEquippedHitSourceByFormID(DefeatActor* source, RE::FormID hitSource) {
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

    bool DefeatActorManager::wornHasAnyKeyword(DefeatActor* source, std::list<std::string> kwds) {
        auto actor = source->getTESActor();

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
    bool DefeatActorManager::hasKeywordString(DefeatActor* source, std::string_view kwd) {
        return source->getTESActor()->HasKeywordString(kwd);
    }
    bool DefeatActorManager::isInFaction(DefeatActor* actor, RE::TESFaction* faction) { 
        return actor->getTESActor()->IsInFaction(faction);
    }
    bool DefeatActorManager::isInFaction(DefeatActor* actor, RE::BGSListForm* faction) {
        return actor->getTESActor()->VisitFactions([&](RE::TESFaction* _faction, uint8_t rank) {
            if (!_faction || rank < 0) return false;

            return faction->HasForm(_faction);
        });
    }
    bool DefeatActorManager::hasCombatTarget(DefeatActor* source, DefeatActor* target) { 
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
    bool DefeatActorManager::notInFlyingState(DefeatActor* source) {
        return source->getTESActor()->AsActorState()->GetFlyState() == RE::FLY_STATE::kNone;
    }
    bool DefeatActorManager::hasSpell(DefeatActor* source, RE::SpellItem* spell) {
        return spell != nullptr && source->getTESActor()->HasSpell(spell);
    }
    bool DefeatActorManager::hasMagicEffect(DefeatActor* source, RE::EffectSetting* effect) {
        return effect != nullptr && source->getTESActor()->GetMagicTarget()->HasMagicEffect(effect);
    }
    bool DefeatActorManager::isInKillMove(DefeatActor* source) { return source->getTESActor()->IsInKillMove(); }
    bool DefeatActorManager::isQuestEnabled(RE::TESQuest* quest) { return quest != nullptr && quest->IsEnabled(); }
    bool DefeatActorManager::isInCombat(DefeatActor* source) { return source->getTESActor()->IsInCombat(); }
    bool DefeatActorManager::IsHostileToActor(DefeatActor* source, DefeatActor* target) {
        return source->getTESActor()->IsHostileToActor(target->getTESActor());
    }
    bool DefeatActorManager::isCommandedActor(DefeatActor* source) {
        return source->getTESActor()->IsCommandedActor();
    }
    bool DefeatActorManager::isPlayerTeammate(DefeatActor* source) {
        return source->getTESActor()->IsPlayerTeammate();
    }

    void DefeatRequestCallback::operator()(RE::BSScript::Variable a_result) {
        //auto process = RE::ProcessLists::GetSingleton();
        //process->runDetection = false;
        //process->ClearCachedFactionFightReactions();
        //process->runDetection = true;
    }

}