#pragma once

#include <Defeat.h>

#include "DefeatUtils.h"
#include "DefeatSpinLock.h"
#include "DefeatConfig.h"
#include "DefeatActorManager.h"

namespace SexLabDefeat {
    class DefeatManager;

    class DefeatCombatManager : public IDefeatCombatManager {
    public:
        DefeatCombatManager(DefeatActorManager* defeatActorManager, DefeatManager* defeatManager);
        ~DefeatCombatManager();
        DefeatCombatManager(DefeatCombatManager const&) = delete;
        void operator=(DefeatCombatManager const& x) = delete;

        DefeatManager* getDefeatManager() { return _defeatManager; };

        void onActorEnteredToCombatState(RE::Actor* actor, RE::Actor* target_actor) override;
        void onActorEnteredToNonCombatState(RE::Actor* actor) override;
        void onActorEnterBleedout(RE::Actor* target_actor) override;
        void onHitHandler(RawHitEvent event) override;
        void onSexLabSceneInterrupt(DefeatActorType target, DefeatActorType source, bool isHit = false);

        HitEventType createHitEvent(DefeatActorType target_actor, DefeatActorType aggr_actor, RawHitEvent rawHitEvent);

        void calculatePlayerHit(HitEventType event);
        HitResult KDWay(HitEventType event);
        HitResult KDWayWound(HitEventType event);
        HitResult KDWayExhaustion(HitEventType event);
        HitResult KDWayVulnerability(HitEventType event);
        HitResult KDWayDynamic(HitEventType event);
        float KDWayDynamicCalculation(HitEventType event);
        HitResult KDWayPowerAtk(HitEventType event);

        void shedulePlayerDeplateDynamicDefeat();
        std::atomic<bool> _playerDeplateDynamicDefeatStopThread = true;
        void interruptPlayerDeplateDynamicDefeat();

        bool KDOnlyBack(bool opt, HitEventType event);

    protected:
        DefeatActorManager* _defeatActorManager;
        DefeatManager* _defeatManager;

        void onPlayerHitHandler(HitEvent event, DefeatActorType defActor, DefeatActorType source);
        void onNvNHitHandler(HitEvent event, DefeatActorType defActor, DefeatActorType source);
    };
}