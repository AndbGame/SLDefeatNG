#pragma once

#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

#include "RE/Skyrim.h"
#include "SKSE/SKSE.h"

//we are not making nasa sw, so stfu
#pragma warning( disable : 4100 ) 
#pragma warning( disable : 4244 )

    using namespace std::literals;

#define PAPYRUSFUNCHANDLE RE::StaticFunctionTag*

#include <spdlog/sinks/basic_file_sink.h>