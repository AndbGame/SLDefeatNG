#pragma once

#include <Defeat.h>

#include "DefeatManager.h"

namespace SexLabDefeat {
    void installHooks(DefeatManager* defeatManager);
    void installEventSink(DefeatManager* defeatManager);
    void installInputEventSink(DefeatManager* defeatManager);
}