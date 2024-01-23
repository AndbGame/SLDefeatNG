#pragma once

#include <Defeat.h>

#include "DefeatActor.h"
#include "PapyrusInterface\ActorExtraDataCallQueue.h"

namespace SexLabDefeat {

    
    class DefeatActorImpl : public IDefeatActorImpl {
        friend class DefeatActorManager;

    public:

        DefeatActorImpl(RE::FormID formID, IDefeatActorManager* defeatActorManager) {
            _data.TESFormId = formID;
            _defeatActorManager = defeatActorManager;
            extradataQueue = new PapyrusInterface::ActorExtraDataCallQueue(10min, 1min);
        };
        ~DefeatActorImpl() { delete extradataQueue; }
        DefeatActorImpl(DefeatActorImpl const&) = delete;
        void operator=(DefeatActorImpl const& x) = delete;

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
        void responseExtraData(ActorExtraData data) override {
            UniqueSpinLock lock(*this);
            _data.ignoreActorOnHit = data.ignoreActorOnHit;
            _data.sexLabGender = data.sexLabGender;
            _data.sexLabSexuality = data.sexLabSexuality;
            _data.sexLabAllowed = data.sexLabAllowed;
            _data.sexLabRaceKey = data.sexLabRaceKey;
            _data.DFWVulnerability = data.DFWVulnerability;
            _data.extraDataExpiration = clock::now() + 2min;
            extradataQueue->functionResponse(data);
        }
        bool isSheduledDeplateDynamicDefeat() override { return false; }
        bool sheduleDeplateDynamicDefeat() override { return false; }
        void stopDeplateDynamicDefeat() override { }

        IDefeatActorManager* getActorManager() override { return _defeatActorManager; }

    protected:
        IDefeatActorManager* _defeatActorManager;
        PapyrusInterface::ActorExtraDataCallQueue* extradataQueue;
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

    class DefeatActorManager : public IDefeatActorManager {
    public:
        DefeatActorManager(IDefeatManager* defeatManager) : _defeatManager(defeatManager) {};
        ~DefeatActorManager() = default;
        DefeatActorManager(DefeatActorManager const&) = delete;
        void operator=(DefeatActorManager const& x) = delete;

        void reset();

        DefeatPlayerActorImplType getPlayerImpl() override { return _player; }
        DefeatPlayerActorType getPlayer(RE::Actor* actor = nullptr);

        DefeatActorImplType getDefeatActorImpl(RE::Actor* actor) override;
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
        std::map<RE::FormID, DefeatActorImplType> _actorMap;
        DefeatPlayerActorImplType _player;

        IDefeatManager* _defeatManager;
    };
}