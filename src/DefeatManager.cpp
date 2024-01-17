#include "Defeat.h"

namespace {

    SexLabDefeat::DefeatManager* _defeatManager;

    inline void responseActorExtraData(PAPYRUSFUNCHANDLE, RE::Actor* actor, bool ignoreActorOnHit, int sexLabGender,
        int sexLabSexuality, bool sexLabAllowed, std::string sexLabRaceKey,
        bool defeatAllowed2PC, bool defeatAllowed2NvN) {
        if (actor == nullptr) {
            return;
        }
        SKSE::log::trace("Papyrus call responseActorExtraData(<{:08X}:{}>, {}, {}, {}, {}, {})", actor->GetFormID(), actor->GetName(), ignoreActorOnHit, sexLabGender, sexLabSexuality, sexLabAllowed,
                         sexLabRaceKey);

        auto defeatActor = _defeatManager->getActorManager()->getActor(actor);
        SexLabDefeat::ActorExtraData data;
        data.ignoreActorOnHit = ignoreActorOnHit;
        data.sexLabGender = sexLabGender;
        data.sexLabSexuality = sexLabSexuality;
        data.sexLabAllowed = sexLabAllowed;
        data.sexLabRaceKey = sexLabRaceKey;
        //data.defeatAllowed2PC = defeatAllowed2PC;
        //data.defeatAllowed2NvN = defeatAllowed2NvN;
        defeatActor.get()->extraData->initializeValue(data);
    }

    inline void setActorVulnerability(PAPYRUSFUNCHANDLE, RE::Actor* actor, float vulnerability) {
        if (actor == nullptr) {
            return;
        }
        SKSE::log::trace("Papyrus call setActorVulnerability(<{:08X}:{}>, {})", actor->GetFormID(), actor->GetName(),
                         static_cast<int>(vulnerability));

        auto defeatActor = _defeatManager->getActorManager()->getActor(actor);
        defeatActor->setVulnerability(vulnerability);
    }

    inline void setActorState(PAPYRUSFUNCHANDLE, RE::Actor* actor, std::string state) {
        if (actor == nullptr) {
            return;
        }
        SKSE::log::trace("Papyrus call setActorState(<{:08X}:{}>, {})", actor->GetFormID(), actor->GetName(), state);

        std::transform(state.begin(), state.end(), state.begin(), ::toupper);
        SexLabDefeat::DefeatActor::States _state = SexLabDefeat::DefeatActor::States::NONE;
        if (state.compare("ACTIVE") == 0) {
            _state = SexLabDefeat::DefeatActor::States::ACTIVE;
        } else if (state.compare("DISACTIVE") == 0) {
            _state = SexLabDefeat::DefeatActor::States::DISACTIVE;
        }
        _defeatManager->setActorState(actor, _state);
    }

