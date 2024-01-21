#pragma once

#include <DefeatForms.h>
#include <DefeatActor.h>
#include <DefeatManager.h>

namespace SexLabDefeat {

    class DefeatActorManager : public SexLabDefeat::SpinLock {
    public:
        DefeatActorManager(DefeatManager* defeatManager) {
            _defeatManager = defeatManager;
        };
        ~DefeatActorManager() = default;
        DefeatActorManager(DefeatActorManager const&) = delete;
        void operator=(DefeatActorManager const& x) = delete;

        void reset() {
            SexLabDefeat::UniqueSpinLock lock(*this);
            _actorMap.clear();

            auto actor = RE::PlayerCharacter::GetSingleton();
            DefeatPlayerActorImplType defeatActor = std::make_shared<DefeatPlayerActorImpl>(actor, _defeatManager);
            _actorMap.emplace(actor->GetFormID(), defeatActor);
            _player = defeatActor;
        }

        DefeatPlayerActorImplType getPlayerImpl() { return _player; }
        DefeatPlayerActorType getPlayer(RE::Actor* actor) {
            auto defPlayer = getPlayerImpl();
            defPlayer->spinLock();
            auto obj = DefeatPlayerActorType(new DefeatPlayerActor(defPlayer->_data, actor, defPlayer));
            defPlayer->spinUnlock();
            return obj;
        }

        DefeatActorImplType getDefeatActorImpl(RE::Actor* actor) {
            assert(actor != nullptr);
            auto formID = actor->GetFormID();

            SexLabDefeat::UniqueSpinLock lock(*this);
            auto val = _actorMap.find(formID);
            DefeatActorImplType defeatActorImpl;
            if (val == _actorMap.end()) {
                defeatActorImpl = std::make_shared<DefeatActorImpl>(formID, this);
                _actorMap.emplace(formID, defeatActorImpl);
            } else {
                defeatActorImpl = val->second;
            }
            return defeatActorImpl;
        }

        DefeatActorType getDefeatActor(RE::Actor* actor) { 
            return constructDefeatActor(getDefeatActorImpl(actor), actor);
        }

//----- DefeatActorType Utils

        inline bool isDefeatAllowedByAgressor(DefeatActorType target, DefeatActorType aggressor) {
            if (target->isPlayer()) {
                // SKSE::log::trace("isDefeatAllowedByAgressor {}", aggressor->isDefeatAllowed2PC());
                return aggressor->isDefeatAllowed2PC();
            }
            return aggressor->isDefeatAllowed2NvN();
        };

        inline bool IsSexualAssaulAllowedByAggressor(DefeatActorType target, DefeatActorType aggressor) {
            return hasSexInterestByAggressor(target, aggressor) && hasSexCombinationWithAggressor(target, aggressor);
        };

        inline bool hasSexInterestByAggressor(DefeatActorType target, DefeatActorType aggressor) {
            if (aggressor->isSatisfied()) {
                SKSE::log::trace("CheckAggressor: isSatisfied");
                return false;
            }
            if (target->isPlayer()) {
                return _defeatManager->randomChanse(_defeatManager->getConfig()->Config.PvicRaped->get());
            }
            if (target->isFollower()) {
                return _defeatManager->randomChanse(_defeatManager->getConfig()->Config.NVNRapedFollower->get());
            }
            return _defeatManager->randomChanse(_defeatManager->getConfig()->Config.NVNRaped->get());
        }

