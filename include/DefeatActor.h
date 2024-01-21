#pragma once

#include <DefeatSpinLock.h>
#include "DefeatPapyrus.h"
#include "DefeatUtils.h"
#include <DefeatActorManager.h>

namespace SexLabDefeat {

    class DefeatActor;
    class DefeatPlayerActor;
    class DefeatActorImpl;
    class DefeatPlayerActorImpl;
        
    using DefeatActorType = std::shared_ptr<DefeatActor>;
    using DefeatActorImplType = std::shared_ptr<DefeatActorImpl>;
    using DefeatPlayerActorType = std::shared_ptr<DefeatPlayerActor>;
    using DefeatPlayerActorImplType = std::shared_ptr<DefeatPlayerActorImpl>;

    using clock = std::chrono::high_resolution_clock;

    class IDefeatActor {
    public:
        enum States { NONE, ACTIVE, DISACTIVE, KNONKOUT, STANDING_STRUGGLE, KNONKDOWN };
        enum class StateFlags : uint8_t { 
            NONE = 0, 
            KNOCK_ALLOWED = 1 << 0, 
            //flag2 = 1 << 1,
            //flag3 = 1 << 2,
            //flag3 = 1 << 3,
            //flag3 = 1 << 4,
            //flag3 = 1 << 5,
            //flag3 = 1 << 6,
            //flag3 = 1 << 7
        };
        struct DataType {
            RE::FormID          TESFormId               = 0;
            clock::time_point   hitImmunityExpiration   = SexLabDefeat::emptyTime;
            RE::FormID          lastHitAggressor        = 0;
            bool                isSurrender             = false;
            States              state                   = States::ACTIVE;
            StateFlags flags = StateFlags::NONE;
            float dynamicDefeat = 0;
            float vulnerability = 0;

            /* External Papyrus Data */
            clock::time_point externalPapyrusDataExpiration = SexLabDefeat::emptyTime;
            float DFWVulnerability = 0;
            bool ignoreActorOnHit = true;
            int sexLabGender = -1;
            int sexLabSexuality = -1;
            bool sexLabAllowed = false;
            std::string sexLabRaceKey = "";
        };

        RE::FormID getTESFormId() const { return _data.TESFormId; }

        bool hasHitImmunity() const { return clock::now() < _data.hitImmunityExpiration; }
        bool isExternalPapyrusDataExpired() const { return clock::now() > _data.externalPapyrusDataExpiration; }
        virtual void setHitImmunityFor(std::chrono::milliseconds ms) = 0;

        bool isSame(RE::Actor* actor) const {
            assert(actor != nullptr);
            return actor->GetFormID() == getTESFormId();
        };

        bool isSame(IDefeatActor* actor) const {
            return actor->getTESFormId() == getTESFormId(); 
        };

        virtual bool isPlayer() { return false; };

        States getState() { return _data.state; };
        bool isSurrender() { return _data.isSurrender; }
        float getDynamicDefeat() { return _data.dynamicDefeat; }
        virtual float getVulnerability() { return _data.vulnerability; }
        float getDFWVulnerability() { return _data.DFWVulnerability; }
        bool isIgnoreActorOnHit() { return _data.ignoreActorOnHit; };

        int getSexLabGender() { return _data.sexLabGender; };
        int getSexLabSexuality() { return _data.sexLabSexuality; };
        virtual bool isSexLabAllowed() {  return _data.sexLabAllowed; }
        std::string getSexLabRaceKey() { return _data.sexLabRaceKey; }

        bool isFemale() { return getSexLabGender() == 1; }
        bool IsStraight() { return getSexLabSexuality() >= 65; }
        bool IsGay() { return getSexLabSexuality() <= 35; }
        bool IsBisexual() {
            auto ratio = getSexLabSexuality();
            return (ratio < 65 && ratio > 35);
        }

        virtual void incrementDynamicDefeat(float val) = 0;
        virtual void decrementDynamicDefeat(float val) = 0;
        virtual void resetDynamicDefeat() = 0;
        virtual void setState(States state) = 0;
        virtual void setVulnerability(float vulnerability) = 0;
        virtual void setDFWVulnerability(float vulnerability) = 0;
        virtual void setLastHitAggressor(DefeatActorType lastHitAggressor) = 0;
        virtual void setIgnoreActorOnHit(bool val) = 0;
        virtual void setSexLabGender(int val) = 0;
        virtual void setSexLabSexuality(int val) = 0;
        virtual void setSexLabAllowed(bool val) = 0;
        virtual void setSexLabRaceKey(std::string val) = 0;
        virtual void setExternalPapyrusDataExpirationFor(std::chrono::milliseconds ms) = 0;

    protected:
        DataType _data;

    };

    class DefeatActor : public IDefeatActor {
        friend class DefeatActorManager;

    public:
        DefeatActor(){};
        ~DefeatActor() {}

        bool isCreature() {
            return !DefeatActorManager::hasKeywordString(*this, "ActorTypeNPC");
        }
        // TODO:
        bool isFollower() { return false; }
        bool notInFlyingState() {
            return !DefeatActorManager::notInFlyingState(*this);
        }
        bool isSatisfied() {
            return DefeatActorManager::hasSpell(*this, _impl->getActorManager()->getForms().SatisfiedSPL);
        }

