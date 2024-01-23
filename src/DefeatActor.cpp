#include "DefeatActor.h"

namespace SexLabDefeat {

    DefeatActor::DefeatActor(RE::Actor* actor, IDefeatManager* defeatManager) {
        _defeatManager = defeatManager;
        _actorFormId = actor->GetFormID();
        _actor = actor;
        _minTime = std::chrono::high_resolution_clock::time_point::min();
        hitImmunityExpiration = _minTime;
        extraData = new DeferredExpiringValue<ActorExtraData>(
            std::make_unique<PapyrusInterface::DeferredActorExtraDataInitializer>(actor), 5 * 60 * 1000, 60 * 1000);
    }

    DefeatActor::~DefeatActor() {
        extraData->spinLock();
        delete extraData;

        if (_dynamicDefeatSpinLock != nullptr) {
            delete _dynamicDefeatSpinLock;
        }
    }

    RE::Actor* DefeatActor::getActor() {
        spinLock();
        if (_actor == nullptr) {
            SKSE::log::critical("getActor() - actor in nullptr, try fetch");
            auto actor = RE::TESForm::LookupByID<RE::Actor>(getActorFormId());
            _actor = actor;
        }
        spinUnlock();
        return _actor;
    }

    void DefeatActor::setActor(RE::Actor* actor) { _actor = actor; }

    IDefeatActorManager* DefeatActor::getActorManager() { return _defeatManager->getActorManager(); }

    bool DefeatActor::isSame(RE::Actor* actor) const { return actor->GetFormID() == getActorFormId(); }

    bool DefeatActor::hasHitImmunity() {
        spinLock();
        if (hitImmunityExpiration != _minTime) {
            if (std::chrono::high_resolution_clock::now() < hitImmunityExpiration) {
                spinUnlock();
                return true;
            } else {
                hitImmunityExpiration = _minTime;
            }
        }
        spinUnlock();
        return false;
    }

    void DefeatActor::addHitImmunity(int ms) {
        spinLock();
        hitImmunityExpiration = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(ms);
        spinUnlock();
    }

    void DefeatActor::setLastHitAggressor(DefeatActorType lastHitAggressor) {
        spinLock();
        _lastHitAggressor = lastHitAggressor;
        spinUnlock();
    }

    bool DefeatActor::isSurrender() {
        bool ret;
        spinLock();
        ret = _isSurrender;
        spinUnlock();
        return ret;
    }

    bool DefeatActor::isCreature() {
        auto actor = getActor();
        if (actor != nullptr && actor->HasKeywordString("ActorTypeNPC")) {
            return false;
        }
        return true;
    }

    // TODO:
    bool DefeatActor::isFollower() { return false; }


    bool DefeatActor::isSatisfied() {
        if (_defeatManager->Forms.SatisfiedSPL == nullptr) {
            return false;
        }
        return getActor()->HasSpell(_defeatManager->Forms.SatisfiedSPL);
    }

    bool DefeatActor::isKDImmune() {
        if (_defeatManager->Forms.MiscMagicEffects.ImmunityEFF == nullptr) {
            return false;
        }
        return getActor()->GetMagicTarget()->HasMagicEffect(_defeatManager->Forms.MiscMagicEffects.ImmunityEFF);
    }

    bool DefeatActor::isKDAllowed() {
        auto actor = getActor();
        if (getActor()->IsInKillMove() || isKDImmune() || actor->HasKeywordString("FavorBrawlEvent")) {
            //        SKSE::log::trace("isKDAllowed - false {} {} {}",
            //                         getActor()->IsInKillMove(), isKDImmune(),
            //                         actor->HasKeywordString("FavorBrawlEvent"));
            return false;
        }
        if (_defeatManager->Forms.MiscQuests.DGIntimidateQuest != nullptr &&
            _defeatManager->Forms.MiscQuests.DGIntimidateQuest->IsEnabled()) {
            SKSE::log::trace("isKDAllowed - false DGIntimidateQuest");
            return false;
        }
        return true;
    }

    bool DefeatActor::isTied() {
        if (_defeatManager->SoftDependency.ZaZ) {
            return getActorManager()->wornHasAnyKeyword(*this, std::list<std::string>{"zbfWornWrist", "DefeatWornDevice"});
        }
        return false;
    }

    void DefeatActor::setVulnerability(float vulnerability) {
        spinLock();
        if (vulnerability > 100) vulnerability = 100;
        _vulnerability = vulnerability;
        spinUnlock();
    }

    float DefeatActor::getVulnerability() {
        float ret;
        spinLock();
        ret = _vulnerability;
        spinUnlock();
        return ret;
    }

    bool DefeatActor::isIgnoreActorOnHit() {
        bool ret;
        extraData->spinLock();
        ret = extraData->getValue().ignoreActorOnHit;
        extraData->spinUnlock();
        return ret;
    };

    int DefeatActor::getSexLabGender() {
        int ret;
        extraData->spinLock();
        ret = extraData->getValue().sexLabGender;
        extraData->spinUnlock();
        return ret;
    };

    int DefeatActor::getSexLabSexuality() {
        int ret;
        extraData->spinLock();
        ret = extraData->getValue().sexLabSexuality;
        extraData->spinUnlock();
        return ret;
    };

