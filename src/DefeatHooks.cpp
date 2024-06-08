#include "DefeatHooks.h"

namespace EventSync {

    class OnTESCombatEventHandler : public RE::BSTEventSink<RE::TESCombatEvent> {
    public:
        OnTESCombatEventHandler(SexLabDefeat::DefeatManager* defeatManager) { _defeatManager = defeatManager; };
        ~OnTESCombatEventHandler() = default;
        SexLabDefeat::DefeatManager* _defeatManager;

        virtual RE::BSEventNotifyControl ProcessEvent(const RE::TESCombatEvent* a_event,
            RE::BSTEventSource<RE::TESCombatEvent>* a_eventSource) {
            if (a_event->actor != nullptr) {
                auto actor = a_event->actor->As<RE::Actor>();
                if (actor != nullptr) {
                    if (a_event->newState.get() == RE::ACTOR_COMBAT_STATE::kCombat) {
                        RE::Actor* target_actor = nullptr;
                        if (a_event->targetActor != nullptr) {
                            target_actor = a_event->targetActor->As<RE::Actor>();
                        }
                        if (target_actor) {
                            SKSE::log::info("OnTESCombatEventHandler <{:08X}:{}> in combat state with <{:08X}:{}>",
                                            actor->GetFormID(), actor->GetName(), target_actor->GetFormID(),
                                            target_actor->GetName());
                        } else {
                            SKSE::log::info("OnTESCombatEventHandler <{:08X}:{}> in combat state", actor->GetFormID(),
                                            actor->GetName());
                        }
                        _defeatManager->getCombatManager()->onActorEnteredToCombatState(actor, target_actor);
                    }
                    if (a_event->newState.get() == RE::ACTOR_COMBAT_STATE::kNone) {
                        SKSE::log::info("OnTESCombatEventHandler <{:08X}:{}> in none state", actor->GetFormID(),
                                        actor->GetName());
                        _defeatManager->getCombatManager()->onActorEnteredToNonCombatState(actor);
                    }
                }
            }
            return RE::BSEventNotifyControl::kContinue;
        }
    };

    class OnBSAnimationGraphEvent : public RE::BSTEventSink<RE::BSAnimationGraphEvent> {
    public:
        OnBSAnimationGraphEvent(SexLabDefeat::DefeatManager* defeatManager) { _defeatManager = defeatManager; };
        ~OnBSAnimationGraphEvent() = default;
        SexLabDefeat::DefeatManager* _defeatManager;

        virtual RE::BSEventNotifyControl ProcessEvent(const RE::BSAnimationGraphEvent* a_event,
                                                      RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource) {

            if (!a_event || a_event->holder->IsNot(RE::FormType::ActorCharacter)) {
                return RE::BSEventNotifyControl::kContinue;
            }

            auto source = const_cast<RE::Actor*>(a_event->holder->As<RE::Actor>());
            // constexpr std::array events{ "MTState", "IdleStop", "JumpLandEnd" };
            auto defActor = _defeatManager->getActorManager()->getDefeatActor(source);
            if (defActor && defActor->getState() == SexLabDefeat::DefeatActorStates::KNONKDOWN_STATE &&
                a_event->tag == "MTState") {
                SKSE::log::trace("OnBSAnimationGraphEvent for <{:08X}:{}> event tag: <{}>; payload: <{}>; play Bleedout", source->GetFormID(),
                                 source->GetName(), a_event->tag, a_event->payload);
                if (auto process = source->GetActorRuntimeData().currentProcess) {
                    process->PlayIdle(source, _defeatManager->Forms.Idle.BleedoutStart, source);
                } else {
                    source->NotifyAnimationGraph("BleedoutStart");
                }
            }
            return RE::BSEventNotifyControl::kContinue;
        }
    };

    class OnTESHitEventHandler : public RE::BSTEventSink<RE::TESHitEvent> {
    public:
        OnTESHitEventHandler(SexLabDefeat::DefeatManager* defeatManager) { _defeatManager = defeatManager; };
        ~OnTESHitEventHandler() = default;
        SexLabDefeat::DefeatManager* _defeatManager;

