#pragma once

#include <Defeat.h>

#include "DefeatActor.h"
#include "PapyrusInterface\ActorExtraDataCallQueue.h"
#include "DefeatUtils.h"
#include "DefeatSpinLock.h"

namespace SexLabDefeat {

    class DefeatActorManager : public IDefeatActorManager, public SpinLock {
    public:
        DefeatActorManager(IDefeatManager* defeatManager) : _defeatManager(defeatManager) {};
        ~DefeatActorManager() = default;
        DefeatActorManager(DefeatActorManager const&) = delete;
        void operator=(DefeatActorManager const& x) = delete;

        void reset();

        std::shared_ptr<DefeatPlayerActorImpl> getPlayerImpl() { return _player; }
        DefeatPlayerActorType getPlayer(RE::Actor* actor = nullptr) override;

        std::shared_ptr<DefeatActorImpl> getDefeatActorImpl(RE::Actor* actor);
        std::shared_ptr<DefeatActorImpl> getDefeatActorImpl(RE::FormID formId);
        DefeatActorType getDefeatActor(RE::FormID formID) override;
        DefeatActorType getDefeatActor(RE::Actor* actor) override;
        DefeatActorType getSuitableAggressor(DefeatActorType actor) override;
        std::list<DefeatActorType> getSuitableFollowers(DefeatActorType actor) override;
        std::list<DefeatActorType> getSuitableAggressors(DefeatActorType actor) override;

        bool isIgnored(RE::Actor* actor) override;
        bool validPlayerForVictimRole(RE::Actor* actor) override;
        bool hasSexInterestByAggressor(DefeatActorType target, DefeatActorType aggressor) override;
        bool hasSexCombinationWithAggressor(DefeatActorType target, DefeatActorType aggressor) override;
        bool checkAggressor(DefeatActorType target, DefeatActorType aggressor) override;

        void playerKnockDownEvent(DefeatActorType target, DefeatActorType aggressor, HitResult event) override;
        void sexLabSceneInterrupt(DefeatActorType target, DefeatActorType aggressor, bool isHit) override;
        void sexLabSceneInterrupt(RE::Actor* target, RE::Actor* aggressor, bool isHit) override;
        void npcKnockDownEvent(DefeatActorType target, DefeatActorType aggressor, HitResult event,
                               bool isBleedout = false, bool isAssault = false) override;

        DefeatConfig* getConfig() override;
        DefeatForms getForms() override;
        SoftDependencyType getSoftDependency() override;

    protected:
        std::map<RE::FormID, std::shared_ptr<DefeatActorImpl>> _actorMap;
        std::shared_ptr<DefeatPlayerActorImpl> _player;

        std::map<RE::FormID, clock::time_point> _sexLabInterruptExpirations;
        SexLabDefeat::SpinLock _sexLabInterruptExpirationsLock;

        IDefeatManager* _defeatManager;
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

    
    
    class OnBSAnimationGraphEventHandler : public RE::BSTEventSink<RE::BSAnimationGraphEvent> {
    public:
        OnBSAnimationGraphEventHandler(){};
        ~OnBSAnimationGraphEventHandler() = default;

        virtual RE::BSEventNotifyControl ProcessEvent(const RE::BSAnimationGraphEvent* a_event,
                                                      RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource) {
            if (!a_event || a_event->holder->IsNot(RE::FormType::ActorCharacter))
                return RE::BSEventNotifyControl::kContinue;

            auto source = const_cast<RE::Actor*>(a_event->holder->As<RE::Actor>());

            SKSE::log::trace("OnBSAnimationGraphEventHandler for <{:08X}:{}> from <{}>", source->GetFormID(),
                             source->GetName(), a_event->tag);

            // constexpr std::array events{ "MTState", "IdleStop", "JumpLandEnd" };
            if (a_event->tag == "MTState") {
                if (auto process = source->GetActorRuntimeData().currentProcess) {
                    //process->PlayIdle(source, GameForms::BleedoutStart, source);
                } else {
                    //source->NotifyAnimationGraph("BleedoutStart");
                }
            }
            return RE::BSEventNotifyControl::kContinue;
        }
    };
}