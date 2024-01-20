#include "Defeat.h"

namespace SexLabDefeat {

    DefeatActorType DefeatActorManager::getActor(RE::Actor* actor) {
        spinLock();
        auto val = _actorMap.find(actor->GetFormID());
        DefeatActorType defeatActor;
        if (val == _actorMap.end()) {
            defeatActor = std::make_shared<DefeatActor>(actor, _defeatManager);
            _actorMap.emplace(actor->GetFormID(), defeatActor);
        } else {
            defeatActor = val->second;
        }
        spinUnlock();
        defeatActor->setActor(actor);
        return defeatActor;
    }

    void DefeatActorManager::reset() {
        spinLock();
        _actorMap.clear();

        auto actor = RE::PlayerCharacter::GetSingleton();
        DefeatActorType defeatActor = std::make_shared<DefetPlayerActor>(actor, _defeatManager);
        _actorMap.emplace(actor->GetFormID(), defeatActor);
        _player = defeatActor;

        spinUnlock();
    }

    bool DefeatActorManager::validForAggressorRole(RE::Actor* actor) {
        if (actor == nullptr || actor->IsGhost()) {
            return false;
        }
        return true;
    }

    bool DefeatActorManager::validForAggressorRoleOverPlayer(RE::Actor* actor) {
        if (actor == nullptr || actor->IsGhost()) {
            return false;
        }
        return true;
    }

    bool DefeatActorManager::validPlayerForVictimRole(RE::Actor* actor) {
        if (actor == nullptr || actor->IsOnMount() || actor->HasKeywordString("DefeatActive")) {
            return false;
        }
        if (!actor->HasKeywordString("ActorTypeNPC")) {
            return _defeatManager->getConfig()->Config.BeastImmunity->get();
        }
        return true;
    }

