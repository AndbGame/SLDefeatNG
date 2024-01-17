#include "Defeat.h"

namespace SexLabDefeat {
    /*
    template float DefeatConfig::getConfig<float>(std::string configKey, float _def) const;
    template bool DefeatConfig::getConfig<bool>(std::string configKey, bool _def) const;

    template bool DefeatConfig::getSslConfig<bool>(std::string configKey, bool _def) const;
    */
    void DefeatConfig::readIniConfig() {
        _iniConfig = boost::property_tree::ptree();
        try {
            boost::property_tree::ini_parser::read_ini("Data\\skse\\plugins\\SexLabDefeat.ini", _iniConfig);

            CFG_PAPYUNHOOK = _iniConfig.get<int>("General.UnhookPapyrus");
            CFG_LOGGING = _iniConfig.get<int>("General.Logging");
            HIT_SPAM_GUARD_EXPIRATION_MS = _iniConfig.get<int>("CombatManager.HitSpamGuardMs");
        } catch (std::exception& ex) {
            SKSE::log::warn("ERROR LOADING ini FILE: {}", ex.what());
        }
    }

    void DefeatConfig::Setup(SexLabDefeat::DefeatManager* defeatManager) { _defeatManager = defeatManager; }

    void DefeatConfig::Reset() {
        LoadScriptObjects();
        
#define BOOL_PROPERTY(NAME)                                                                 \
        Config.NAME = PapyrusInterface::BoolVarPtr(new PapyrusInterface::BoolVar(                  \
            DefeatMCMScr, std::string_view(#NAME), PapyrusInterface::ObjectVariableConfig(true, false)))

#define FLOAT_PROPERTY(NAME)                                            \
        Config.NAME = PapyrusInterface::FloatVarPtr(new PapyrusInterface::FloatVar( \
        DefeatMCMScr, std::string_view(#NAME), PapyrusInterface::ObjectVariableConfig(true, false)))


        
        FLOAT_PROPERTY(PvicRaped);
        FLOAT_PROPERTY(NVNRapedFollower);
        FLOAT_PROPERTY(NVNRaped);
        BOOL_PROPERTY(EveryonePvic);
        BOOL_PROPERTY(HuntCrea);

        BOOL_PROPERTY(SexualityPvic);
        BOOL_PROPERTY(SexualityNVN);
        BOOL_PROPERTY(MaleHunterPvic);
        BOOL_PROPERTY(FemaleHunterPvic);
        BOOL_PROPERTY(HuntFCrea);
        BOOL_PROPERTY(MaleOnGal);
        BOOL_PROPERTY(GalOnGal);
        BOOL_PROPERTY(MaleOnMale);
        BOOL_PROPERTY(GalOnMale);
        BOOL_PROPERTY(CreaOnFemale);
        BOOL_PROPERTY(CreaFemaleOnFemale);
        BOOL_PROPERTY(CreaFemaleOnMale);
        BOOL_PROPERTY(CreaOnMale);
        BOOL_PROPERTY(BeastImmunity);


        BOOL_PROPERTY(KDWayThreshold);
        BOOL_PROPERTY(KDHealthBlock);
        FLOAT_PROPERTY(ChanceOnHitPvic);
        FLOAT_PROPERTY(ThresholdPvic);
        FLOAT_PROPERTY(ThresholdPvicMin);
        FLOAT_PROPERTY(KnockOutHPvic);
        FLOAT_PROPERTY(SStruggleHealthPvic);

        BOOL_PROPERTY(KDWayStamina);
        BOOL_PROPERTY(KDStaminaBlock);
        BOOL_PROPERTY(KDWayStaminaOB);
        FLOAT_PROPERTY(ChanceOnHitPvicS);
        FLOAT_PROPERTY(ExhaustionPvic);
        FLOAT_PROPERTY(KnockOutSPvic);
        FLOAT_PROPERTY(SStruggleExhaustionPvic);

        BOOL_PROPERTY(KDWayPowerAtk);
        FLOAT_PROPERTY(KDWayPowerAtkCOH);
        FLOAT_PROPERTY(PowerAtkStagger);
        FLOAT_PROPERTY(KnockOutPPvic);
        FLOAT_PROPERTY(SStrugglePowerPvic);

        BOOL_PROPERTY(bResistQTE);

#undef BOOL_PROPERTY
#undef FLOAT_PROPERTY

#define BOOL_PROPERTY_LRG(NAME)                                                        \
    Config.LRGPatch.NAME = PapyrusInterface::BoolVarPtr(new PapyrusInterface::BoolVar( \
        DefeatMCMScr, std::string_view(#NAME), PapyrusInterface::ObjectVariableConfig(true, false)))

#define FLOAT_PROPERTY_LRG(NAME)                                                         \
    Config.LRGPatch.NAME = PapyrusInterface::FloatVarPtr(new PapyrusInterface::FloatVar( \
        DefeatMCMScr, std::string_view(#NAME), PapyrusInterface::ObjectVariableConfig(true, false)))

        /* LRG */
        BOOL_PROPERTY_LRG(KDWayVulnerability);
        BOOL_PROPERTY_LRG(KDVulnerabilityBlock);
        BOOL_PROPERTY_LRG(KDWayVulnerabilityOB);
        FLOAT_PROPERTY_LRG(ChanceOnHitPvicVulnerability);
        FLOAT_PROPERTY_LRG(VulnerabilityPvic);
        FLOAT_PROPERTY_LRG(KnockOutVulnerabilityPvic);
        FLOAT_PROPERTY_LRG(SStruggleVulnerabilityPvic);

        BOOL_PROPERTY_LRG(KDWayDynamic);
        FLOAT_PROPERTY_LRG(KnockOutDynamicPvic);
        FLOAT_PROPERTY_LRG(SStruggleDynamicPvic);
        FLOAT_PROPERTY_LRG(DynamicDefeatOnHitBase);
        FLOAT_PROPERTY_LRG(DynamicDefeatOnHitOneHand);
        FLOAT_PROPERTY_LRG(DynamicDefeatOnHitTwoHand);
        FLOAT_PROPERTY_LRG(DynamicDefeatOnHitBow);
        FLOAT_PROPERTY_LRG(DynamicDefeatOnHitSpell);
        FLOAT_PROPERTY_LRG(DynamicDefeatVulnerabilityMult);
        FLOAT_PROPERTY_LRG(DynamicDefeatPowerAttackMult);
        FLOAT_PROPERTY_LRG(DynamicDefeatLowStaminaMult);
        FLOAT_PROPERTY_LRG(DynamicDefeatLowStaminaThreshold);
        FLOAT_PROPERTY_LRG(DynamicDefeatLowHealthMult);
        FLOAT_PROPERTY_LRG(DynamicDefeatLowHealthThreshold);
        FLOAT_PROPERTY_LRG(DynamicDefeatBackHitMult);
        FLOAT_PROPERTY_LRG(DynamicDefeatBlockReduction);
        FLOAT_PROPERTY_LRG(DynamicDefeatDepleteOverTime);
        /* /LRG */
#undef BOOL_PROPERTY_LRG
#undef FLOAT_PROPERTY_LRG

        Config.SexLab.UseCreatureGender = PapyrusInterface::BoolVarPtr(
            new PapyrusInterface::BoolVar(sslSystemConfig, std::string_view("UseCreatureGender"),
                                          PapyrusInterface::ObjectVariableConfig(true, false)));

        Config.RaceAllowedNVN = PapyrusInterface::StringSetVarPtr(new PapyrusInterface::StringSetVar(
            defeatconfig, "RaceAllowedNVN"sv, PapyrusInterface::ObjectVariableConfig(false)));
        Config.RaceAllowedPvic = PapyrusInterface::StringSetVarPtr(new PapyrusInterface::StringSetVar(
            defeatconfig, "RaceAllowedPvic"sv, PapyrusInterface::ObjectVariableConfig(false)));


    }

    void DefeatConfig::LoadScriptObjects() {

        if (_defeatManager->Forms.DefeatMCMQst == nullptr) {
            SKSE::log::error("LoadForms : Not found TESQuest 'DefeatMCMQst'");
        } else {
            DefeatMCMScr = SexLabDefeat::Papyrus::GetScriptObject(_defeatManager->Forms.DefeatMCMQst, "defeatmcmscr");
            if (DefeatMCMScr == nullptr) {
                SKSE::log::error("LoadForms : Not found attached Script 'defeatmcmscr'");
            }
        }

        if (_defeatManager->Forms.DefeatRessourcesQst == nullptr) {
            SKSE::log::error("LoadForms : Not found TESQuest 'DefeatRessourcesQst'");
        } else {
            defeatconfig =
                SexLabDefeat::Papyrus::GetScriptObject(_defeatManager->Forms.DefeatRessourcesQst, "defeatconfig");
            if (defeatconfig == nullptr) {
                SKSE::log::error("LoadForms : Not found attached Script 'defeatconfig'");
            }
        }

        if (_defeatManager->Forms.SexLabQuestFramework == nullptr) {
            SKSE::log::error("LoadForms : Not found TESQuest 'SexLabQuestFramework'");
        } else {
            sslSystemConfig =
                SexLabDefeat::Papyrus::GetScriptObject(_defeatManager->Forms.SexLabQuestFramework, "sslSystemConfig");
            if (sslSystemConfig == nullptr) {
                SKSE::log::error("LoadForms : Not found attached Script 'sslSystemConfig'");
            }
        }
    }
    /*
    template <class T>
    T DefeatConfig::getSslConfig(std::string configKey, T _def) const {
        if (sslSystemConfig != nullptr) {
            T ret = ::SexLabDefeat::Papyrus::GetProperty<T>(sslSystemConfig, configKey);
            // SKSE::log::trace("getSslConfig({}) = {}", configKey, ret);
            return ret;
        } else {
            SKSE::log::critical("getSslConfig - DefeatConfig.sslSystemConfig is nullptr");
            return _def;
        }
    }

    template <class T>
    T DefeatConfig::getConfig(std::string configKey, T _def) const {
        if (DefeatMCMScr != nullptr) {
            T ret = ::SexLabDefeat::Papyrus::GetProperty<T>(DefeatMCMScr, configKey);
            // SKSE::log::trace("getConfig({}) = {}", configKey, ret);
            return ret;
        } else {
            SKSE::log::critical("getConfig - DefeatConfig.DefeatMCMScr is nullptr");
            return _def;
        }
    }
    */
}