    bool RegisterPapyrusFunctions(RE::BSScript::IVirtualMachine* vm) {
        const bool loc_unhook = _defeatManager->getConfig()->CFG_PAPYUNHOOK;

#define REGISTERPAPYRUSFUNC(name, unhook) \
    { vm->RegisterFunction(#name, "defeat_skse_api", name, unhook&& loc_unhook); }

        REGISTERPAPYRUSFUNC(responseActorExtraData, true)
        REGISTERPAPYRUSFUNC(setActorVulnerability, true)
        REGISTERPAPYRUSFUNC(setActorState, true)

#undef REGISTERPAPYRUSFUNC
            return true;
    }
}

namespace SexLabDefeat {
    extern template class DeferredExpiringValue<ActorExtraData>;

    DefeatManager::DefeatManager(DefeatConfig* defeatConfig) {
        setGameState(DefeatManager::GameState::NONE);
        _defeatConfig = defeatConfig;
        _defeatActorManager = new DefeatActorManager(this);
        _defeatCombatManager = new DefeatCombatManager(_defeatActorManager, this);
    }

    void DefeatManager::load() {
        _defeatManager = this;

        if (SKSE::GetPapyrusInterface()->Register(RegisterPapyrusFunctions)) {
            SKSE::log::info("Papyrus functions bound.");
        } else {
            SKSE::stl::report_and_fail("Failure to register Papyrus bindings.");
        }
    }

    void DefeatManager::reset() {
        _defeatActorManager->reset();
        _defeatCombatManager->interruptPlayerDeplateDynamicDefeat();
        initializeDependency();
        initializeForms();
        _defeatConfig->Reset();
        reInitializeWidget();
        setGameState(DefeatManager::GameState::IN_GAME);
    }

    void DefeatManager::reInitializeWidget() const {
        _defeatManager->setWidget(nullptr);
        if (Forms.DefeatPlayerQTE == nullptr) {
            SKSE::log::error("LoadForms : Not found TESQuest 'DefeatPlayerQTE'");
        } else {
            auto DefeatQTEMeter =
                SexLabDefeat::Papyrus::GetScriptObject(_defeatManager->Forms.DefeatPlayerQTE, "defeatmcmscr");
            if (DefeatQTEMeter == nullptr) {
                SKSE::log::error("LoadForms : Not found attached Script 'DefeatQTEMeter'");
            } else {
                auto defeatWidget = new DefeatWidget();
                defeatWidget->widgetRoot = DefeatWidget::StringVarPtr(new DefeatWidget::StringVar(
                    DefeatQTEMeter, "_widgetRoot"sv, PapyrusInterface::ObjectVariableConfig(true, true)));
                _defeatManager->setWidget(defeatWidget);
            }
        }
    }

    void DefeatManager::initializeDependency() {
        RE::BSSimpleList<RE::TESFile*>* plugins = &RE::TESDataHandler::GetSingleton()->files;

        for (auto&& it : *plugins) {
            const auto loc_indx = RE::TESDataHandler::GetSingleton()->GetModIndex(it->GetFilename());

            if (!loc_indx.has_value() || loc_indx.value() == 255U) {
                continue;
            };
            if (!SoftDependency.ZaZ && it->GetFilename() == "ZaZAnimationPack.esm") {
                SoftDependency.ZaZ = true;
            };
        };
    }

    void DefeatManager::initializeForms() {
        RE::TESDataHandler* handler = RE::TESDataHandler::GetSingleton();

#define LOAD_FORM(NAME, TYPE, ID, PLUGIN)                                                                  \
        NAME = handler->LookupForm<TYPE>(ID, PLUGIN);                                                          \
        if (NAME == nullptr) {                                                                                 \
            SKSE::log::error("initializeForms : Not found <" #TYPE " - " #ID "> '" #NAME "' in '" PLUGIN "'"); \
        }

        LOAD_FORM(Forms.DefeatMCMQst, RE::TESQuest, 0x06D3D4, "SexLabDefeat.esp");
        LOAD_FORM(Forms.DefeatRessourcesQst, RE::TESQuest, 0x04B8D1, "SexLabDefeat.esp");
        LOAD_FORM(Forms.DefeatPlayerQTE, RE::TESQuest, 0x0B5F7C, "SexLabDefeat.esp");

        LOAD_FORM(Forms.SexLabQuestFramework, RE::TESQuest, 0x000D62, "SexLab.esm");

        LOAD_FORM(Forms.SatisfiedSPL, RE::SpellItem, 0x0D6FF0, "SexLabDefeat.esp");

        LOAD_FORM(Forms.MiscQuests.PlayerQST, RE::TESQuest, 0x000D62, "SexLabDefeat.esp");
        LOAD_FORM(Forms.MiscQuests.PAQst, RE::TESQuest, 0x0BD631, "SexLabDefeat.esp");
        LOAD_FORM(Forms.MiscQuests.PlayerActionQst, RE::TESQuest, 0x036014, "SexLabDefeat.esp");
        LOAD_FORM(Forms.MiscQuests.NPCsQst, RE::TESQuest, 0x06E968, "SexLabDefeat.esp");
        LOAD_FORM(Forms.MiscQuests.NPCsRefreshQst, RE::TESQuest, 0x06E96C, "SexLabDefeat.esp");
        LOAD_FORM(Forms.MiscQuests.Robber, RE::TESQuest, 0x0C16D7, "SexLabDefeat.esp");
        LOAD_FORM(Forms.MiscQuests.DGIntimidateQuest, RE::TESQuest, 0x047AE6, "Skyrim.esm");
        LOAD_FORM(Forms.MiscQuests.WerewolfQst, RE::TESQuest, 0x02BA16, "Skyrim.esm");

        LOAD_FORM(Forms.MiscMagicEffects.ImmunityEFF, RE::EffectSetting, 0x0D44CF, "SexLabDefeat.esp");
        LOAD_FORM(Forms.MiscMagicEffects.HKActionEFF, RE::EffectSetting, 0x04DE88, "SexLabDefeat.esp");
        LOAD_FORM(Forms.MiscMagicEffects.HKFollowerActionEFF, RE::EffectSetting, 0x059B45, "SexLabDefeat.esp");
        LOAD_FORM(Forms.MiscMagicEffects.SexCrimeEFF, RE::EffectSetting, 0x107D97, "SexLabDefeat.esp");
        LOAD_FORM(Forms.MiscMagicEffects.NVNAssaultEFF, RE::EffectSetting, 0x06E969, "SexLabDefeat.esp");

#undef LOAD_FORM
    }

    HitEventType DefeatManager::createHitEvent(RE::Actor* target_actor,
                                                                           RE::Actor* aggr_actor,
                                                                           RawHitEvent rawHitEvent) {
        HitEventType event = std::make_shared<HitEvent>();

        event->target = _defeatActorManager->getActor(target_actor);
        event->aggressor = _defeatActorManager->getActor(aggr_actor);
        event->source = rawHitEvent.source;
        event->projectile = rawHitEvent.projectile;
        event->isPowerAttack = rawHitEvent.isPowerAttack;
        event->isSneakAttack = rawHitEvent.isSneakAttack;
        event->isBashAttack = rawHitEvent.isBashAttack;
        event->isHitBlocked = rawHitEvent.isHitBlocked;

        return event;
    }

    void DefeatManager::setWidget(DefeatWidget* widget) {
        if (_defeatWidget != nullptr) {
            _defeatWidget->spinLock();
            delete _defeatWidget;
        }
        _defeatWidget = widget;
    }

    DefeatWidget* DefeatManager::getWidget() {
        if (_defeatWidget == nullptr) {
            return nullptr;
        }
        _defeatWidget->spinLock();
        auto widget = _defeatWidget;
        _defeatWidget->spinUnlock();
        return widget;
    }

    void DefeatManager::setActorState(RE::Actor* target_actor, DefeatActor::States state) {
        auto defeatActor = getActorManager()->getActor(target_actor);
        if (state != DefeatActor::States::NONE) {
            defeatActor->setState(state);
            defeatActor->resetDynamicDefeat();
            if (defeatActor->isPlayer()) {
                _defeatCombatManager->interruptPlayerDeplateDynamicDefeat();
                auto widget = getWidget();
                if (widget != nullptr && widget->getState() == DefeatWidget::State::DYNAMIC_WIDGET) {
                    if (!getWidget()->stopDynamicWidget()) {
                        SKSE::log::error("Error on stop Dynamic Widget");
                    }
                }
            }
        }
    }

    bool DefeatManager::randomChanse(float chanse, float min, float max) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> distr(min, max);

        return distr(gen) < chanse;
    }

    DefeatManager::GameState DefeatManager::getGameState() { return _gameState.load(); }
    void DefeatManager::setGameState(DefeatManager::GameState state) { _gameState.store(state); }

    void DefeatManager::ActorEnterdToCombatState(RE::Actor* target_actor) {
        auto defActor = _defeatActorManager->getActor(target_actor);
        defActor.get()->extraData->getCallback([this, defActor] {
            SKSE::log::info("Extra data received for <{:08X}:{}>", defActor->getActor()->GetFormID(),
                            defActor->getActor()->GetName());
        });
    }
}