    bool DefeatActor::isSexLabAllowed() {
        if (!isCreature()) {
            return true;
        }
        bool ret;
        extraData->spinLock();
        ret = extraData->getValue().sexLabAllowed;
        extraData->spinUnlock();
        return ret;
    }

    bool DefeatActor::isDefeatAllowed2PC() {
        bool ret = true;
        if (isCreature()) {
            extraData->spinLock();
            std::string raceKey = extraData->getValue().sexLabRaceKey;
            extraData->spinUnlock();
            auto set = _defeatManager->getConfig()->Config.RaceAllowedPvic->get();
            if (auto search = set.find(raceKey); search == set.end()) {
                ret = false;
            }
        }
        return ret;
    }

    bool DefeatActor::isDefeatAllowed2NvN() {
        bool ret = true;
        if (isCreature()) {
            extraData->spinLock();
            std::string raceKey = extraData->getValue().sexLabRaceKey;
            extraData->spinUnlock();
            auto set = _defeatManager->getConfig()->Config.RaceAllowedNVN->get();
            if (auto search = set.find(raceKey); search == set.end()) {
                ret = false;
            }
        }
        return ret;
    }

    std::string DefeatActor::getSexLabRaceKey() {
        std::string ret;
        extraData->spinLock();
        ret = extraData->getValue().sexLabRaceKey;
        extraData->spinUnlock();
        return ret;
    }

    float DefeatActor::getDynamicDefeat() {
        spinLock();
        auto ret = _dynamicDefeat;
        spinUnlock();
        return ret;
    }

    SpinLock* DefeatActor::getDynamicDefeatSpinLock() {
        if (_dynamicDefeatSpinLock == nullptr) {
            _dynamicDefeatSpinLock = new SpinLock();
        }
        return _dynamicDefeatSpinLock;
    }

    void DefeatActor::incrementDynamicDefeat(float val) {
        spinLock();
        _dynamicDefeat += val;
        if (_dynamicDefeat > 1) {
            _dynamicDefeat = 1;
        }
        spinUnlock();
    }

    void DefeatActor::decrementDynamicDefeat(float val) {
        spinLock();
        _dynamicDefeat -= val;
        if (_dynamicDefeat < 0) {
            _dynamicDefeat = 0;
        }
        spinUnlock();
    }

    void DefeatActor::resetDynamicDefeat() {
        spinLock();
        _dynamicDefeat = 0;
        spinUnlock();
    }

    bool DefeatActor::isFemale() { return getSexLabGender() == 1; }

    bool DefeatActor::IsStraight() { return getSexLabSexuality() >= 65; }

    bool DefeatActor::IsGay() { return getSexLabSexuality() <= 35; }

    bool DefeatActor::IsBisexual() {
        auto ratio = getSexLabSexuality();
        return (ratio < 65 && ratio > 35);
    }

    void DefeatActor::setState(DefeatActor::States state) {
        spinLock();
        _state = state;
        spinUnlock();
    };
    DefeatActor::States DefeatActor::getState() {
        DefeatActor::States ret;
        spinLock();
        ret = _state;
        spinUnlock();
        return ret;
    };

    DefetPlayerActor::DefetPlayerActor(RE::Actor* actor, IDefeatManager* defeatManager)
        : DefeatActor(actor, defeatManager) {
        if (_defeatManager->SoftDependency.LRGPatch) {
            _LRGVulnerabilityVar = PapyrusInterface::FloatVarPtr(new PapyrusInterface::FloatVar(
                [this] { return this->getLRGDefeatPlayerVulnerabilityScript(); }, "Vulnerability_Total"sv,
                PapyrusInterface::ObjectVariableConfig(true, false)));
        }
    }

    float DefetPlayerActor::getVulnerability() {
        if (!_defeatManager->SoftDependency.LRGPatch) {
            return 0;
        }
        float ret = 0;
        if (_defeatManager->getConfig()->Config.LRGPatch.DeviousFrameworkON->get() &&
            _defeatManager->getConfig()->Config.LRGPatch.KDWayVulnerabilityUseDFW->get()) {
            extraData->spinLock();
            ret = extraData->getValue().DFWVulnerability;
            extraData->spinUnlock();

        } else {
            ret = _LRGVulnerabilityVar->get();
        }
        SKSE::log::trace("DefetPlayerActor::getVulnerability {}", static_cast<int>(ret));
        return ret;
    }

    PapyrusInterface::ObjectPtr SexLabDefeat::DefetPlayerActor::getLRGDefeatPlayerVulnerabilityScript() const {
        if (!_defeatManager->SoftDependency.LRGPatch || _defeatManager->Forms.LRGPatch.DefeatVulnerability == nullptr ||
            _defeatManager->Forms.LRGPatch.DefeatVulnerability->aliases.size() == 0) {
            return nullptr;
        }
        const auto alias1 = _defeatManager->Forms.LRGPatch.DefeatVulnerability->aliases[0];
        if (alias1 == nullptr) {
            return nullptr;
        }
        const auto refAlias = skyrim_cast<RE::BGSRefAlias*>(alias1);
        if (refAlias == nullptr) {
            return nullptr;
        }
        auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
        auto policy = vm->GetObjectHandlePolicy();
        auto handle = policy->GetHandleForObject(refAlias->GetVMTypeID(), refAlias);
        PapyrusInterface::ObjectPtr object = nullptr;
        vm->FindBoundObject(handle, "DefeatPlayer_Vulnerability", object);
        return object;
    }
}