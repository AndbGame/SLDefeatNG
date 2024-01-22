#pragma once

#include <Defeat.h>

namespace SexLabDefeat {

    class DefeatActorManager : public SpinLock {
    public:
        DefeatActorManager(DefeatIManager* defeatManager) /* : SpinLock()*/ { _defeatManager = defeatManager; };
        ~DefeatActorManager() = default;

        void reset();

        DefeatActorType getPlayer() { return _player; }

        DefeatActorType getActor(RE::Actor* actor);

        /* Pre Checks functions */
        bool validForAggressorRole(RE::Actor* actor);
        bool validForAggressorRoleOverPlayer(RE::Actor* actor);
        bool validPlayerForVictimRole(RE::Actor* actor);
        /* / Pre Checks functions  */

    protected:
        std::map<RE::FormID, DefeatActorType> _actorMap;

        DefeatActorType _player;
        SexLabDefeat::DefeatIManager* _defeatManager;
    };
}
