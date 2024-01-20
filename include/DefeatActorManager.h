#pragma once

#include <DefeatActor.h>

namespace SexLabDefeat_TODO {
    
    class DefeatManager;

    class DefeatActorManager : public SexLabDefeat::SpinLock {
    public:
        DefeatActorManager(DefeatManager* defeatManager) {
            _defeatManager = defeatManager;
        };
        ~DefeatActorManager() = default;
        DefeatActorManager(DefeatActorManager const&) = delete;
        void operator=(DefeatActorManager const& x) = delete;

        void reset();

        DefeatActorType getPlayer() { return _player; }

        DefeatActorType getDefeatActor(RE::Actor* actor) { 
            assert(actor != nullptr);
            auto formID = actor->GetFormID();

            spinLock();
            auto val = _actorMap.find(formID);
            DefeatActorImplType defeatActorImpl;
            if (val == _actorMap.end()) {
                defeatActorImpl = std::make_shared<DefeatActorImpl>(formID, this);
                _actorMap.emplace(formID, defeatActorImpl);
            } else {
                defeatActorImpl = val->second;
            }
            spinUnlock();
            return constructDefeatActor(defeatActorImpl, actor);
        }

        /* Pre Checks functions */
        bool validForAggressorRole(RE::Actor* actor);
        bool validForAggressorRoleOverPlayer(RE::Actor* actor);
        bool validPlayerForVictimRole(RE::Actor* actor);
        /* / Pre Checks functions  */

//----- Utils

        static inline float getDistanceBetween(DefeatActor& source, DefeatActor& target) {
            auto a1 = (&source)->getTESActor()->GetPosition();
            auto a2 = (&target)->getTESActor()->GetPosition();
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

    protected:
        std::map<RE::FormID, DefeatActorImplType> _actorMap;

        DefeatActorType _player;
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