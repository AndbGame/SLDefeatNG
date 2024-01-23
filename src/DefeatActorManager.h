#pragma once

#include <Defeat.h>

#include "DefeatActor.h"
#include "PapyrusInterface\ActorExtraDataCallQueue.h"

namespace SexLabDefeat {

    class DefeatActorManager : public IDefeatActorManager {
    public:
        DefeatActorManager(IDefeatManager* defeatManager) : _defeatManager(defeatManager) {};
        ~DefeatActorManager() = default;
        DefeatActorManager(DefeatActorManager const&) = delete;
        void operator=(DefeatActorManager const& x) = delete;

        void reset();

        DefeatPlayerActorImplType getPlayerImpl() override { return _player; }
        DefeatPlayerActorType getPlayer(RE::Actor* actor = nullptr);

        DefeatActorImplType getDefeatActorImpl(RE::Actor* actor) override;
        DefeatActorType getDefeatActor(RE::Actor* actor) override;

        bool validPlayerForVictimRole(RE::Actor* actor) override;
        bool hasSexInterestByAggressor(DefeatActorType target, DefeatActorType aggressor) override;
        bool hasSexCombinationWithAggressor(DefeatActorType target, DefeatActorType aggressor) override;
        bool checkAggressor(DefeatActorType target, DefeatActorType aggressor) override;

        void playerKnockDownEvent(DefeatActorType target, DefeatActorType aggressor, HitResult event) override;

        DefeatConfig* getConfig() override;
        DefeatForms getForms() override;
        SoftDependencyType getSoftDependency() override;

    protected:
        std::map<RE::FormID, DefeatActorImplType> _actorMap;
        DefeatPlayerActorImplType _player;

        IDefeatManager* _defeatManager;
    };
}