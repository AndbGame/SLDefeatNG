#include <Defeat.h>

#include "BuildVersion.h"
#include "src/DefeatManager.h"
#include "src/DefeatHooks.h"

int SexLabDefeat::format_as(SexLabDefeat::DefeatActorStates f) { return fmt::underlying(f); }
int SexLabDefeat::format_as(SexLabDefeat::HitResult f) { return fmt::underlying(f); }

namespace {
    SexLabDefeat::DefeatConfig* defeatConfig;
    SexLabDefeat::DefeatManager* defeatManager;

    void static SetupLog() {
        auto logsFolder = SKSE::log::log_directory();
        if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
        auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
        auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
        auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
        auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
        spdlog::set_default_logger(std::move(loggerPtr));
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%L] [%t] [%s:%#] %v");
        spdlog::set_level(spdlog::level::trace);
        spdlog::flush_on(spdlog::level::trace);
    }

    void ConfigureLog() {
        if (defeatConfig->CFG_LOGGING == 0) {
            spdlog::set_level(spdlog::level::err);
        } else if (defeatConfig->CFG_LOGGING == 1) {
            spdlog::set_level(spdlog::level::info);
        } else {
            spdlog::set_level(spdlog::level::trace);
        }
    }

    void OnSKSEMessageReceived(SKSE::MessagingInterface::Message* a_msg) {
        if (a_msg != nullptr) {
            switch (a_msg->type) {
                case SKSE::MessagingInterface::kInputLoaded:
                    SKSE::log::trace("kInputLoaded");
                    SexLabDefeat::installInputEventSink(defeatManager);
                    break;
                case SKSE::MessagingInterface::kDataLoaded:
                    SKSE::log::trace("kDataLoaded");
                    defeatConfig->Setup(defeatManager);
                    break;
                case SKSE::MessagingInterface::kPostLoad:
                    SKSE::log::trace("kPostLoad");
                    break;
                case SKSE::MessagingInterface::kPreLoadGame:  // set reload flag, so we can prevent in papyrus calls of
                                                              // native function untill view get reset by invoking
                                                              // _reset
                    SKSE::log::trace("kPreLoadGame");
                    defeatManager->setGameState(SexLabDefeat::DefeatManager::GameState::PRE_LOAD);
                    break;
                case SKSE::MessagingInterface::kPostLoadGame:  // for loading existing game
                    [[fallthrough]];
                case SKSE::MessagingInterface::kNewGame:  // for new game
                    SKSE::log::trace("kPostLoadGame | kNewGame");
                    defeatManager->reset();
                    break;
            }
        }
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SKSE::Init(skse);
            
    SetupLog();
     
    defeatConfig = new SexLabDefeat::DefeatConfig();
    defeatConfig->readIniConfig();

    ConfigureLog();
    SKSE::log::info("Build {}", BUILD_VERSION);

    defeatManager = new SexLabDefeat::DefeatManager(defeatConfig);
    defeatManager->load();

    if (!SKSE::GetMessagingInterface()->RegisterListener(OnSKSEMessageReceived)) {
        SKSE::stl::report_and_fail("Unable to register message listener.");
    }

    SexLabDefeat::installHooks(defeatManager);
    SexLabDefeat::installEventSink(defeatManager);

    return true;
}