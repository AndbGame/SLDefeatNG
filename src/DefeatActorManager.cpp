#include "DefeatActorManager.h"

namespace SexLabDefeat {

    DefeatActorType DefeatActorManager::getActor(RE::Actor* actor) {
        spinLock();
        auto val = _actorMap.find(actor->GetFormID());
        DefeatActorType defeatActor;
        if (val == _actorMap.end()) {
            defeatActor = std::make_shared<DefeatActor>(actor, _defeatManager);
            _actorMap.emplace(actor->GetFormID(), defeatActor);
        } else {
            defeatActor = val->second;
        }
        defeatActor->setActor(actor);
        spinUnlock();
        return defeatActor;
    }

    void DefeatActorManager::reset() {
        spinLock();
        _actorMap.clear();

        auto actor = RE::PlayerCharacter::GetSingleton();
        DefeatActorType defeatActor = std::make_shared<DefetPlayerActor>(actor, _defeatManager);
        _actorMap.emplace(actor->GetFormID(), defeatActor);
        _player = defeatActor;

        spinUnlock();
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

    bool IDefeatActorManager::validForAggressorRole(RE::Actor* actor) {
        if (actor == nullptr || actor->IsGhost()) {
            return false;
        }
        return true;
    }

    bool IDefeatActorManager::validForAggressorRoleOverPlayer(RE::Actor* actor) {
        if (actor == nullptr || actor->IsGhost()) {
            return false;
        }
        return true;
    }

    bool DefeatActorManager::validPlayerForVictimRole(RE::Actor* actor) {
        if (actor == nullptr || actor->IsOnMount() || actor->HasKeywordString("DefeatActive")) {
            return false;
        }
        if (!actor->HasKeywordString("ActorTypeNPC")) {
            return _defeatManager->getConfig()->Config.BeastImmunity->get();
        }
        return true;
    }
    float IDefeatActorManager::getDistanceBetween(DefeatActorType source, DefeatActorType target) {
        auto a1 = source->getActor()->GetPosition();
        auto a2 = target->getActor()->GetPosition();
        return a1.GetDistance(a2);
    }
    float IDefeatActorManager::getHeadingAngleBetween(DefeatActorType source, DefeatActorType target) {
        auto a1 = target->getActor()->GetPosition();
        return source->getActor()->GetHeadingAngle(a1, false);
    }
    float IDefeatActorManager::getActorValuePercentage(DefeatActorType source, RE::ActorValue av) {
        auto actor = source->getActor();
        auto actorAV = actor->AsActorValueOwner();

        auto temporary = actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, av);
        auto total = actorAV->GetPermanentActorValue(av);
        auto current = actorAV->GetActorValue(av);

        return total > 0 ? current / (total + temporary) : 1.0;
    }
    RE::TESForm* IDefeatActorManager::getEquippedHitSourceByFormID(DefeatActorType source, RE::FormID hitSource) {
        auto actor = source->getActor();
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
        auto actor = source.getActor();

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
    bool IDefeatActorManager::hasKeywordString(DefeatActorType source, std::string kwd) {
        return source->getActor()->HasKeywordString(kwd);
    }
    bool IDefeatActorManager::notInFlyingState(DefeatActorType source) {
        return notInFlyingState(*source);
    }
    bool IDefeatActorManager::notInFlyingState(DefeatActor& source) {
        return source.getActor()->AsActorState()->GetFlyState() == RE::FLY_STATE::kNone;
    }
    bool IDefeatActorManager::hasSpell(DefeatActorType source, RE::SpellItem* spell) {
        return spell != nullptr && source->getActor()->HasSpell(spell);
    }
    bool IDefeatActorManager::hasMagicEffect(DefeatActorType source, RE::EffectSetting* effect) {
        return effect != nullptr && source->getActor()->GetMagicTarget()->HasMagicEffect(effect);
    }
    bool IDefeatActorManager::isInKillMove(DefeatActorType source) { return source->getActor()->IsInKillMove(); }
    bool IDefeatActorManager::isQuestEnabled(RE::TESQuest* quest) { return quest != nullptr && quest->IsEnabled(); }
    bool IDefeatActorManager::isInCombat(DefeatActorType source) { return source->getActor()->IsInCombat(); }
}