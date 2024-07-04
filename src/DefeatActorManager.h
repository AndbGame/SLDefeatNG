#pragma once

#include <Defeat.h>

#include "PapyrusInterface\ActorExtraDataCallQueue.h"
#include "DefeatUtils.h"
#include "DefeatSpinLock.h"

namespace SexLabDefeat {

    class DefeatActorImpl;
    class DefeatPlayerActorImpl;

    class DefeatActorManager : public IDefeatActorManager, public SpinLock {
    public:
        DefeatActorManager(IDefeatManager* defeatManager) : _defeatManager(defeatManager) {};
        ~DefeatActorManager() = default;
        DefeatActorManager(DefeatActorManager const&) = delete;
        void operator=(DefeatActorManager const& x) = delete;

        void reset();

        std::shared_ptr<DefeatPlayerActorImpl> getPlayerImpl() { return _player; }
        DefeatPlayerActorType getPlayer() override;

        std::shared_ptr<DefeatActorImpl> getDefeatActorImpl(RE::Actor* actor);
        std::shared_ptr<DefeatActorImpl> getDefeatActorImpl(RE::FormID formId);
        DefeatActorType getDefeatActor(RE::FormID formID) override;
        DefeatActorType getDefeatActor(RE::Actor* actor) override;
        DefeatActorType getDefeatActor(IDefeatActorType actor) override;
        RE::Actor* getTESActor(DefeatActor* actor) override;

        /* Pre Checks functions */
        bool isIgnored(RE::Actor* actor);
        bool validForAggressorRole(DefeatActor* actor) const;
        bool validForVictrimRole(DefeatActor* actor) const;
        bool validPlayerForVictimRole(RE::Actor* actor);
        /* / Pre Checks functions  */

        bool isDefeatAllowedByAgressor(DefeatActor* target, DefeatActor* aggressor);
        bool IsSexualAssaulAllowedByAggressor(DefeatActor* target, DefeatActor* aggressor);
        bool hasSexInterestByAggressor(DefeatActor* target, DefeatActor* aggressor);
        bool hasSexCombinationWithAggressor(DefeatActor* target, DefeatActor* aggressor);
        bool checkAggressor(DefeatActor* target, DefeatActor* aggressor);

        void playerKnockDownEvent(DefeatActor* target, DefeatActor* aggressor, HitResult event);
        void sexLabSceneInterrupt(DefeatActor* target, DefeatActor* aggressor, bool isHit);
        void sexLabSceneInterrupt(RE::Actor* target, RE::Actor* aggressor, bool isHit);
        void npcKnockDownEvent(DefeatActor* target, DefeatActor* aggressor, HitResult event,
                               bool isBleedout = false, bool isAssault = false);

        float getDistanceBetween(DefeatActor* source, DefeatActor* target);
        float getHeadingAngleBetween(DefeatActor* source, DefeatActor* target);
        float getActorValuePercentage(DefeatActor* source, RE::ActorValue av);
        RE::TESForm* getEquippedHitSourceByFormID(DefeatActor* source, RE::FormID hitSource);
        bool wornHasAnyKeyword(DefeatActor* source, std::list<std::string> kwds);
        bool hasKeywordString(DefeatActor* source, std::string_view kwd);
        bool isInFaction(DefeatActor* actor, RE::TESFaction* faction);
        bool isInFaction(DefeatActor* actor, RE::BGSListForm* faction);
        bool hasCombatTarget(DefeatActor* source, DefeatActor* target);
        bool notInFlyingState(DefeatActor* source);
        bool hasSpell(DefeatActor* source, RE::SpellItem* spell);
        bool hasMagicEffect(DefeatActor* source, RE::EffectSetting* effect);
        bool isInKillMove(DefeatActor* source);
        bool isQuestEnabled(RE::TESQuest* quest);
        bool isInCombat(DefeatActor* source);
        bool IsHostileToActor(DefeatActor* source, DefeatActor* target);
        bool isCommandedActor(DefeatActor* source);
        bool isPlayerTeammate(DefeatActor* source);

        std::list<DefeatActorType> getNearestAggressors(DefeatActor* actor);
        std::list<DefeatActorType> getNearestFollowers(DefeatActor* actor);

