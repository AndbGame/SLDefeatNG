#pragma once

#include <Defeat.h>
#include "DefeatActorManager.h"

namespace SexLabDefeat {

    class DefeatCombatManager {
    public:
        struct HitSpamKey {
            RE::FormID actor;
            RE::FormID source;
        };
        struct ProjectileSpamHash {
            std::size_t operator()(const HitSpamKey& k) const {
                return std::hash<std::uint32_t>()(k.actor) ^ (std::hash<std::uint32_t>()(k.source) << 1);
            }
        };

        struct HitSpamEqual {
            bool operator()(const HitSpamKey& lhs, const HitSpamKey& rhs) const {
                return lhs.actor == rhs.actor && lhs.source == rhs.source;
            }
        };

        RE::BSFixedString* OnSLDefeatPlayerKnockDownEventName;

        DefeatCombatManager(DefeatActorManager* defeatActorManager, DefeatIManager* defeatManager);
        ~DefeatCombatManager();

        DefeatIManager* getDefeatManager() { return _defeatManager; };

        void onHitHandler(RawHitEvent event);

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
        bool registerAndCheckHitGuard(RE::TESObjectREFR* actor, RE::FormID source, RE::FormID projectile);

    protected:
        SexLabDefeat::DefeatActorManager* _defeatActorManager;
        SexLabDefeat::DefeatIManager* _defeatManager;
        // std::unordered_map<HitSpamKey, std::chrono::high_resolution_clock::time_point, ProjectileSpamHash,
        // HitSpamEqual>
        //     projectileSpamGuard;
        std::unordered_map<HitSpamKey, std::chrono::high_resolution_clock::time_point, ProjectileSpamHash, HitSpamEqual>
            hitSpamGuard;
        std::chrono::milliseconds _hitGuardExpiration;
        SpinLock* hitSpamGuardSpinLock;

        void onPlayerHitHandler(RawHitEvent event, SexLabDefeat::DefeatActorType defActor);
    };
}