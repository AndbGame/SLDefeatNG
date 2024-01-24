#pragma once

#include <Defeat.h>

#include "PapyrusInterface\ActorExtraDataCallQueue.h"
#include "DefeatSpinLock.h"
#include "PapyrusInterface\ObjectVariable.h"

namespace SexLabDefeat {

    class DefeatActorImpl : public IDefeatActorImpl, public SpinLock {
        friend class DefeatActorManager;

    public:
        DefeatActorImpl(RE::FormID formID, IDefeatActorManager* defeatActorManager) {
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
        void setLastHitAggressor(DefeatActorType lastHitAggressor) override {
            UniqueSpinLock lock(*this);
            _data.lastHitAggressor = lastHitAggressor->getTESFormId();
        }

        void incrementDynamicDefeat(float val) override {
            UniqueSpinLock lock(*this);
            _data.dynamicDefeat += val;
            if (_data.dynamicDefeat > 1) {
                _data.dynamicDefeat = 1;
            }
        }

        void decrementDynamicDefeat(float val) override {
            UniqueSpinLock lock(*this);
            _data.dynamicDefeat -= val;
            if (_data.dynamicDefeat < 0) {
                _data.dynamicDefeat = 0;
            }
        }

        void resetDynamicDefeat() override {
            UniqueSpinLock lock(*this);
            _data.dynamicDefeat = 0;
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

        void requestExtraData(RE::Actor* TesActor, std::function<void()> callback, milliseconds timeoutMs) override {
            extradataQueue->functionCall(TesActor, callback, timeoutMs);
        }
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
        bool isSheduledDeplateDynamicDefeat() override { return false; }
        bool sheduleDeplateDynamicDefeat() override { return false; }
        void stopDeplateDynamicDefeat() override {}

        IDefeatActorManager* getActorManager() override { return _defeatActorManager; }

    protected:
        IDefeatActorManager* _defeatActorManager;
    };

    class DefeatPlayerActorImpl : public DefeatActorImpl {
    public:
        PapyrusInterface::FloatVarPtr LRGVulnerabilityVar;

        DefeatPlayerActorImpl(RE::FormID formID, IDefeatActorManager* defeatActorManager);
        bool isPlayer() override { return true; };
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
        PapyrusInterface::ObjectPtr getLRGDefeatPlayerVulnerabilityScript();
        std::atomic<bool> _isSheduledDeplateDynamicDefeat = false;
    };
}