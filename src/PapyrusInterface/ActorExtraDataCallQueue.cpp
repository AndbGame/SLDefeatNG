#include "ActorExtraDataCallQueue.h"

namespace SexLabDefeat {
    namespace PapyrusInterface {
        bool ActorExtraDataCallQueue::sendRequest(RE::Actor* actor) {
            if (actor != nullptr) {
                SexLabDefeat::Papyrus::CallbackPtr callback(
                    new EmptyRequestCallback("DeferredActorExtraDataInitializer"));

                SKSE::log::trace("DeferredActorExtraDataInitializer - <{:08X}:{}>", actor->GetFormID(),
                                 actor->GetName());

                if (SexLabDefeat::Papyrus::DispatchStaticCall("defeat_skse_api", "requestActorExtraData", callback,
                                                              std::move(actor))) {
                    return true;
                }
                SKSE::log::error("Failed to dispatch static call [defeat_skse_api::requestActorExtraData].");
            } else {
                SKSE::log::error("Failed to dispatch static call [defeat_skse_api::requestActorExtraData]. Actor nullptr");
            }
            return false;
        }
        bool ActorExtraDataCallQueue::receiveResponse(ActorExtraData val) {
            _defActor->setExtraData(val);
            return true;
        }
    }
}