#pragma once

#include <DefeatSpinLock.h>
#include <DefeatUtils.h>


namespace SexLabDefeat_TODO {

    class DefeatActorManager;
    class DefeatActor;
    class DefeatActorImpl;
        
    using DefeatActorType = std::shared_ptr<DefeatActor>;
    using DefeatActorImplType = std::shared_ptr<DefeatActorImpl>;

    using clock = std::chrono::high_resolution_clock;

    class IDefeatActor {
    public:
        struct DataType {
            RE::FormID TESFormId = 0;
            clock::time_point hitImmunityExpiration = SexLabDefeat::emptyTime;
            RE::FormID lastHitAggressor = 0;
        };

        RE::FormID getTESFormId() const { return _data.TESFormId; }

        bool hasHitImmunity() const {
            if (clock::now() < _data.hitImmunityExpiration) {
                return true;
            }
            return false;
        }
        void virtual setHitImmunityFor(std::chrono::milliseconds ms) = 0;

        bool isSame(RE::Actor* actor) const {
            assert(actor != nullptr);
            return actor->GetFormID() == getTESFormId();
        };

        bool isSame(IDefeatActor* actor) const {
            return actor->getTESFormId() == getTESFormId(); 
        };

        bool virtual isPlayer() { return false; };

        float virtual getVulnerability() = 0;
        void virtual setLastHitAggressor(DefeatActorType lastHitAggressor) = 0;

    protected:
        DataType _data;

    };

    class DefeatActor : public IDefeatActor {
        friend class DefeatActorManager;

    public:
        DefeatActor(){};
        ~DefeatActor() {}

        float virtual getVulnerability() { return 0; };

        
        float getDistanceTo(DefeatActorType target) {
            return DefeatActorManager::getDistanceBetween(*this, *target.get());
        }
        float getHeadingAngleWith(DefeatActorType target) {
            return DefeatActorManager::getHeadingAngleBetween(*this, *target.get());
        }
        float getActorValuePercentage(RE::ActorValue av) {
            return DefeatActorManager::getActorValuePercentage(*this, av);
        }
        RE::TESForm* getEquippedHitSourceByFormID(RE::FormID hitSource) {
            return DefeatActorManager::getEquippedHitSourceByFormID(*this, hitSource);
        }
        bool wornHasAnyKeyword(std::list<std::string> kwds) { 
            return DefeatActorManager::wornHasAnyKeyword(*this, kwds);
        }

        void setHitImmunityFor(std::chrono::milliseconds ms) override {
            SexLabDefeat::UniqueSpinLock lock(*_impl.get());
            _impl->setHitImmunityFor(ms);
        };
        void setLastHitAggressor(DefeatActorType lastHitAggressor) {
            SexLabDefeat::UniqueSpinLock lock(*_impl.get());
            _impl->setLastHitAggressor(lastHitAggressor);
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
        struct DataType {};
        bool isPlayer() override { return true; };
        float getVulnerability() override;
    };

//================================================================================================================
//                  IMPLEMENTATION
//================================================================================================================
    class DefeatActorImpl : public IDefeatActor, public SexLabDefeat::SpinLock {
        friend class DefeatActorManager;

    public:
        DefeatActorImpl(RE::FormID formID, DefeatActorManager* defeatActorManager) {
            _data.TESFormId = formID;
            _defeatActorManager = defeatActorManager;
        };
        ~DefeatActorImpl() {}
        DefeatActorImpl(DefeatActorImpl const&) = delete;
        void operator=(DefeatActorImpl const& x) = delete;
                
        void setHitImmunityFor(std::chrono::milliseconds ms) override {
            _data.hitImmunityExpiration = clock::now() + ms;
        };
        void setLastHitAggressor(DefeatActorType lastHitAggressor) {
            _data.lastHitAggressor = lastHitAggressor->getTESFormId();
        }

    protected:
        DefeatActorManager* _defeatActorManager;
    };

    class DefeatPlayerActorImpl : public DefeatActorImpl {
    public:
        bool isPlayer() override { return true; };
    protected:
        SexLabDefeat::PapyrusInterface::ObjectPtr getLRGDefeatPlayerVulnerabilityScript() const {};
        SexLabDefeat::PapyrusInterface::FloatVarPtr _LRGVulnerabilityVar = nullptr;
    };
}