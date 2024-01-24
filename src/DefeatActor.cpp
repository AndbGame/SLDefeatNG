#include "DefeatActor.h"

namespace SexLabDefeat {

    DefeatActor::DefeatActor(DefeatActorDataType data, RE::Actor* actor, IDefeatActorImplType impl) {
        assert(actor != nullptr);
        _data = data;
        _actor = actor;
        _impl = impl;
    }

    bool DefeatActor::isCreature() { return !_impl->getActorManager()->hasKeywordString(*this, "ActorTypeNPC"); }

    bool DefeatActor::isSatisfied() {
        return _impl->getActorManager()->hasSpell(*this, _impl->getActorManager()->getForms().SatisfiedSPL);
    }

    bool DefeatActor::isKDImmune() {
        return _impl->getActorManager()->hasMagicEffect(
            *this, _impl->getActorManager()->getForms().MiscMagicEffects.ImmunityEFF);
    }

    bool DefeatActor::isKDAllowed() {
        if (_impl->getActorManager()->isInKillMove(*this) || isKDImmune() ||
            _impl->getActorManager()->hasKeywordString(*this, "FavorBrawlEvent")) {
            //        SKSE::log::trace("isKDAllowed - false {} {} {}",
            //                         getActor()->IsInKillMove(), isKDImmune(),
            //                         actor->HasKeywordString("FavorBrawlEvent"));
            return false;
        }
        if (_impl->getActorManager()->isQuestEnabled(
                _impl->getActorManager()->getForms().MiscQuests.DGIntimidateQuest)) {
            SKSE::log::trace("isKDAllowed - false DGIntimidateQuest");
            return false;
        }
        return true;
    }

    bool DefeatActor::isTied() {
        if (_impl->getActorManager()->getSoftDependency().ZaZ) {
            return _impl->getActorManager()->wornHasAnyKeyword(
                *this, std::list<std::string>{"zbfWornWrist", "DefeatWornDevice"});
        }
        return false;
    }

    bool DefeatActor::isSexLabAllowed() {
        if (!isCreature()) {
            return true;
        }
        return IDefeatActor::isSexLabAllowed();
    }

    bool DefeatActor::isDefeatAllowed2PC() {
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

    bool DefeatActor::isDefeatAllowed2NvN() {
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


    float DefeatPlayerActor::getVulnerability() {
        if (!_impl->getActorManager()->getSoftDependency().LRGPatch) {
            return 0;
        }
        float ret = 0;
        if (_impl->getActorManager()->getConfig()->Config.LRGPatch.DeviousFrameworkON->get() &&
            _impl->getActorManager()->getConfig()->Config.LRGPatch.KDWayVulnerabilityUseDFW->get()) {
            ret = _data.DFWVulnerability;

        } else {
            ret = _impl->getVulnerability();
        }
        SKSE::log::trace("DefetPlayerActor::getVulnerability {}", static_cast<int>(ret));
        return ret;
    }

    DefeatPlayerActorImpl::DefeatPlayerActorImpl(RE::FormID formID, IDefeatActorManager* defeatActorManager)
        : DefeatActorImpl(formID, defeatActorManager) {
        if (defeatActorManager->getSoftDependency().LRGPatch) {
            LRGVulnerabilityVar = PapyrusInterface::FloatVarPtr(new PapyrusInterface::FloatVar(
                [this] { return this->getLRGDefeatPlayerVulnerabilityScript(); }, "Vulnerability_Total"sv,
                PapyrusInterface::ObjectVariableConfig(true, false)));
        }
    }

    float DefeatPlayerActorImpl::getVulnerability() {
        if (!getActorManager()->getSoftDependency().LRGPatch) {
            return 0;
        }
        return LRGVulnerabilityVar->get();
    }

    PapyrusInterface::ObjectPtr DefeatPlayerActorImpl::getLRGDefeatPlayerVulnerabilityScript() {
        if (!getActorManager()->getSoftDependency().LRGPatch ||
            getActorManager()->getForms().LRGPatch.DefeatVulnerability == nullptr ||
            getActorManager()->getForms().LRGPatch.DefeatVulnerability->aliases.size() == 0) {
            return nullptr;
        }
        const auto alias1 = getActorManager()->getForms().LRGPatch.DefeatVulnerability->aliases[0];
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