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

    DefeatActor::DefeatActor(RE::Actor* actor, DefeatManager* defeatManager) {
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
        delete extraData1;

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

    bool DefeatActor::isSame(RE::Actor* actor) const { return actor->GetFormID() == getActorFormId(); }

    bool DefeatActor::isPlayer() { return false; }

    float DefeatActor::getDistanceTo(DefeatActorType target) {
        auto a1 = getActor()->GetPosition();
        auto a2 = target->getActor()->GetPosition();
        return a1.GetDistance(a2);
    }

    float DefeatActor::getHeadingAngle(DefeatActorType target) {
        auto a1 = getActor()->GetPosition();
        return getActor()->GetHeadingAngle(target->getActor()->GetPosition(), false);
    }

    float DefeatActor::getActorValuePercentage(RE::ActorValue av) {
        auto actor = getActor();
        auto actorAV = actor->AsActorValueOwner();

        auto temporary = actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, av);
        auto total = actorAV->GetPermanentActorValue(av);
        auto current = actorAV->GetActorValue(av);

        return total > 0 ? current / (total + temporary) : 1.0;
    }

    RE::TESForm* DefeatActor::getEquippedSource(HitSource source) {
        auto actor = getActor();
        RE::TESForm* form = nullptr;
        auto equipped_right = actor->GetEquippedObject(false);
        if (equipped_right != nullptr && equipped_right->GetFormID() == source) {
            form = equipped_right;
        } else {
            auto equipped_left = actor->GetEquippedObject(true);
            if (equipped_left != nullptr && equipped_left->GetFormID() == source) {
                form = equipped_left;
            }
        }
        return form;
    }

    bool DefeatActor::wornHasAnyKeyword(std::list<std::string> kwds) {
        auto actor = getActor();

        bool ret = false;

        auto visitor = WornVisitor([&kwds, &ret](RE::InventoryEntryData* a_entry) {
            auto loc_object = a_entry->GetObject();
            RE::TESObjectARMO* loc_armor = nullptr;
            if (loc_object != nullptr && loc_object->IsArmor()) {
                loc_armor = static_cast<RE::TESObjectARMO*>(loc_object);
                if (loc_armor != nullptr) {
                    for (const std::string& kwd : kwds) {
                        SKSE::log::trace("wornHasAnyKeyword - {}:{}", loc_armor->GetFullName(), kwd);
                        if (loc_armor->HasKeywordString(kwd)) {
                            ret = true;
                            return RE::BSContainer::ForEachResult::kStop;
                        }
                    }
                }
            }
            return RE::BSContainer::ForEachResult::kContinue;
        });
        actor->GetInventoryChanges()->VisitWornItems(visitor);
        return ret;
    }

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
        _LRGVulnerabilityVar = PapyrusInterface::FloatVarPtr(new PapyrusInterface::FloatVar(
            [this] { return this->getLRGDefeatPlayerVulnerabilityScript(); }, "Vulnerability_Total"sv,
            PapyrusInterface::ObjectVariableConfig(true, false)));
    }

    float DefetPlayerActor::getVulnerability() {
        auto ret = _LRGVulnerabilityVar->get();
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

    void DefetPlayerActor::requestExtraData(std::function<void()> callback,
                                             DefeatExtraData::RequestedExtraData keys) {
        if (_defeatManager->SoftDependency.LRGPatch &&
            _defeatManager->getConfig()->Config.LRGPatch.DeviousFrameworkON->get() &&
            _defeatManager->getConfig()->Config.LRGPatch.KDWayVulnerabilityUseDFW->get()) {
            keys.insert(DefeatUserExtraData::DataKeyType::DFW_VULNERABILITY);
        }
        DefeatActor::requestExtraData(callback, keys);
    }
}