#pragma once

namespace SexLabDefeat {

    struct DefeatForms {
        RE::TESQuest* DefeatRessourcesQst = nullptr;
        RE::TESQuest* DefeatMCMQst = nullptr;
        RE::TESQuest* DefeatPlayerQTE = nullptr;

        RE::TESQuest* SexLabQuestFramework = nullptr;

        RE::SpellItem* SatisfiedSPL = nullptr;

        struct {
            RE::TESQuest* PlayerQST = nullptr;
            RE::TESQuest* PAQst = nullptr;
            RE::TESQuest* PlayerActionQst = nullptr;
            RE::TESQuest* NPCsQst = nullptr;
            RE::TESQuest* NPCsRefreshQst = nullptr;
            RE::TESQuest* Robber = nullptr;
            RE::TESQuest* DGIntimidateQuest = nullptr;
            RE::TESQuest* WerewolfQst = nullptr;
        } MiscQuests;

        struct {
            RE::EffectSetting* ImmunityEFF = nullptr;
            RE::EffectSetting* HKActionEFF = nullptr;
            RE::EffectSetting* HKFollowerActionEFF = nullptr;
            RE::EffectSetting* SexCrimeEFF = nullptr;
            RE::EffectSetting* NVNAssaultEFF = nullptr;
        } MiscMagicEffects;

        struct {
            RE::TESQuest* DefeatVulnerability = nullptr;
        } LRGPatch;

        struct {
        } Keywords;
    };
}