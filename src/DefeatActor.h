#pragma once

#include <Defeat.h>

#include "PapyrusInterface\ActorExtraDataCallQueue.h"
#include "DefeatSpinLock.h"
#include "DefeatConfig.h"

namespace SexLabDefeat {

    class DefeatActorManager;

    /***************************************************************************************************
     * DefeatActorImpl
     ****************************************************************************************************/

    class DefeatActorImpl : public IDefeatActor, public SpinLock {
        friend class DefeatActorManager;

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

        DefeatActorImpl(RE::FormID formID, DefeatActorManager* defeatActorManager) {
            _data.TESFormId = formID;
            _defeatActorManager = defeatActorManager;
            extradataQueue = new PapyrusInterface::ActorExtraDataCallQueue(this, 10min, 1min);
        };
        ~DefeatActorImpl() { delete extradataQueue; }
        DefeatActorImpl(DefeatActorImpl const&) = delete;
        void operator=(DefeatActorImpl const& x) = delete;

        PapyrusInterface::ActorExtraDataCallQueue* extradataQueue;

        void setHitImmunityFor(std::chrono::milliseconds ms) override {
            UniqueSpinLock lock(*this);
            _data.hitImmunityExpiration = clock::now() + ms;
        };
        void setLastHitAggressor(DefeatActorType lastHitAggressor) override;
        void clearLastHitAggressors() override {
            UniqueSpinLock lock(*this);
            _data.lastHitAggressors.clear();
        }
        void setInCombat() override {
            UniqueSpinLock lock(*this);
            _data.inCombat = true;
        };
        void setNotInCombat() override {
            UniqueSpinLock lock(*this);
            _data.inCombat = false;
        };

        float incrementDynamicDefeat(float val) override {
            UniqueSpinLock lock(*this);
            _data.dynamicDefeat += val;
            if (_data.dynamicDefeat > 1) {
                _data.dynamicDefeat = 1;
            }
            return _data.dynamicDefeat;
        }

        float decrementDynamicDefeat(float val) override {
            UniqueSpinLock lock(*this);
            _data.dynamicDefeat -= val;
            if (_data.dynamicDefeat < 0) {
                _data.dynamicDefeat = 0;
            }
            return _data.dynamicDefeat;
        }

        float resetDynamicDefeat() override {
            UniqueSpinLock lock(*this);
            return _data.dynamicDefeat = 0;
        }

        void setState(DefeatActorStates state) override {
            UniqueSpinLock lock(*this);
            _data.state = state;
        };
        void setVulnerability(float vulnerability) override {
            UniqueSpinLock lock(*this);
            _data.vulnerability = vulnerability;
            if (_data.vulnerability > 100) {
                _data.vulnerability = 100;
            }
        };
        void setDFWVulnerability(float vulnerability) override {
            UniqueSpinLock lock(*this);
            _data.DFWVulnerability = vulnerability;
            if (_data.DFWVulnerability > 100) {
                _data.DFWVulnerability = 100;
            }
        };

        void setIgnoreActorOnHit(bool val) override {
            UniqueSpinLock lock(*this);
            _data.ignoreActorOnHit = val;
        };

        void setSexLabGender(int val) override {
            UniqueSpinLock lock(*this);
            _data.sexLabGender = val;
        };

        void setSexLabSexuality(int val) override {
            UniqueSpinLock lock(*this);
            _data.sexLabSexuality = val;
        };

        void setSexLabAllowed(bool val) override {
            UniqueSpinLock lock(*this);
            _data.sexLabAllowed = val;
        }

        void setSexLabRaceKey(std::string val) override {
            UniqueSpinLock lock(*this);
            _data.sexLabRaceKey = val;
        }
        void setExtraDataExpirationFor(std::chrono::milliseconds ms) override {
            UniqueSpinLock lock(*this);
            _data.extraDataExpiration = clock::now() + ms;
        }

