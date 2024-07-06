#include "DefeatScene.h"

#include "DefeatManager.h"
#include "DefeatActorManager.h"

namespace SexLabDefeat {

    DefeatSceneResult TDefeatScene::makeDefeatSceneResult(DefeatSceneResult::ResultStatus status,
                                                          std::string reason) {
        DefeatSceneResult ret;
        ret.status = status;
        ret.reason = reason;
        return ret;
    }

    DefeatSceneResult DefeatInternalScene::start() {
        auto result = resolve();
        if (result.status != DefeatSceneResult::ResultStatus::SUCCESS) {
            return result;
        }
        return result;
    }

    DefeatSceneResult DefeatInternalScene::resolve() {
        return makeDefeatSceneResult(DefeatSceneResult::ResultStatus::SUCCESS);
    }

    DefeatSceneResult TDefeatSceneWithVictim::resolveVictim() {
        if (!manager->getActorManager()->getDefeatActor(victim)->validForVictrimRole()) {
            auto ret = selectVictim();
            if (ret.status != DefeatSceneResult::ResultStatus::SUCCESS) {
                return ret;
            }
            SKSE::log::trace("DefeatRapeScene - <{:08X}> not validForVictrimRole", victim->getTESFormId());
            return makeDefeatSceneResult(
                DefeatSceneResult::ResultStatus::VICTIM_NOT_SUITABLE,
                fmt::format("Actor <{:08X}> not valid for victrim role", victim->getTESFormId()));
        }
        return makeDefeatSceneResult(DefeatSceneResult::ResultStatus::SUCCESS);
    }

    DefeatSceneResult TDefeatSceneWithVictim::selectVictim() {
        return makeDefeatSceneResult(DefeatSceneResult::ResultStatus::SUCCESS);
    }

