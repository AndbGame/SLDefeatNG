#pragma once

#include <Defeat.h>

#include "ExpiringFunctionCallQueue.h"

namespace SexLabDefeat {
    namespace PapyrusInterface {
                    
        class ActorExtraDataCallQueue : public ExpiringFunctionCallQueue<ActorExtraData, RE::Actor*> {
        public:
            ActorExtraDataCallQueue(milliseconds expirationMs = 0ms, milliseconds accessProlongationMs = 0ms)
                : ExpiringFunctionCallQueue<ActorExtraData, RE::Actor*>(expirationMs, accessProlongationMs) {}
            // Inherited via ExpiringFunctionCallQueue
            bool sendRequest(RE::Actor* actor) override;
            bool receiveResponse(ActorExtraData val) override;

        };
    }
}