        bool isKDImmune() {
            return DefeatActorManager::hasMagicEffect(
                *this, _impl->getActorManager()->getForms().MiscMagicEffects.ImmunityEFF);
        }
        bool isKDAllowed() {
            if (DefeatActorManager::isInKillMove(*this) || isKDImmune() ||
                DefeatActorManager::hasKeywordString(*this, "FavorBrawlEvent")) {
                //        SKSE::log::trace("isKDAllowed - false {} {} {}",
                //                         getActor()->IsInKillMove(), isKDImmune(),
                //                         actor->HasKeywordString("FavorBrawlEvent"));
                return false;
            }
            if (DefeatActorManager::isQuestEnabled(_impl->getActorManager()->getForms().MiscQuests.DGIntimidateQuest)) {
                SKSE::log::trace("isKDAllowed - false DGIntimidateQuest");
                return false;
            }
            return true;
        }

        bool isTied() {
            if (_impl->getActorManager()->getSoftDependency().ZaZ) {
                return DefeatActorManager::wornHasAnyKeyword(
                    *this, std::list<std::string>{"zbfWornWrist", "DefeatWornDevice"});
            }
            return false;
        }
        bool isSexLabAllowed() override {
            if (!isCreature()) {
                return true;
            }
            return IDefeatActor::isSexLabAllowed(); 
        }

        bool isDefeatAllowed2PC() {
            bool ret = true;
            if (isCreature()) {
                std::string raceKey = getSexLabRaceKey();
                auto set = _impl->getActorManager()->getConfig()->Config.RaceAllowedPvic->get();
                if (auto search = set.find(raceKey); search == set.end()) {
                    ret = false;
                }
            }
            return ret;
        }

        bool isDefeatAllowed2NvN() {
            bool ret = true;
            if (isCreature()) {
                std::string raceKey = getSexLabRaceKey();
                auto set = _impl->getActorManager()->getConfig()->Config.RaceAllowedNVN->get();
                if (auto search = set.find(raceKey); search == set.end()) {
                    ret = false;
                }
            }
            return ret;
        }

        void setHitImmunityFor(std::chrono::milliseconds ms) override {
            _impl->setHitImmunityFor(ms);
        };

        void setLastHitAggressor(DefeatActorType lastHitAggressor) override {
            _impl->setLastHitAggressor(lastHitAggressor); }

        void incrementDynamicDefeat(float val) override {
            _impl->incrementDynamicDefeat(val);
        }

        void decrementDynamicDefeat(float val) override {
            _impl->decrementDynamicDefeat(val);
        }

        void resetDynamicDefeat() override {
            _impl->resetDynamicDefeat(); 
        }

        void setState(IDefeatActor::States state) override {
            _impl->setState(state);
        };

        void setVulnerability(float vulnerability) override {
            _impl->setVulnerability(vulnerability); 
        };
        void setDFWVulnerability(float vulnerability) override { _impl->setDFWVulnerability(vulnerability); };

        void setIgnoreActorOnHit(bool val) override { _impl->setIgnoreActorOnHit(val); };

        void setSexLabGender(int val) override { _impl->setSexLabGender(val); };

        void setSexLabSexuality(int val) override { _impl->setSexLabSexuality(val); };

        void setSexLabAllowed(bool val) override { _impl->setSexLabAllowed(val); }

        void setSexLabRaceKey(std::string val) override { _impl->setSexLabRaceKey(val); }

        void setExternalPapyrusDataExpirationFor(std::chrono::milliseconds ms) override {
            _impl->setExternalPapyrusDataExpirationFor(ms);
        }

    protected:
        RE::Actor* _actor;
        DefeatActorImplType _impl;

        DefeatActor(DataType data, RE::Actor* actor, DefeatActorImplType impl) {
            assert(actor != nullptr);
            _data = data;
            _actor = actor;
            _impl = impl;
        };

        RE::Actor* getTESActor() { return _actor; }

    };

    class DefeatPlayerActor : public DefeatActor {
        friend class DefeatActorManager;

    public:
        DefeatPlayerActor(DataType data, RE::Actor* actor, DefeatActorImplType impl) : DefeatActor(data, actor, impl) {};
        bool isPlayer() override { return true; };
        float getVulnerability() override;

        bool isSheduledDeplateDynamicDefeat() { return _impl->isSheduledDeplateDynamicDefeat(); }
        bool sheduleDeplateDynamicDefeat() { return _impl->sheduleDeplateDynamicDefeat(); }
        void stopDeplateDynamicDefeat() { _impl->stopDeplateDynamicDefeat(); }

    protected:
        DefeatPlayerActorImplType _impl;
    };

//================================================================================================================
//                  IMPLEMENTATION
//================================================================================================================

    class DefeatActorImpl : public IDefeatActor, public SexLabDefeat::SpinLock {
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
            _data.externalPapyrusDataExpiration = clock::now() + ms;
        }

        DefeatActorManager* getActorManager() {
            return _defeatActorManager;
        }

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
}