        DefeatConfig* getConfig();
        DefeatForms getForms() const;
        SoftDependencyType getSoftDependency();

    protected:
        std::map<RE::FormID, std::shared_ptr<DefeatActorImpl>> _actorMap;
        std::shared_ptr<DefeatPlayerActorImpl> _player;

        std::map<RE::FormID, clock::time_point> _sexLabInterruptExpirations;
        SexLabDefeat::SpinLock _sexLabInterruptExpirationsLock;

        IDefeatManager* _defeatManager;

        void forEachActorsInRange(RE::Actor* target, float a_range, std::function<bool(RE::Actor* a_actor)> a_callback);
        std::list<DefeatActorType> getNearestActorsInRangeByFilter(
            DefeatActor* actor, float a_range, std::function<bool(RE::Actor* aggressor)> a_callback);
    };

    

    struct DefeatRequestCallback : public RE::BSScript::IStackCallbackFunctor {
    public:
        enum ActorType {
            PLAYER,
            FOLLOWER,
            NPC
        };
        DefeatRequestCallback(ActorType actorType) { _actorType = actorType; }
        ~DefeatRequestCallback() = default;

        virtual void operator()([[maybe_unused]] RE::BSScript::Variable a_result) override;

        virtual void SetObject([[maybe_unused]] const RE::BSTSmartPointer<RE::BSScript::Object>& a_object){};

    private:
        ActorType _actorType;
    };

    
    /*
    class OnBSAnimationGraphEventHandler : public RE::BSTEventSink<RE::BSAnimationGraphEvent> {
    public:
        OnBSAnimationGraphEventHandler(IDefeatManager* defeatManager) { _defeatManager = defeatManager; };
        ~OnBSAnimationGraphEventHandler() = default;
        SexLabDefeat::IDefeatManager* _defeatManager;

        virtual RE::BSEventNotifyControl ProcessEvent(const RE::BSAnimationGraphEvent* a_event,
                                                      RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource) {

            if (!a_event || a_event->holder->IsNot(RE::FormType::ActorCharacter)) {
                return RE::BSEventNotifyControl::kContinue;
            }
            auto source = const_cast<RE::Actor*>(a_event->holder->As<RE::Actor>());
            SKSE::log::trace("OnBSAnimationGraphEvent for <{:08X}:{}> event tag: <{}>; payload: <{}>",
                             source->GetFormID(), source->GetName(), a_event->tag.c_str(), a_event->payload.c_str());
            // constexpr std::array events{ "MTState", "IdleStop", "JumpLandEnd" };
            auto defActor = _defeatManager->getActorManager()->getDefeatActor(source);
            if (defActor && defActor->getState() == SexLabDefeat::DefeatActorStates::VICTIM_KNONKDOWN_STATE) {
            
                if (a_event->tag == "MTState1111") {
                    SKSE::log::trace(
                        "OnBSAnimationGraphEvent for <{:08X}:{}> event tag: <{}>; payload: <{}>; MTState; play Bleedout",
                        source->GetFormID(), source->GetName(), a_event->tag.c_str(), a_event->payload.c_str());
                    if (auto process = source->GetActorRuntimeData().currentProcess) {
                        process->PlayIdle(source, _defeatManager->Forms.Idle.BleedoutStart, source);
                    } else {
                        source->NotifyAnimationGraph("BleedoutStart");
                    }
                }
                if (a_event->tag == "BleedoutStop") {
                    SKSE::log::trace(
                        "OnBSAnimationGraphEvent for <{:08X}:{}> event tag: <{}>; payload: <{}>; BleedoutStop; play "
                        "Bleedout",
                        source->GetFormID(), source->GetName(), a_event->tag.c_str(), a_event->payload.c_str());
                    if (auto process = source->GetActorRuntimeData().currentProcess) {
                        process->PlayIdle(source, _defeatManager->Forms.Idle.BleedoutStart, source);
                    } else {
                        source->NotifyAnimationGraph("BleedoutStart");
                    }
                }
            }
            return RE::BSEventNotifyControl::kContinue;
        }
    };
    */
}