        virtual RE::BSEventNotifyControl ProcessEvent(const RE::TESHitEvent* a_event,
                                                      RE::BSTEventSource<RE::TESHitEvent>* a_eventSource) {
            if (a_event->cause == nullptr) {
                return RE::BSEventNotifyControl::kContinue;
            }

            SexLabDefeat::RawHitEvent evt;
            evt.target = a_event->target.get();
            evt.aggressor = (a_event->cause != nullptr) ? a_event->cause.get() : nullptr;
            evt.source = a_event->source;
            evt.projectile = a_event->projectile;
            evt.isPowerAttack = a_event->flags.any(RE::TESHitEvent::Flag::kPowerAttack);
            evt.isSneakAttack = a_event->flags.any(RE::TESHitEvent::Flag::kSneakAttack);
            evt.isBashAttack = a_event->flags.any(RE::TESHitEvent::Flag::kBashAttack);
            evt.isHitBlocked = a_event->flags.any(RE::TESHitEvent::Flag::kHitBlocked);

            SKSE::log::trace("TESHitEvent(target=<{:08X}:{}>, aggressor=<{:08X}:{}>, source=<{:08X}>, projectile=<{:08X}>, {}, {}, {}, {})",
                a_event->target->GetFormID(), a_event->target->GetName(),
                (a_event->cause == nullptr) ? 0 : a_event->cause->GetFormID(), (a_event->cause == nullptr) ? "NULL" : a_event->cause->GetName(), 
                a_event->source, a_event->projectile,
                evt.isPowerAttack ? "Power" : "-", evt.isSneakAttack ? "Sneak" : "-", evt.isBashAttack ? "Bash" : "-",
                evt.isHitBlocked ? "Blocked" : "-");


            _defeatManager->getCombatManager()->onHitHandler(evt);

            return RE::BSEventNotifyControl::kContinue;
        }

    };

    class OnTESEnterBleedoutEventHandler : public RE::BSTEventSink<RE::TESEnterBleedoutEvent> {
    public:
        OnTESEnterBleedoutEventHandler(SexLabDefeat::DefeatManager* defeatManager) { _defeatManager = defeatManager; };
        ~OnTESEnterBleedoutEventHandler() = default;
        SexLabDefeat::DefeatManager* _defeatManager;

        virtual RE::BSEventNotifyControl ProcessEvent(const RE::TESEnterBleedoutEvent* a_event,
                                                      RE::BSTEventSource<RE::TESEnterBleedoutEvent>* a_eventSource) {
            if (a_event->actor != nullptr) {
                auto target_actor = a_event->actor->As<RE::Actor>();
                if (target_actor != nullptr) {
                    SKSE::log::info("OnTESEnterBleedoutEventHandler <{:08X}:{}> Enter Bleedout", target_actor->GetFormID(),
                                    target_actor->GetName());
                    _defeatManager->getCombatManager()->onActorEnterBleedout(target_actor);
                }
            }
            return RE::BSEventNotifyControl::kContinue;
        }
    };

    class OnTESEquipEventHandler : public RE::BSTEventSink<RE::TESEquipEvent> {
    public:
        OnTESEquipEventHandler(SexLabDefeat::IDefeatManager* defeatManager) { _defeatManager = defeatManager; };
        ~OnTESEquipEventHandler() = default;
        SexLabDefeat::IDefeatManager* _defeatManager;

        virtual RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* a_event,
                                                      RE::BSTEventSource<RE::TESEquipEvent>* a_eventSource) {

            SKSE::log::trace("OnTESEquipEventHandler, {:08X} - {} - {:08X}", a_event->actor->GetFormID(),
                             a_event->equipped, a_event->baseObject);
            return RE::BSEventNotifyControl::kContinue;
        }
    };

    class OnInputEventHandler : public RE::BSTEventSink<RE::InputEvent*> {
    public:
        OnInputEventHandler(SexLabDefeat::IDefeatManager* defeatManager) { _defeatManager = defeatManager; };
        ~OnInputEventHandler() = default;
        SexLabDefeat::IDefeatManager* _defeatManager;

        virtual RE::BSEventNotifyControl ProcessEvent(RE::InputEvent* const* a_event,
                                                      RE::BSTEventSource<RE::InputEvent*>* a_eventSource) {
            if (!a_event) {
                return RE::BSEventNotifyControl::kContinue;
            }

            for (auto* event = *a_event; event; event = event->next) {
                if (event->eventType != RE::INPUT_EVENT_TYPE::kButton) {
                    return RE::BSEventNotifyControl::kContinue;
                }

                const auto* button = event->AsButtonEvent();

                if (button->IsUp()) {
                    SKSE::log::trace("OnInputEventHandler, {}", button->GetIDCode());
                }
            }
            return RE::BSEventNotifyControl::kContinue;
        }
    };
}

