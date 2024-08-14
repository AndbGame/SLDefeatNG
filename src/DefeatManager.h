#pragma once

#include <Defeat.h>

#include "DefeatActorManager.h"
#include "DefeatCombatManager.h"
#include "DefeatScene.h"
#include "DefeatActor.h"
#include "DefeatSpinLock.h"
#include "DefeatWidget.h"
#include "DefeatConfig.h"
#include "DefeatScene.h"

namespace SexLabDefeat {

    class DefeatManager : public IDefeatManager {
    public:
        static std::map<DefeatActorStates, std::string> DefeatActorStatesStrings;

        DefeatManager(DefeatConfig* defeatConfig);
        DefeatManager(DefeatManager const&) = delete;
        void operator=(DefeatManager const& x) = delete;

        GameState getGameState() override;
        void setGameState(GameState state) override;
        DefeatActorStates getStateByString(std::string state) override;

        DefeatWidget* getWidget() override;
        DefeatCombatManager* getCombatManager() override { return _defeatCombatManager; };
        DefeatSceneManager* getSceneManager() override { return _defeatSceneManager; };
        DefeatActorManager* getActorManager() override { return _defeatActorManager; };
        DefeatConfig* getConfig() override { return _defeatConfig; };
        bool hasTraceLog() override { return getConfig()->CFG_LOGGING > 1; };

        void load();
        void reset();
        void reInitializeWidget();
        PapyrusInterface::ObjectPtr getDefeatQTEWidgetScript() const;
        void setWidget(DefeatWidget* widget);
        void setActorState(RE::Actor* target_actor, DefeatActorStates state, bool isTransition = true);
        bool tryExchangeActorState(RE::Actor* target_actor, DefeatActorStates oldState, DefeatActorStates newState,
                                   bool isTransition = true);

    protected:
        void initializeDependency();
        void initializeForms();

        std::atomic<GameState> _gameState;
        DefeatConfig* _defeatConfig;
        DefeatCombatManager* _defeatCombatManager;
        DefeatSceneManager* _defeatSceneManager;
        DefeatActorManager* _defeatActorManager;
        DefeatWidget* _defeatWidget = nullptr;
    };
}