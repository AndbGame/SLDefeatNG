#pragma once

#include "Defeat.h"

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

        HitEventType createHitEvent(RE::Actor* target_actor, RE::Actor* aggr_actor, RawHitEvent rawHitEvent);
        void setWidget(SexLabDefeat::DefeatWidget* widget);
        SexLabDefeat::DefeatWidget* getWidget();
        void setActorState(RE::Actor* target_actor, DefeatActor::States state);

        SexLabDefeat::DefeatCombatManager* getCombatManager() { return _defeatCombatManager; };
        SexLabDefeat::DefeatActorManager* getActorManager() { return _defeatActorManager; };
        SexLabDefeat::DefeatConfig* getConfig() { return _defeatConfig; };

    protected:
        void initializeDependency();
        void initializeForms();

        std::atomic<GameState> _gameState;
        SexLabDefeat::DefeatConfig* _defeatConfig;
        SexLabDefeat::DefeatCombatManager* _defeatCombatManager;
        SexLabDefeat::DefeatActorManager* _defeatActorManager;
        SexLabDefeat::DefeatWidget* _defeatWidget = nullptr;
    };
}