    DefeatSceneResult TDefeatSceneWithAggressors::resolveAgressors() {
        auto defeatActorVictim = manager->getActorManager()->getDefeatActor(victim);

        struct aggressorData {
            RE::FormID formID = {};
            float lastHit = {};
            float dist = {};
            DefeatActorType actor = {};
        };
        std::vector<aggressorData> aggressorDataList;
        std::vector<aggressorData> aggressorDataActorList;

        auto LastHitAggressors = defeatActorVictim->getLastHitAggressors();
        for (LastHitAggressorsType::const_iterator it = LastHitAggressors.begin(); it != LastHitAggressors.end();
             ++it) {
            aggressorDataList.push_back({it->first, it->second, 0, nullptr});
        }

        bool hasSkippedActors = false;

        auto nearestAggressors = manager->getActorManager()->getNearestAggressors(defeatActorVictim.get());
        for (DefeatActorType _agg : nearestAggressors) {
            if (!_agg->validForAggressorRole()) {
                SKSE::log::trace("DefeatRapeScene - skipp aggressor: <{:08X}> by reason: validForAggressorRole",
                                 _agg->getTESFormId());
                continue;
            }
            if (_agg->getState() != DefeatActorStates::ACTIVE) {
                SKSE::log::trace("DefeatRapeScene - skipp aggressor: <{:08X}> by reason: DefeatActorState",
                                 _agg->getTESFormId());
                hasSkippedActors = true;
                continue;
            }
            if (_agg->inSexLabScene()) {
                SKSE::log::trace("DefeatRapeScene - skipp aggressor: <{:08X}> by reason: inSexLabScene",
                                 _agg->getTESFormId());
                hasSkippedActors = true;
                continue;
            }

            auto result = resolveAgressor(_agg);
            if (result.status != DefeatSceneResult::ResultStatus::SUCCESS) {
                if (result.status == DefeatSceneResult::ResultStatus::AGGRESSOR_NOT_READY) {
                    hasSkippedActors = true;
                    SKSE::log::trace("DefeatRapeScene - skipp aggressor: <{:08X}> not ready by resolveAgressor",
                                     _agg->getTESFormId());
                } else {
                    SKSE::log::trace("DefeatRapeScene - skipp aggressor: <{:08X}> not allowed by resolveAgressor",
                                     _agg->getTESFormId());
                }
                continue;
            }

            for (aggressorData _aggData : aggressorDataList) {
                if (_agg->getTESFormId() == _aggData.formID) {
                    _aggData.actor = _agg;
                    _aggData.dist = manager->getActorManager()->getDistanceBetween(defeatActorVictim.get(), _agg.get());
                    aggressorDataActorList.push_back(_aggData);
                }
            }
        }

        if (aggressorDataActorList.size() == 0) {
            if (hasSkippedActors) {
                return makeDefeatSceneResult(
                    DefeatSceneResult::ResultStatus::AGGRESSOR_NOT_READY,
                    fmt::format("Actor <{:08X}> not have ready suitable aggressor", defeatActorVictim->getTESFormId()));
            } else {
                return makeDefeatSceneResult(
                    DefeatSceneResult::ResultStatus::AGGRESSOR_NOT_FOUND,
                    fmt::format("Actor <{:08X}> not have any suitable aggressor", defeatActorVictim->getTESFormId()));
            }
        }

        std::vector<aggressorData> aggressorDataActorListByLastHit(aggressorDataActorList.size());

        std::partial_sort_copy(
            aggressorDataActorList.begin(), aggressorDataActorList.end(), aggressorDataActorListByLastHit.begin(),
            aggressorDataActorListByLastHit.end(),
            [](const aggressorData& lVal, const aggressorData& RVal) { return lVal.lastHit > RVal.lastHit; });

        if (manager->hasTraceLog()) {
            std::string aggressors_string = "";
            for (aggressorData agg : aggressorDataActorListByLastHit) {
                aggressors_string += fmt::format("<{:08X}> ", agg.formID);
            }
            SKSE::log::trace("DefeatRapeScene - list of aggressors for <{:08X}> by Hit: {}",
                             defeatActorVictim->getTESFormId(),
                             aggressors_string);
        }

        DefeatActorType sceneAggressor = nullptr;
        std::string reason = "";

        while (true) {
            if (aggressorDataActorListByLastHit.size() > 0) {
                aggressorData agg = aggressorDataActorListByLastHit.front();
                reason = "LastHit";

                if (aggressorDataActorListByLastHit.size() > 1) {
                    std::vector<aggressorData> aggressorDataActorListByDist(aggressorDataActorListByLastHit.size() - 1);
                    std::partial_sort_copy(
                        aggressorDataActorListByLastHit.begin() + 1, aggressorDataActorListByLastHit.end(),
                        aggressorDataActorListByDist.begin(), aggressorDataActorListByDist.end(),
                        [](const aggressorData& lVal, const aggressorData& RVal) { return lVal.dist < RVal.dist; });

                    aggressorData aggByDist = aggressorDataActorListByDist.front();

                    if (aggByDist.formID != agg.formID && (aggByDist.dist < std::lerp(0.0, agg.dist, 0.4)) &&
                        randomChanse(50.0)) {
                        reason = "Distance";
                        agg = aggByDist;
                    }
                }

                auto ret = selectAgressor(agg.actor);
                if (ret.status != DefeatSceneResult::ResultStatus::SUCCESS) {
                    for (std::vector<aggressorData>::iterator it = aggressorDataActorListByLastHit.begin();
                         it != aggressorDataActorListByLastHit.end();) {
                        if ((*it).formID == agg.formID) {
                            aggressorDataActorListByLastHit.erase(it);
                            break;
                        }
                    }
                    continue;
                } else {
                    sceneAggressor = agg.actor;
                }
            }
            break;
        }
        if (sceneAggressor == nullptr) {
            return makeDefeatSceneResult(
                DefeatSceneResult::ResultStatus::AGGRESSOR_NOT_READY,
                fmt::format("Actor <{:08X}> not have ready suitable aggressor", defeatActorVictim->getTESFormId()));
        }
        SKSE::log::trace("DefeatRapeScene - <{:08X}> selected aggressor: <{:08X}> by {}",
                         defeatActorVictim->getTESFormId(), sceneAggressor->getTESFormId(), reason);
        aggressors.push_back(sceneAggressor);
        return makeDefeatSceneResult(DefeatSceneResult::ResultStatus::SUCCESS);
    }

    DefeatSceneResult TDefeatSceneWithAggressors::resolveAgressor(DefeatActorType agg) {
        return makeDefeatSceneResult(DefeatSceneResult::ResultStatus::SUCCESS);
    }

    DefeatSceneResult TDefeatSceneWithAggressors::selectAgressor(DefeatActorType agg) {
        return makeDefeatSceneResult(DefeatSceneResult::ResultStatus::SUCCESS);
    }

