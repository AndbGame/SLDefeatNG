#pragma once

#include <Defeat.h>

#include "DefeatUtils.h"
#include "DefeatSpinLock.h"
#include "PapyrusInterface\ObjectVariable.h"

namespace SexLabDefeat {

    class DefeatCombatManager : public IDefeatCombatManager {
    public:
        DefeatCombatManager(IDefeatActorManager* defeatActorManager, IDefeatManager* defeatManager);
        ~DefeatCombatManager();
        DefeatCombatManager(DefeatCombatManager const&) = delete;
        void operator=(DefeatCombatManager const& x) = delete;

        IDefeatManager* getDefeatManager() { return _defeatManager; };

        void onActorEnteredToCombatState(RE::Actor* target_actor) override;
        void onHitHandler(RawHitEvent event) override;

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
        IDefeatActorManager* _defeatActorManager;
        IDefeatManager* _defeatManager;

        void onPlayerHitHandler(HitEvent event, DefeatPlayerActorType defActor, DefeatActorType source);
        void onNvNHitHandler(HitEvent event, DefeatActorType defActor, DefeatActorType source);
    };
}