#pragma once

#include <Defeat.h>

#include "DefeatManager.h"

namespace SexLabDefeat {
    void installHooks(DefeatManager* defeatManager);
    void installEventSink(DefeatManager* defeatManager);
    void installInputEventSink(DefeatManager* defeatManager);
}

namespace Hooks {

    static uint8_t* DoDetect(RE::Actor* viewer, RE::Actor* target, int32_t& detectval, uint8_t& unk04,
                                    uint8_t& unk05, uint32_t& unk06, RE::NiPoint3& pos, float& unk08, float& unk09,
                                    float& unk10);

    static inline REL::Relocation<decltype(DoDetect)> _DoDetect;
}