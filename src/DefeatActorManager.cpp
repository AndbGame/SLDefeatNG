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

    DefeatActorType DefeatActorManager::getDefeatActor(RE::Actor* actor) {
        assert(actor != nullptr);
        auto defPlayerImpl = getDefeatActorImpl(actor);
        SexLabDefeat::UniqueSpinLock lock(*defPlayerImpl);
        return std::make_shared<DefeatPlayerActor>(defPlayerImpl->_data, actor, defPlayerImpl);
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
        auto vm = RE::SkyrimVM::GetSingleton();
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
        }
    }

    bool DefeatActorManager::isIgnored(RE::Actor* actor) {
        for (RE::TESFaction* faction : _defeatManager->Forms.Ignore.Factions) {
            if (actor->IsInFaction(faction)) {
                return true;
            }
        };
        return false;
    }

    bool IDefeatActorManager::validForAggressorRole(RE::Actor* actor) {
        if (actor == nullptr || actor->IsGhost()) {
            return false;
        }
        return true;
    }

    bool IDefeatActorManager::validForVictrimRole(RE::Actor* actor) {
        if (actor == nullptr || actor->IsGhost()) {
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
    bool IDefeatActorManager::isPlayerTeammate(DefeatActorType source) { return isPlayerTeammate(*source); }
    bool IDefeatActorManager::isPlayerTeammate(DefeatActor& source) {
        return source.getTESActor()->IsPlayerTeammate();
    }

}