    DefeatSceneResult DefeatRapeScene::resolve() {
        auto result = resolveVictim();
        if (result.status != DefeatSceneResult::ResultStatus::SUCCESS) {
            return result;
        }
        result = resolveAgressors();
        if (result.status != DefeatSceneResult::ResultStatus::SUCCESS) {
            return result;
        }
        return result;
    }
    DefeatSceneResult DefeatRapeScene::resolveAgressor(DefeatActorType agg) {
        if (agg->isKDImmune()) {
            return makeDefeatSceneResult(DefeatSceneResult::ResultStatus::AGGRESSOR_NOT_READY);
        }

        auto defeatActorVictim = manager->getActorManager()->getDefeatActor(victim);

        auto configNPCLastEnemy = manager->getConfig()->Config.NPCLastEnemy->get();
        if (!defeatActorVictim->isPlayer()) {
            if (agg->inCombat() && configNPCLastEnemy) {
                SKSE::log::trace("DefeatRapeScene - skipp aggressor: <{:08X}> by reason: inCombat",
                                 agg->getTESFormId());
                return makeDefeatSceneResult(DefeatSceneResult::ResultStatus::AGGRESSOR_NOT_READY);
            }
        } else {
            // TODO: ???
        }
        if (!manager->getActorManager()->IsSexualAssaulAllowedByAggressor(defeatActorVictim.get(), agg.get())) {
            return makeDefeatSceneResult(DefeatSceneResult::ResultStatus::AGGRESSOR_NOT_FOUND);
        }

        return makeDefeatSceneResult(DefeatSceneResult::ResultStatus::SUCCESS);
    }

    DefeatSceneResult DefeatRapeScene::selectAgressor(DefeatActorType agg) {
        if (agg->tryExchangeState(DefeatActorStates::ACTIVE, DefeatActorStates::ASSAULT_STATE)) {
            return makeDefeatSceneResult(DefeatSceneResult::ResultStatus::SUCCESS);
        }
        return makeDefeatSceneResult(DefeatSceneResult::ResultStatus::AGGRESSOR_NOT_READY);
    }

    DefeatSceneResult DefeatKnockdownScene::resolve() {
        auto result = resolveVictim();
        if (result.status != DefeatSceneResult::ResultStatus::SUCCESS) {
            return result;
        }
        result = resolveAgressors();
        if (result.status != DefeatSceneResult::ResultStatus::SUCCESS) {
            return result;
        }
        return result;
    }
    DefeatSceneResult DefeatKnockdownScene::resolveAgressor(DefeatActorType agg) {
        if (!manager->getActorManager()->isInCombat(agg.get())) {
            return makeDefeatSceneResult(DefeatSceneResult::ResultStatus::AGGRESSOR_NOT_FOUND);
        }

        auto defeatActorVictim = manager->getActorManager()->getDefeatActor(victim);
        if (!manager->getActorManager()->IsHostileToActor(agg.get(), defeatActorVictim.get())) {
            return makeDefeatSceneResult(DefeatSceneResult::ResultStatus::AGGRESSOR_NOT_FOUND);
        }

        auto EveryoneNVN = manager->getConfig()->Config.EveryoneNVN->get();
        if (EveryoneNVN ||
            manager->getActorManager()->hasSexCombinationWithAggressor(defeatActorVictim.get(), agg.get())) {
            return makeDefeatSceneResult(DefeatSceneResult::ResultStatus::SUCCESS);
        }
        return makeDefeatSceneResult(DefeatSceneResult::ResultStatus::AGGRESSOR_NOT_FOUND);
    }

    std::shared_ptr<DefeatRapeScene> DefeatSceneManager::querySceneForVictimRape(DefeatActorType actor) {
        auto scene = std::make_shared<DefeatRapeScene>(actor, manager);
        auto ret = scene->start();
        if (ret.status != DefeatSceneResult::ResultStatus::SUCCESS) {
            return nullptr;
        }
        return scene;
    }

    std::shared_ptr<DefeatKnockdownScene> DefeatSceneManager::querySceneForVictimKnockdown(DefeatActorType actor) {
        auto scene = std::make_shared<DefeatKnockdownScene>(actor, manager);
        auto ret = scene->start();
        if (ret.status != DefeatSceneResult::ResultStatus::SUCCESS) {
            return nullptr;
        }
        return scene;
    }
}
