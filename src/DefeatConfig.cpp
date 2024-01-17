#include "Defeat.h"

namespace SexLabDefeat {

    template float DefeatConfig::getConfig<float>(std::string configKey, float _def) const;
    template bool DefeatConfig::getConfig<bool>(std::string configKey, bool _def) const;

    template bool DefeatConfig::getSslConfig<bool>(std::string configKey, bool _def) const;

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
        Config.RaceAllowedNVN = DefeatConfig::StringSetVarPtr(
            new StringSetVar(defeatconfig, "RaceAllowedNVN"sv, PapyrusInterface::ObjectVariableConfig(false)));
        Config.RaceAllowedPvic = DefeatConfig::StringSetVarPtr(
            new StringSetVar(defeatconfig, "RaceAllowedPvic"sv, PapyrusInterface::ObjectVariableConfig(false)));
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

        if (_defeatManager->Forms.DefeatPlayerQTE == nullptr) {
            SKSE::log::error("LoadForms : Not found TESQuest 'DefeatPlayerQTE'");
        } else {
            auto defeatqtewidget =
                SexLabDefeat::Papyrus::GetScriptObject(_defeatManager->Forms.DefeatPlayerQTE, "defeatqtewidget");
            if (defeatqtewidget == nullptr) {
                SKSE::log::error("LoadForms : Not found attached Script 'defeatqtewidget'");
            } else {
                auto var = defeatqtewidget->GetVariable("_widgetRoot");
                if (var->IsString()) {
                    SKSE::log::error("LoadForms : Ndefeatqtewidget: {}", var->GetString());
                }
            }
        }
    }

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

        // TODO: Caching?
        auto val = _config.find(configKey);
        if (val == _config.end()) {
            return _def;
        }
        try {
            return std::get<T>(val->second);
        } catch (const std::bad_variant_access& ex) {
            SKSE::log::critical("getConfig - Wrong Type of configKey: {}", ex.what());
        }
        return _def;
    }
}