namespace Hooks {
    using clock = std::chrono::high_resolution_clock;
    SexLabDefeat::DefeatManager* manager;

    static void UpdateCombatControllerSettings(RE::Character* a_this);
    static inline REL::Relocation<decltype(UpdateCombatControllerSettings)> _UpdateCombatControllerSettings;

    std::map<RE::FormID, clock::time_point> _sexLabInterruptExpirations;
    SexLabDefeat::SpinLock _sexLabInterruptExpirationsLock;

    void UpdateCombatControllerSettings(RE::Character* character) {
        _UpdateCombatControllerSettings(character);

        /* if (character && character->IsInFaction(manager->Forms.Faction.DefeatFaction)) {
            if (character->IsInCombat()) {
                character->StopCombat();
            }
            return;
        }*/
        auto group = character->GetCombatGroup();
        if (!group) {
            return;
        }
        auto now = clock::now();

        group->lock.LockForRead();

        for (auto&& cmbtarget : group->targets) {
            auto targetptr = cmbtarget.targetHandle.get();
            if (targetptr) {
                auto targetActor = targetptr.get();
                if (targetActor) {
                    SKSE::log::info("UpdateCombatControllerSettings <{:08X}:{}> -> <{:08X}:{}>", character->GetFormID(),
                                    character->GetName(), targetActor->GetFormID(), targetActor->GetName());
                }
                if (targetActor && !targetActor->IsPlayer() &&
                    targetActor->HasKeywordString(manager->Forms.KeywordId.SexLabActive)) {
                    _sexLabInterruptExpirationsLock.spinLock();
                    auto val = _sexLabInterruptExpirations.find(targetActor->GetFormID());
                    if (val == _sexLabInterruptExpirations.end() || (now > val->second)) {
                        _sexLabInterruptExpirations.insert_or_assign(targetActor->GetFormID(), now + 5000ms);
                        _sexLabInterruptExpirationsLock.spinUnlock();

                        auto target = manager->getActorManager()->getDefeatActor(targetActor);
                        auto aggressor = manager->getActorManager()->getDefeatActor(character);
                        manager->getCombatManager()->onSexLabSceneInterrupt(target, aggressor, true);
                    } else {
                        _sexLabInterruptExpirationsLock.spinUnlock();
                    }
                }
            }
        }

        group->lock.UnlockForRead();
    }
}

void SexLabDefeat::installHooks(SexLabDefeat::DefeatManager* defeatManager) {
    Hooks::manager = defeatManager;

    SKSE::log::info("Install hooks pre");
    if (defeatManager->getConfig()->Hooks.UpdateCombatControllerSettings > 0) {
        //REL::Relocation<std::uintptr_t> upccs{RE::Character::VTABLE[0]};
        //Hooks::_UpdateCombatControllerSettings = upccs.write_vfunc(0x11B, Hooks::UpdateCombatControllerSettings);
    }
    SKSE::log::info("Install hooks post");
}

void SexLabDefeat::installEventSink(SexLabDefeat::DefeatManager* defeatManager) {
    SKSE::log::info("Install EventSink pre");
    auto scriptEventSource = RE::ScriptEventSourceHolder::GetSingleton();
    if (scriptEventSource) {
        scriptEventSource->AddEventSink(new EventSync::OnTESCombatEventHandler(defeatManager));
        scriptEventSource->AddEventSink(new EventSync::OnTESHitEventHandler(defeatManager));
        scriptEventSource->AddEventSink(new EventSync::OnTESEnterBleedoutEventHandler(defeatManager));
        //scriptEventSource->AddEventSink(new EventSync::OnTESEquipEventHandler(defeatManager));
    } else {
        SKSE::log::critical("Install EventSink failed");
    }
    SKSE::log::info("Install EventSink post");
}

void SexLabDefeat::installInputEventSink(SexLabDefeat::DefeatManager* defeatManager) {
    SKSE::log::info("Install Input EventSink pre");
    auto inputEvent = RE::BSInputDeviceManager::GetSingleton();
    if (inputEvent) {
        //inputEvent->AddEventSink(new EventSync::OnInputEventHandler(defeatManager));
    } else {
        SKSE::log::critical("Install Input EventSink failed");
    }
    SKSE::log::info("Install Input EventSink post");
}
