#include "DefeatHooks.h"

namespace EventSync {

    class OnTESCombatEventHandler : public RE::BSTEventSink<RE::TESCombatEvent> {
    public:
        OnTESCombatEventHandler(SexLabDefeat::DefeatManager* defeatManager) { _defeatManager = defeatManager; };
        ~OnTESCombatEventHandler() = default;
        SexLabDefeat::DefeatManager* _defeatManager;

        virtual RE::BSEventNotifyControl ProcessEvent(const RE::TESCombatEvent* a_event,
            RE::BSTEventSource<RE::TESCombatEvent>* a_eventSource) {
            if (a_event->actor != nullptr && a_event->newState.any(RE::ACTOR_COMBAT_STATE::kCombat)) {
                auto target_actor = a_event->actor->As<RE::Actor>();
                if (target_actor != nullptr) {
                    SKSE::log::info("OnTESCombatEventHandler <{:08X}:{}> in combat state",
                                    target_actor->GetFormID(), target_actor->GetName());
                    _defeatManager->getCombatManager()->onActorEnteredToCombatState(target_actor);
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

void SexLabDefeat::installHooks(SexLabDefeat::DefeatManager* defeatManager) {
    //SKSE::log::info("Install hooks pre");
    //SKSE::log::info("Install hooks post");
}

void SexLabDefeat::installEventSink(SexLabDefeat::DefeatManager* defeatManager) {
    SKSE::log::info("Install EventSink pre");
    auto scriptEventSource = RE::ScriptEventSourceHolder::GetSingleton();
    if (scriptEventSource) {
        scriptEventSource->AddEventSink(new EventSync::OnTESCombatEventHandler(defeatManager));
        scriptEventSource->AddEventSink(new EventSync::OnTESHitEventHandler(defeatManager));
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
