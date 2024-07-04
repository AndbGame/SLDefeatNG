#include "DefeatActor.h"

#include "DefeatActorManager.h"

namespace SexLabDefeat {

    DefeatActor::DefeatActor(DefeatActorDataType data, RE::Actor* actor, std::shared_ptr<DefeatActorImpl> impl) {
        assert(actor != nullptr);
        _data = data;
        _actor = actor;
        _impl = impl;
    }

    bool DefeatActor::isCreature() {
        return !getActorManager()->hasKeywordString(this, getActorManager()->getForms().KeywordId.ActorTypeNPC);
    }

    bool DefeatActor::isFollower() {
        return getActorManager()->isPlayerTeammate(this) ||
               getActorManager()->isInFaction(this, getActorManager()->getForms().Faction.CurrentFollowerFaction) ||
               getActorManager()->isInFaction(this, getActorManager()->getForms().Faction.CurrentHireling);
    }

    bool DefeatActor::isSatisfied() {
        return getActorManager()->hasSpell(this, getActorManager()->getForms().SatisfiedSPL);
    }

    bool DefeatActor::isKDImmune() {
        return getActorManager()->hasMagicEffect(this, getActorManager()->getForms().MiscMagicEffects.ImmunityEFF);
    }

    bool DefeatActor::isKDAllowed() {
        if (getActorManager()->isInKillMove(this) || isKDImmune()) {
            //        SKSE::log::trace("isKDAllowed - false {} {} {}",
            //                         getActor()->IsInKillMove(), isKDImmune(),
            //                         actor->HasKeywordString("FavorBrawlEvent"));
            return false;
        }
        if (isPlayer()) {
            if (getActorManager()->hasKeywordString(this, "FavorBrawlEvent")) {
                SKSE::log::trace("isKDAllowed - false FavorBrawlEvent");
                return false;
            }
            if (getActorManager()->isQuestEnabled(getActorManager()->getForms().MiscQuests.DGIntimidateQuest)) {
                SKSE::log::trace("isKDAllowed - false DGIntimidateQuest");
                return false;
            }
        }
        return true;
    }

    bool DefeatActor::isTied() {
        if (getActorManager()->getSoftDependency().ZaZ) {
            return getActorManager()->wornHasAnyKeyword(
                this, std::list<std::string>{"zbfWornWrist", "DefeatWornDevice"});
        }
        return false;
    }

    bool DefeatActor::isSexLabAllowed() {
        if (!isCreature()) {
            return true;
        }
        return IDefeatActor::isSexLabAllowed();
    }

    bool DefeatActor::inSexLabScene() {
        return getActorManager()->hasKeywordString(this, getActorManager()->getForms().KeywordId.SexLabActive) || 
            (getActorManager()->getForms().Faction.SexLabAnimatingFaction &&
                getActorManager()->isInFaction(this, getActorManager()->getForms().Faction.SexLabAnimatingFaction));
    }

    bool DefeatActor::isDefeated() {
        return getActorManager()->isInFaction(this, getActorManager()->getForms().Faction.DefeatFaction);
    }

    bool DefeatActor::isDefeatAllowed2PC() {
        bool ret = true;
        if (isCreature()) {
            std::string raceKey = getSexLabRaceKey();
            auto set = getActorManager()->getConfig()->Config.RaceAllowedPvic->get();
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
            auto set = getActorManager()->getConfig()->Config.RaceAllowedNVN->get();
            if (auto search = set.find(raceKey); search == set.end()) {
                ret = false;
            }
        }
        return ret;
    }

    bool DefeatActor::validForAggressorRole() {
        return getActorManager()->validForAggressorRole(this); 
    }

    bool DefeatActor::validForVictrimRole() {
        return getActorManager()->validForVictrimRole(this);
    }

    bool DefeatActor::isIgnored() { 
        return getActorManager()->isIgnored(_actor); }

    bool DefeatActor::isEvilFaction() {
        return getActorManager()->isInFaction(this, getActorManager()->getForms().Faction.EvilFactionList);
    }


    float DefeatPlayerActor::getVulnerability() {
        if (!getActorManager()->getSoftDependency().LRGPatch) {
            return 0;
        }
        float ret = 0;
        if (getActorManager()->getConfig()->Config.LRGPatch.DeviousFrameworkON->get() &&
            getActorManager()->getConfig()->Config.LRGPatch.KDWayVulnerabilityUseDFW->get()) {
            ret = _data.DFWVulnerability;

        } else {
            ret = _impl->getVulnerability();
        }
        SKSE::log::trace("DefetPlayerActor::getVulnerability {}", static_cast<int>(ret));
        return ret;
    }

    
    void DefeatActorImpl::setLastHitAggressor(DefeatActorType lastHitAggressor) {
        UniqueSpinLock lock(*this);
        _data.lastHitAggressors[lastHitAggressor->getTESFormId()] = clock::now();
    }

