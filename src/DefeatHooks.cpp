#include "DefeatHooks.h"

#include <Windows.h>
#include <detours/detours.h>

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
            if (defActor && defActor->getState() == SexLabDefeat::DefeatActorStates::VICTIM_KNONKDOWN_STATE &&
                a_event->tag == "MTState") {
                SKSE::log::trace(
                    "OnBSAnimationGraphEvent for <{:08X}:{}> event tag: <{}>; payload: <{}>; play Bleedout",
                    source->GetFormID(), source->GetName(), a_event->tag.c_str(), a_event->payload.c_str());
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
#include <boost/stacktrace.hpp>
namespace Hooks {
    SexLabDefeat::DefeatManager* manager;
    
	uint8_t* Hooks::DoDetect(RE::Actor* viewer, RE::Actor* target, int32_t& detectval, uint8_t& unk04, uint8_t& unk05,
                             uint32_t& unk06, RE::NiPoint3& pos, float& unk08, float& unk09, float& unk10) {
        if (target && manager->getActorManager()->getDefeatActorImplState(target->GetFormID()) !=
                          SexLabDefeat::DefeatActorStates::ACTIVE) {
            TRACE("DoDetect interrupt: {:08X} -> {:08X}", viewer->GetFormID(), target->GetFormID());
            return nullptr;
        }
        if (viewer && manager->getActorManager()->getDefeatActorImplState(viewer->GetFormID()) !=
                          SexLabDefeat::DefeatActorStates::ACTIVE) {
            return nullptr;
        }
        return _DoDetect(viewer, target, detectval, unk04, unk05, unk06, pos, unk08, unk09, unk10);
    }

    void DrawWeaponMagicHands(RE::Actor* actor, bool a_draw) {
        if (a_draw && manager->getActorManager()->getDefeatActorImplState(actor->GetFormID()) !=
                          SexLabDefeat::DefeatActorStates::ACTIVE) {
            TRACE("DrawWeaponMagicHands interrupt: {:08X}", actor->GetFormID());
            return;
        }
        TRACE("DrawWeaponMagicHands {} : {:08X}: {}", a_draw, actor->GetFormID(),
                         (std::uint32_t)actor->AsActorState()->actorState2.weaponState);


        _DrawWeaponMagicHands(actor, a_draw);
    }

}

void SexLabDefeat::installHooks(SexLabDefeat::DefeatManager* defeatManager) {
    Hooks::manager = defeatManager;

    SKSE::log::info("Install hooks pre");
    SKSE::AllocTrampoline(14 * 2);
    auto& trampoline = SKSE::GetTrampoline();

    REL::Relocation<std::uintptr_t> det{REL::RelocationID(41659, 42742), REL::VariantOffset(0x526, 0x67B, 0)};
    Hooks::_DoDetect = trampoline.write_call<5>(det.address(), Hooks::DoDetect);

    REL::Relocation<std::uintptr_t> actor_vt{RE::VTABLE_Actor[0]};
    REL::Relocation<std::uintptr_t> character_vt{RE::Character::VTABLE[0]};
    Hooks::_DrawWeaponMagicHands = actor_vt.write_vfunc(0xA6, Hooks::DrawWeaponMagicHands);
    Hooks::_DrawWeaponMagicHands = character_vt.write_vfunc(0xA6, Hooks::DrawWeaponMagicHands);

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
