#include "DefeatActor.h"

namespace SexLabDefeat {

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