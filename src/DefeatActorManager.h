#pragma once

#include <Defeat.h>

#include "PapyrusInterface\ActorExtraDataCallQueue.h"

namespace SexLabDefeat {
    class DefeatActorManager : public IDefeatActorManager {
    public:
        DefeatActorManager(IDefeatManager* defeatManager) { _defeatManager = defeatManager; };
        ~DefeatActorManager() = default;
        DefeatActorManager(DefeatActorManager const&) = delete;
        void operator=(DefeatActorManager const& x) = delete;

        void reset();

        DefeatActorType getPlayer() override { return _player; }

        DefeatActorType getActor(RE::Actor* actor) override;

        bool validPlayerForVictimRole(RE::Actor* actor) override;
        bool hasSexInterestByAggressor(DefeatActorType target, DefeatActorType aggressor) override;
        bool hasSexCombinationWithAggressor(DefeatActorType target, DefeatActorType aggressor) override;
        bool checkAggressor(DefeatActorType target, DefeatActorType aggressor) override;

    protected:
        std::map<RE::FormID, DefeatActorType> _actorMap;
        DefeatActorType _player;

        IDefeatManager* _defeatManager;
    };
}