        void requestExtraData(DefeatActorType actor, std::function<void()> callback, milliseconds timeoutMs) override;
        void setExtraData(ActorExtraData data) override {
            UniqueSpinLock lock(*this);
            _data.ignoreActorOnHit = data.ignoreActorOnHit;
            _data.sexLabGender = data.sexLabGender;
            _data.sexLabSexuality = data.sexLabSexuality;
            _data.sexLabAllowed = data.sexLabAllowed;
            _data.sexLabRaceKey = data.sexLabRaceKey;
            _data.DFWVulnerability = data.DFWVulnerability;
            _data.extraDataExpiration = clock::now() + 2min;
        }

        bool registerAndCheckHitGuard(DefeatActorType aggressor, RE::FormID source, RE::FormID projectile) override;

        void setStateTransition(bool val) override {
            UniqueSpinLock lock(*this);
            if (val) {
                _data.flags.set(DefeatActorStateFlags::STATE_TRANSITION);
            } else {
                _data.flags.reset(DefeatActorStateFlags::STATE_TRANSITION);
            }
        }

        bool tryExchangeState(DefeatActorStates oldState, DefeatActorStates newState) override {
            UniqueSpinLock lock(*this);
            if (_data.state == oldState) {
                _data.state = newState;
                return true;
            }
            return false;
        }

        bool isSheduledDeplateDynamicDefeat() override { return false; }
        bool sheduleDeplateDynamicDefeat() override { return false; }
        void stopDeplateDynamicDefeat() override {}

        DefeatActorManager* getActorManager() { return _defeatActorManager; }

    protected:
        DefeatActorManager* _defeatActorManager;

        std::unordered_map<HitSpamKey, time_point, ProjectileSpamHash, HitSpamEqual> hitSpamGuard;
    };

    /***************************************************************************************************
     * DefeatPlayerActorImpl
     ****************************************************************************************************/

    class DefeatPlayerActorImpl : public DefeatActorImpl {
    public:
        PapyrusInterface::FloatVarPtr LRGVulnerabilityVar;
        PapyrusInterface::BoolVarPtr IsSurrenderVar;

        DefeatPlayerActorImpl(RE::FormID formID, DefeatActorManager* defeatActorManager);
        bool isPlayer() override { return true; };
        bool isSurrender() override;
        bool isSheduledDeplateDynamicDefeat() override {
            UniqueSpinLock lock(*this);
            return _isSheduledDeplateDynamicDefeat;
        }
        bool sheduleDeplateDynamicDefeat() override {
            UniqueSpinLock lock(*this);
            if (_isSheduledDeplateDynamicDefeat) {
                return false;
            }
            _isSheduledDeplateDynamicDefeat = true;
            return true;
        }
        void stopDeplateDynamicDefeat() override {
            UniqueSpinLock lock(*this);
            _isSheduledDeplateDynamicDefeat = false;
        }
        float getVulnerability() override;

    protected:
        PapyrusInterface::ObjectPtr getDefeatPlayerScript();
        PapyrusInterface::ObjectPtr getLRGDefeatPlayerVulnerabilityScript();
        std::atomic<bool> _isSheduledDeplateDynamicDefeat = false;
    };

    /***************************************************************************************************
     * DefeatActor
     ****************************************************************************************************/
    class DefeatActor : public IDefeatActor {
        friend class DefeatActorManager;
        friend class IDefeatActorManager;
        friend class DefeatActorImpl;

    public:
        DefeatActor(DefeatActorDataType data, RE::Actor* actor, std::shared_ptr<DefeatActorImpl> impl);
        ~DefeatActor() {}

        bool isCreature();
        bool isFollower();
        bool isSatisfied();
        bool isKDImmune();
        bool isKDAllowed();
        bool isTied();
        bool isSexLabAllowed() override;
        bool inSexLabScene();
        bool isDefeated() override;
        bool isDefeatAllowed2PC();
        bool isDefeatAllowed2NvN();
        bool validForAggressorRole();
        bool validForVictrimRole();

