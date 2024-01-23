#pragma once

#include <Defeat.h>

#include "ExpiringFunctionCallQueue.h"

namespace SexLabDefeat {
    namespace PapyrusInterface {

        class ActorExtraData {
        public:
            bool ignoreActorOnHit = true;
            int sexLabGender = 0;
            int sexLabSexuality = 0;
            bool sexLabAllowed = 0;
            std::string sexLabRaceKey = "";
            float DFWVulnerability = 0;
        };
    
        class ActorExtraDataCallQueue : ExpiringFunctionCallQueue<ActorExtraData, RE::Actor*> {
        public:
            // Inherited via ExpiringFunctionCallQueue
            bool sendRequest(RE::Actor* actor) override;
            bool receiveResponse(ActorExtraData val) override;

        };
    }
}