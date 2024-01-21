#pragma once

#include <DefeatActor.h>
#include <DefeatWidget.h>
#include <DefeatCombatManager.h>

namespace SexLabDefeat {

    class DefeatManager {
    public:
        enum GameState {
            NONE,
            PRE_LOAD,
            IN_GAME,
        };

        struct SoftDependencyType {
            bool ZaZ = false;
            bool DeviousFramework = false;
            bool LRGPatch = false;
        } SoftDependency;

        DefeatManager(SexLabDefeat::DefeatConfig* defeatConfig);
        DefeatManager(DefeatManager const&) = delete;
        void operator=(DefeatManager const& x) = delete;

        void load();
        void reset();
        void reInitializeWidget() const;
        GameState getGameState();
        void setGameState(GameState state);
        void ActorEnterdToCombatState(RE::Actor* target_actor);
        PapyrusInterface::ObjectPtr getDefeatQTEWidgetScript() const;

        bool randomChanse(float chanse, float min = 1, float max = 100);

        DefeatForms Forms;

        void setWidget(DefeatWidget* widget);
        DefeatWidget* getWidget();
        void setActorState(RE::Actor* target_actor, DefeatActor::States state);

        DefeatCombatManager* getCombatManager() { return _defeatCombatManager; };
        DefeatActorManager* getActorManager() { return _defeatActorManager; };
        DefeatConfig* getConfig() { return _defeatConfig; };

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