        bool isIgnored() override;

        void setHitImmunityFor(std::chrono::milliseconds ms) override { _impl->setHitImmunityFor(ms); };
        void setLastHitAggressor(DefeatActorType lastHitAggressor) override {
            _impl->setLastHitAggressor(lastHitAggressor);
        }
        void clearLastHitAggressors() override { _impl->clearLastHitAggressors(); }
        void setInCombat() override { _impl->setInCombat(); };
        void setNotInCombat() override { _impl->setNotInCombat(); };
        float incrementDynamicDefeat(float val) override {
            return _data.dynamicDefeat = _impl->incrementDynamicDefeat(val);
        }
        float decrementDynamicDefeat(float val) override {
            return _data.dynamicDefeat = _impl->decrementDynamicDefeat(val);
        }
        float resetDynamicDefeat() override { return _data.dynamicDefeat = _impl->resetDynamicDefeat(); }
        void setState(DefeatActorStates state) override { _impl->setState(state); };
        void setVulnerability(float vulnerability) override { _impl->setVulnerability(vulnerability); };
        void setDFWVulnerability(float vulnerability) override { _impl->setDFWVulnerability(vulnerability); };
        void setIgnoreActorOnHit(bool val) override { _impl->setIgnoreActorOnHit(val); };
        void setSexLabGender(int val) override { _impl->setSexLabGender(val); };
        void setSexLabSexuality(int val) override { _impl->setSexLabSexuality(val); };
        void setSexLabAllowed(bool val) override { _impl->setSexLabAllowed(val); }
        void setSexLabRaceKey(std::string val) override { _impl->setSexLabRaceKey(val); }
        void setExtraDataExpirationFor(std::chrono::milliseconds ms) override { _impl->setExtraDataExpirationFor(ms); }
        void requestExtraData(DefeatActorType actor, std::function<void()> callback, milliseconds timeoutMs) override {
            _impl->requestExtraData(actor, callback, timeoutMs);
        }
        void setExtraData(ActorExtraData data) override { _impl->setExtraData(data); }
        bool registerAndCheckHitGuard(DefeatActorType aggressor, RE::FormID source, RE::FormID projectile) override {
            return _impl->registerAndCheckHitGuard(aggressor, source, projectile);
        };
        void setStateTransition(bool val) override { return _impl->setStateTransition(val); }

        bool isSheduledDeplateDynamicDefeat() { return _impl->isSheduledDeplateDynamicDefeat(); }
        bool sheduleDeplateDynamicDefeat() { return _impl->sheduleDeplateDynamicDefeat(); }
        void stopDeplateDynamicDefeat() { _impl->stopDeplateDynamicDefeat(); }

        bool tryExchangeState(DefeatActorStates oldState, DefeatActorStates newState) override {
            if (_impl->tryExchangeState(oldState, newState)) {
                _data.state = newState;
                return true;
            }
            return false;
        };

        bool isEvilFaction();

    protected:
        RE::Actor* _actor;
        std::shared_ptr<DefeatActorImpl> _impl;
        RE::Actor* getTESActor() { return _actor; }

        DefeatActorManager* getActorManager() { return _impl->getActorManager(); };
    };

    /***************************************************************************************************
     * DefeatPlayerActor
     ****************************************************************************************************/
    class DefeatPlayerActor : public DefeatActor {
        friend class DefeatActorManager;
        friend class IDefeatActorManager;

    public:
        DefeatPlayerActor(DefeatActorDataType data, RE::Actor* actor, std::shared_ptr<DefeatActorImpl> impl)
            : DefeatActor(data, actor, impl){};
        bool isPlayer() override { return true; };
        bool isSurrender() override { return _impl->isSurrender(); };
        float getVulnerability() override;
    };
}