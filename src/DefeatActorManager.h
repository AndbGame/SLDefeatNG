#pragma once

#include <Defeat.h>

#include "DefeatActor.h"
#include "PapyrusInterface\ActorExtraDataCallQueue.h"
#include "DefeatUtils.h"
#include "DefeatSpinLock.h"

namespace SexLabDefeat {

    class DefeatActorManager : public IDefeatActorManager, public SpinLock {
    public:
        DefeatActorManager(IDefeatManager* defeatManager) : _defeatManager(defeatManager) {};
        ~DefeatActorManager() = default;
        DefeatActorManager(DefeatActorManager const&) = delete;
        void operator=(DefeatActorManager const& x) = delete;

        void reset();

        std::shared_ptr<DefeatPlayerActorImpl> getPlayerImpl() { return _player; }
        DefeatPlayerActorType getPlayer(RE::Actor* actor = nullptr) override;

        std::shared_ptr<DefeatActorImpl> getDefeatActorImpl(RE::Actor* actor);
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
        std::map<RE::FormID, std::shared_ptr<DefeatActorImpl>> _actorMap;
        std::shared_ptr<DefeatPlayerActorImpl> _player;

        IDefeatManager* _defeatManager;
    };
}