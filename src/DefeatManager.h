#pragma once

#include <Defeat.h>

#include "DefeatActorManager.h"

namespace SexLabDefeat {

    class DefeatManager : public IDefeatManager {
    public:
        DefeatManager(DefeatConfig* defeatConfig);
        DefeatManager(DefeatManager const&) = delete;
        void operator=(DefeatManager const& x) = delete;

        GameState getGameState() override;
        void setGameState(GameState state) override;

        DefeatWidget* getWidget() override;
        DefeatCombatManager* getCombatManager() override { return _defeatCombatManager; };
        DefeatActorManager* getActorManager() override { return _defeatActorManager; };
        DefeatConfig* getConfig() override { return _defeatConfig; };
        //void requestActorExtraData(DefeatActorType target) override;

        void load();
        void reset();
        void reInitializeWidget();
        PapyrusInterface::ObjectPtr getDefeatQTEWidgetScript() const;
        void setWidget(DefeatWidget* widget);
        void setActorState(RE::Actor* target_actor, DefeatActorStates state);

    protected:
        void initializeDependency();
        void initializeForms();

        std::atomic<GameState> _gameState;
        DefeatConfig* _defeatConfig;
        DefeatCombatManager* _defeatCombatManager;
        DefeatActorManager* _defeatActorManager;
        DefeatWidget* _defeatWidget = nullptr;
    };
}