        inline bool hasSexCombinationWithAggressor(DefeatActorType target, DefeatActorType aggressor) {
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
                    if (aggressor->notInFlyingState()) {
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
                            return mcmConfig->Config.MaleOnMale->get() &&
                                   (aggressor->IsGay() || aggressor->IsBisexual());
                        }
                    }
                } else {
                    if (aggressor->notInFlyingState()) {
                        if (!mcmConfig->Config.SexLab.UseCreatureGender->get()) {
                            return true;
                        }
                        auto aggressorFemale = aggressor->getSexLabGender() == 3;
                        if (!aggressorFemale && victimFemale &&
                            mcmConfig->Config.CreaOnFemale->get()) {  // Male on Female
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

        inline bool checkAggressor(DefeatActorType target, DefeatActorType aggressor) {
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
        
        void requestActorExtraData(DefeatActorType target) {
            SexLabDefeat::Papyrus::CallbackPtr callback(
                new SexLabDefeat::PapyrusInterface::EmptyRequestCallback("DeferredActorExtraDataInitializer"));

            SKSE::log::trace("DeferredActorExtraDataInitializer - <{:08X}>", target->getTESFormId());

            if (!SexLabDefeat::Papyrus::DispatchStaticCall("defeat_skse_api", "requestActorExtraData", callback,
                                                           target->getTESActor())) {
                SKSE::log::error("Failed to dispatch static call [defeat_skse_api::requestActorExtraData].");
            }
        };

        DefeatConfig* getConfig();
        DefeatForms getForms() const;
        DefeatManager::SoftDependencyType getSoftDependency() const;

//----- TESActor Utils

        inline bool validForAggressorRole(RE::Actor* actor) {
            if (actor == nullptr || actor->IsGhost()) {
                return false;
            }
            return true;
        }

        inline bool validForAggressorRoleOverPlayer(RE::Actor* actor) {
            if (actor == nullptr || actor->IsGhost()) {
                return false;
            }
            return true;
        }

        inline bool validPlayerForVictimRole(RE::Actor* actor) {
            if (actor == nullptr || actor->IsOnMount() || actor->HasKeywordString("DefeatActive")) {
                return false;
            }
            if (!actor->HasKeywordString("ActorTypeNPC")) {
                return _defeatManager->getConfig()->Config.BeastImmunity->get();
            }
            return true;
        }

        static inline float getDistanceBetween(DefeatActorType source, DefeatActorType target) {
            auto a1 = source->getTESActor()->GetPosition();
            auto a2 = target->getTESActor()->GetPosition();
            return a1.GetDistance(a2);
        }
        static inline float getHeadingAngleBetween(DefeatActor& source, DefeatActor& target) {
            auto a1 = (&target)->getTESActor()->GetPosition();
            return (&source)->getTESActor()->GetHeadingAngle(a1, false);
        }
        static inline float getActorValuePercentage(DefeatActor& source, RE::ActorValue av) {
            auto actor = (&source)->getTESActor();
            auto actorAV = actor->AsActorValueOwner();

            auto temporary = actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, av);
            auto total = actorAV->GetPermanentActorValue(av);
            auto current = actorAV->GetActorValue(av);

            return total > 0 ? current / (total + temporary) : 1.0;
        }
        static inline RE::TESForm* getEquippedHitSourceByFormID(DefeatActor& source, RE::FormID hitSource) {
            auto actor = (&source)->getTESActor();
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
        static inline bool wornHasAnyKeyword(DefeatActor& source, std::list<std::string> kwds) {
            auto actor = (&source)->getTESActor();

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
        static inline bool hasKeywordString(DefeatActor& source, std::string kwd) {
            return (&source)->getTESActor()->HasKeywordString(kwd);
        }
        static inline bool notInFlyingState(DefeatActor& source) {
            return (&source)->getTESActor()->AsActorState()->GetFlyState() == RE::FLY_STATE::kNone;
        }
        static inline bool hasSpell(DefeatActor& source, RE::SpellItem* spell) {
            return spell != nullptr && (&source)->getTESActor()->HasSpell(spell);
        }
        static inline bool hasMagicEffect(DefeatActor& source, RE::EffectSetting* effect) {
            return effect != nullptr && (&source)->getTESActor()->GetMagicTarget()->HasMagicEffect(effect);
        }
        static inline bool isInKillMove(DefeatActor& source) {
            return (&source)->getTESActor()->IsInKillMove();
        }
        static inline bool isQuestEnabled(RE::TESQuest* quest) {
            return quest != nullptr && quest->IsEnabled();
        }

    protected:
        std::map<RE::FormID, DefeatActorImplType> _actorMap;

        DefeatPlayerActorImplType _player;
        DefeatManager* _defeatManager;

        DefeatActorType constructDefeatActor(DefeatActorImplType actorImpl, RE::Actor* actor) {
            assert(actor != nullptr);
            actorImpl->spinLock();
            auto obj = DefeatActorType(new DefeatActor(actorImpl->_data, actor, actorImpl));
            actorImpl->spinUnlock();
            return obj;
        }
    };
}