    void DefeatActorImpl::requestExtraData(DefeatActorType actor, std::function<void()> callback,
                                           milliseconds timeoutMs) {
        extradataQueue->functionCall(actor->getTESActor(), callback, timeoutMs);
    }

    bool DefeatActorImpl::registerAndCheckHitGuard(DefeatActorType aggressor, RE::FormID source,
                                                   RE::FormID projectile) {
        std::array<HitSpamKey, 2> toCheck{{{0, 0}, {0, 0}}};

        if (source != 0) {
            toCheck[0].actor = aggressor->getTESFormId();
            toCheck[0].source = source;
        }
        if (projectile != 0) {
            toCheck[1].actor = aggressor->getTESFormId();
            toCheck[1].source = projectile;
        }

        auto now = clock::now();

        UniqueSpinLock lock(*this);

        for (const auto& key : toCheck) {
            if (key.source == 0) {
                continue;
            }
            if (auto search = hitSpamGuard.find(key); search != hitSpamGuard.end()) {
                if ((search->second + getActorManager()->getConfig()->HIT_SPAM_GUARD_EXPIRATION_MS) > now) {
                    return true;
                } else {
                    search->second = now;
                    return false;
                }
            }
            hitSpamGuard.insert({key, now});
        }

        return false;
    }

    DefeatPlayerActorImpl::DefeatPlayerActorImpl(RE::FormID formID, DefeatActorManager* defeatActorManager)
        : DefeatActorImpl(formID, defeatActorManager) {
        if (defeatActorManager->getSoftDependency().LRGPatch) {
            LRGVulnerabilityVar = PapyrusInterface::FloatVarPtr(new PapyrusInterface::FloatVar(
                [this] { return this->getLRGDefeatPlayerVulnerabilityScript(); }, "Vulnerability_Total"sv,
                PapyrusInterface::ObjectVariableConfig(true, false)));
        }
        IsSurrenderVar = PapyrusInterface::BoolVarPtr(new PapyrusInterface::BoolVar([this] { return this->getDefeatPlayerScript(); }, "IsSurrender"sv,
            PapyrusInterface::ObjectVariableConfig(false, false)));
    }

    bool DefeatPlayerActorImpl::isSurrender() { return IsSurrenderVar->get(); }

    float DefeatPlayerActorImpl::getVulnerability() {
        if (!getActorManager()->getSoftDependency().LRGPatch) {
            return 0;
        }
        return LRGVulnerabilityVar->get();
    }

    PapyrusInterface::ObjectPtr DefeatPlayerActorImpl::getDefeatPlayerScript() {
        if (getActorManager()->getForms().DefeatPlayerQST == nullptr ||
            getActorManager()->getForms().DefeatPlayerQST->aliases.size() == 0) {
            return nullptr;
        }
        const auto alias0 = getActorManager()->getForms().DefeatPlayerQST->aliases[0];
        if (alias0 == nullptr) {
            return nullptr;
        }
        const auto refAlias = skyrim_cast<RE::BGSRefAlias*>(alias0);
        if (refAlias == nullptr) {
            return nullptr;
        }
        auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
        auto policy = vm->GetObjectHandlePolicy();
        auto handle = policy->GetHandleForObject(refAlias->GetVMTypeID(), refAlias);
        PapyrusInterface::ObjectPtr object = nullptr;
        vm->FindBoundObject(handle, "defeatplayer", object);
        return object;
    }

    PapyrusInterface::ObjectPtr DefeatPlayerActorImpl::getLRGDefeatPlayerVulnerabilityScript() {
        if (!getActorManager()->getSoftDependency().LRGPatch ||
            getActorManager()->getForms().LRGPatch.DefeatVulnerability == nullptr ||
            getActorManager()->getForms().LRGPatch.DefeatVulnerability->aliases.size() == 0) {
            return nullptr;
        }
        const auto alias0 = getActorManager()->getForms().LRGPatch.DefeatVulnerability->aliases[0];
        if (alias0 == nullptr) {
            return nullptr;
        }
        const auto refAlias = skyrim_cast<RE::BGSRefAlias*>(alias0);
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