    DefeatActor::~DefeatActor() {
        extraData->spinLock();
        delete extraData;

        if (_dynamicDefeatSpinLock != nullptr) {
            delete _dynamicDefeatSpinLock;
        }
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

    bool DefeatActor::notInFlyingState() {
        SKSE::log::trace("notInFlyingState");
        return getActor()->AsActorState()->GetFlyState() == RE::FLY_STATE::kNone;
    }

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
            return wornHasAnyKeyword(std::list<std::string>{"zbfWornWrist", "DefeatWornDevice"});
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

    bool DefeatActor::isDefeatAllowedByAgressor(DefeatActorType aggressor) {
        if (isPlayer()) {
            //SKSE::log::trace("isDefeatAllowedByAgressor {}", aggressor->isDefeatAllowed2PC());
            return aggressor->isDefeatAllowed2PC();
        }
        return aggressor->isDefeatAllowed2NvN();
    };

    bool DefeatActor::IsSexualAssaulterByAggressor(DefeatActorType aggressor) {
        return hasSexInterestByAggressor(aggressor) && hasSexCombinationWithAggressor(aggressor);
    };

    bool DefeatActor::hasSexInterestByAggressor(DefeatActorType aggressor) {
        if (aggressor->isSatisfied()) {
            SKSE::log::trace("CheckAggressor: isSatisfied");
            return false;
        }
        if (isPlayer()) {
            return _defeatManager->randomChanse(_defeatManager->getConfig()->Config.PvicRaped->get());
        }
        if (isFollower()) {
            return _defeatManager->randomChanse(_defeatManager->getConfig()->Config.NVNRapedFollower->get());
        }
        return _defeatManager->randomChanse(_defeatManager->getConfig()->Config.NVNRaped->get());
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

    bool DefeatActor::hasSexCombinationWithAggressor(DefeatActorType aggressor) {
        auto mcmConfig = _defeatManager->getConfig();
        if (isPlayer()) {
            if (!aggressor->isCreature()) {
                bool aggressorFemale = aggressor->isFemale();
                if (!mcmConfig->Config.SexualityPvic->get()) {
                    return (!aggressorFemale && mcmConfig->Config.MaleHunterPvic->get()) ||
                           (aggressorFemale && mcmConfig->Config.FemaleHunterPvic->get());
                } else {
                    bool victimFemale = isFemale();
                    if (!aggressorFemale && victimFemale) {  // Male on Female
                        return mcmConfig->Config.MaleHunterPvic->get() &&
                               (aggressor->IsStraight() || aggressor->IsBisexual());

                    } else if (aggressorFemale && victimFemale) {  // Female on Female
                        return mcmConfig->Config.FemaleHunterPvic->get() &&
                               (aggressor->IsGay() || aggressor->IsBisexual());

                    } else if (aggressorFemale && !victimFemale) {  // Female on Male
                        return mcmConfig->Config.FemaleHunterPvic->get() &&
                               (aggressor->IsStraight() || aggressor->IsBisexual());

                    } else if (!aggressorFemale && !victimFemale) {  // Male on Male
                        return mcmConfig->Config.MaleHunterPvic->get() &&
                               (aggressor->IsGay() || aggressor->IsBisexual());
                    }
                }
            } else {
                if (aggressor->notInFlyingState()) {
                    if (!mcmConfig->Config.SexLab.UseCreatureGender->get()) {
                        return true;
                    }
                    auto gender = aggressor->getSexLabGender();  // 3 - Female creatures, returns 2 male if
                                                                 // SexLabConfig.UseCreatureGender is disabled
                    if (gender == 2 && mcmConfig->Config.HuntCrea->get()) {
                        return true;
                    } else if (gender == 3 && mcmConfig->Config.HuntFCrea->get()) {
                        return true;
                    }
                }
            }
        } else {
            bool victimFemale = isFemale();
            if (!aggressor->isCreature()) {
                bool aggressorFemale = aggressor->isFemale();
                if (!mcmConfig->Config.SexualityNVN->get()) {
                    return (!aggressorFemale && victimFemale && mcmConfig->Config.MaleOnGal->get()) ||
                           (aggressorFemale && victimFemale && mcmConfig->Config.GalOnGal->get()) ||
                           (!aggressorFemale && !victimFemale && mcmConfig->Config.MaleOnMale->get()) ||
                           (aggressorFemale && !victimFemale && mcmConfig->Config.GalOnMale->get());
                } else {
                    if (!aggressorFemale && victimFemale) {  // Male on Female
                        return mcmConfig->Config.MaleOnGal->get() &&
                               (aggressor->IsStraight() || aggressor->IsBisexual());

                    } else if (aggressorFemale && victimFemale) {  // Female on Female
                        return mcmConfig->Config.GalOnGal->get() &&
                               (aggressor->IsGay() || aggressor->IsBisexual());

                    } else if (aggressorFemale && !victimFemale) {  // Female on Male
                        return mcmConfig->Config.GalOnMale->get() &&
                               (aggressor->IsStraight() || aggressor->IsBisexual());

                    } else if (!aggressorFemale && !victimFemale) {  // Male on Male
                        return mcmConfig->Config.MaleOnMale->get() &&
                               (aggressor->IsGay() || aggressor->IsBisexual());
                    }
                }
            } else {
                if (aggressor->notInFlyingState()) {
                    if (!mcmConfig->Config.SexLab.UseCreatureGender->get()) {
                        return true;
                    }
                    auto aggressorFemale = aggressor->getSexLabGender() == 3;
                    if (!aggressorFemale && victimFemale && mcmConfig->Config.CreaOnFemale->get()) {  // Male on Female
                        return true;

                    } else if (aggressorFemale && victimFemale &&
                               mcmConfig->Config.CreaFemaleOnFemale->get()) {  // Female on Female
                        return true;

                    } else if (aggressorFemale && !victimFemale &&
                               mcmConfig->Config.CreaFemaleOnMale->get()) {  // Female on Male
                        return true;

                    } else if (!aggressorFemale && !victimFemale &&
                               mcmConfig->Config.CreaOnMale->get()) {  // Male on Male
                        return true;
                    }
                    // Other genders
                    if ((victimFemale && mcmConfig->Config.CreaOnFemale->get()) ||
                        (!victimFemale && mcmConfig->Config.CreaOnMale->get())) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    bool DefeatActor::CheckAggressor(DefeatActorType aggressor) {
        if (aggressor->isIgnoreActorOnHit()) {
            SKSE::log::trace("CheckAggressor: false - isIgnoreActorOnHit");
            return false;
        }

        if (isSurrender() || _defeatManager->getConfig()->Config.EveryonePvic->get()) {
            return true;
        } else {
            if (!aggressor->isSexLabAllowed() || !isDefeatAllowedByAgressor(aggressor)) {
                return false;
            }
            if (aggressor->isCreature() && !_defeatManager->getConfig()->Config.HuntCrea->get()) {
                return true;
            } else {
                // sexuality
                return IsSexualAssaulterByAggressor(aggressor);
            }
        }
    };

    
    DefetPlayerActor::DefetPlayerActor(RE::Actor* actor, DefeatManager* defeatManager)
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