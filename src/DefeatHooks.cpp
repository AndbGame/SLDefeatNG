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
                    _defeatManager->ActorEnterdToCombatState(target_actor);
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
        OnTESEquipEventHandler(SexLabDefeat::DefeatManager* defeatManager) { _defeatManager = defeatManager; };
        ~OnTESEquipEventHandler() = default;
        SexLabDefeat::DefeatManager* _defeatManager;

        virtual RE::BSEventNotifyControl ProcessEvent(const RE::TESEquipEvent* a_event,
                                                      RE::BSTEventSource<RE::TESEquipEvent>* a_eventSource) {

            //SKSE::log::trace("OnTESEquipEventHandler, {:08X} - {} - {:08X}", a_event->actor->GetFormID(),
            //                 a_event->equipped, a_event->baseObject);
            return RE::BSEventNotifyControl::kContinue;
        }
    };
}


namespace stl {
    using namespace SKSE::stl;

    //void asm_replace(std::uintptr_t a_from, std::size_t a_size, std::uintptr_t a_to);

    template <class T>
    void asm_replace(std::uintptr_t a_from) {
        asm_replace(a_from, T::size, reinterpret_cast<std::uintptr_t>(T::func));
    }

    template <class T>
    void write_thunk_call(std::uintptr_t a_src) {
        auto& trampoline = SKSE::GetTrampoline();
        SKSE::AllocTrampoline(14);

        T::func = trampoline.write_call<5>(a_src, T::thunk);
    }

    template <class F, size_t offset, class T>
    void write_vfunc() {
        REL::Relocation<std::uintptr_t> vtbl{F::VTABLE[offset]};
        T::func = vtbl.write_vfunc(T::idx, T::thunk);
    }

    template <class F, class T>
    void write_vfunc() {
        write_vfunc<F, 0, T>();
    }

    inline std::string as_string(std::string_view a_view) { return {a_view.data(), a_view.size()}; }
}

    struct Load3D {
        static RE::NiAVObject* thunk(RE::Actor* a_actor, bool a_backgroundLoading) {
            auto* node = func(a_actor, a_backgroundLoading);

            SKSE::log::trace("Load3D, {:08X} - {} - {}", a_actor->GetFormID(),
                             a_actor->GetDisplayFullName(),
                             a_backgroundLoading);
            return node;
        }
        static inline REL::Relocation<decltype(thunk)> func;

    };

void SexLabDefeat::installHooks(SexLabDefeat::DefeatManager* defeatManager) {
    //SKSE::log::info("Install hooks pre");
    REL::Relocation<std::uintptr_t> target{REL::ID(37177), 0xD};
    stl::write_thunk_call<Load3D>(target.address());
    //SKSE::log::info("Install hooks post");
}

void SexLabDefeat::installEventSink(SexLabDefeat::DefeatManager* defeatManager) {
    SKSE::log::info("Install EventSink pre");
    auto scriptEventSource = RE::ScriptEventSourceHolder::GetSingleton();
    if (scriptEventSource) {
        scriptEventSource->AddEventSink(new EventSync::OnTESCombatEventHandler(defeatManager));
        scriptEventSource->AddEventSink(new EventSync::OnTESHitEventHandler(defeatManager));
        scriptEventSource->AddEventSink(new EventSync::OnTESEquipEventHandler(defeatManager));
        SKSE::log::info("Install EventSink post");
    } else {
        SKSE::log::critical("Install EventSink failed");
    }
}
