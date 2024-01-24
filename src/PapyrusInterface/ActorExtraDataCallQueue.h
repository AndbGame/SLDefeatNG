#pragma once

#include <Defeat.h>

#include "ExpiringFunctionCallQueue.h"
#include "DefeatPapyrus.h"

namespace SexLabDefeat {
    namespace PapyrusInterface {
                    
        class ActorExtraDataCallQueue : public ExpiringFunctionCallQueue<ActorExtraData, RE::Actor*> {
        public:
            ActorExtraDataCallQueue(IDefeatActor* defActor, milliseconds expirationMs = 0ms, milliseconds accessProlongationMs = 0ms)
                : ExpiringFunctionCallQueue<ActorExtraData, RE::Actor*>(expirationMs, accessProlongationMs),
                  _defActor(defActor) {}
            // Inherited via ExpiringFunctionCallQueue
            bool sendRequest(RE::Actor* actor) override;
            bool receiveResponse(ActorExtraData val) override;

        protected:
            IDefeatActor* _defActor;
        };
    }
}