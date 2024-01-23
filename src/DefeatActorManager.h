#pragma once

#include <Defeat.h>

#include "DefeatActor.h"
#include "PapyrusInterface\ActorExtraDataCallQueue.h"

namespace SexLabDefeat {

    
    class DefeatActorImpl : public IDefeatActor /*, public SpinLock*/ {
        friend class DefeatActorManager;

    public:
        std::atomic<bool> isSheduledDeplateDynamicDefeat = false;

        DefeatActorImpl(RE::FormID formID, DefeatActorManager* defeatActorManager) {
            _data.TESFormId = formID;
            _defeatActorManager = defeatActorManager;
        };
        ~DefeatActorImpl() {}
        DefeatActorImpl(DefeatActorImpl const&) = delete;
        void operator=(DefeatActorImpl const& x) = delete;

        void setHitImmunityFor(std::chrono::milliseconds ms) override {
            UniqueSpinLock lock(*this);
            _data.hitImmunityExpiration = clock::now() + ms;
        };
        void setLastHitAggressor(DefeatActorType lastHitAggressor) override {
            UniqueSpinLock lock(*this);
            //_data.lastHitAggressor = lastHitAggressor->getTESFormId();
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

        void setState(IDefeatActor::States state) override {
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
        void setExternalPapyrusDataExpirationFor(std::chrono::milliseconds ms) override {
            UniqueSpinLock lock(*this);
            _data.extraDataExpiration = clock::now() + ms;
        }

        DefeatActorManager* getActorManager() { return _defeatActorManager; }

    protected:
        DefeatActorManager* _defeatActorManager;
    };

    class DefeatPlayerActorImpl : public DefeatActorImpl {
    public:
        PapyrusInterface::FloatVarPtr LRGVulnerabilityVar;

        DefeatPlayerActorImpl(RE::FormID formID, DefeatActorManager* defeatActorManager);
        bool isPlayer() override { return true; };
        bool isSheduledDeplateDynamicDefeat() {
            UniqueSpinLock lock(*this);
            return _isSheduledDeplateDynamicDefeat;
        }
        bool sheduleDeplateDynamicDefeat() {
            UniqueSpinLock lock(*this);
            if (_isSheduledDeplateDynamicDefeat) {
                return false;
            }
            _isSheduledDeplateDynamicDefeat = true;
            return true;
        }
        void stopDeplateDynamicDefeat() {
            UniqueSpinLock lock(*this);
            _isSheduledDeplateDynamicDefeat = false;
        }

    protected:
        PapyrusInterface::ObjectPtr getLRGDefeatPlayerVulnerabilityScript();
        std::atomic<bool> _isSheduledDeplateDynamicDefeat = false;
    };

    class DefeatActorManager : public IDefeatActorManager {
    public:
        DefeatActorManager(IDefeatManager* defeatManager) { _defeatManager = defeatManager; };
        ~DefeatActorManager() = default;
        DefeatActorManager(DefeatActorManager const&) = delete;
        void operator=(DefeatActorManager const& x) = delete;

        void reset();

        DefeatActorType getPlayer() override { return _player; }

        DefeatActorType getActor(RE::Actor* actor) override;

        bool validPlayerForVictimRole(RE::Actor* actor) override;
        bool hasSexInterestByAggressor(DefeatActorType target, DefeatActorType aggressor) override;
        bool hasSexCombinationWithAggressor(DefeatActorType target, DefeatActorType aggressor) override;
        bool checkAggressor(DefeatActorType target, DefeatActorType aggressor) override;

    protected:
        std::map<RE::FormID, DefeatActorType> _actorMap;
        DefeatActorType _player;

        IDefeatManager* _defeatManager;
    };
}