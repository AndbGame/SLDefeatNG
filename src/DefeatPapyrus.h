#pragma once

#include <Defeat.h>

using time_point = std::chrono::high_resolution_clock::time_point;
using milliseconds = std::chrono::milliseconds;
using high_resolution_clock = std::chrono::high_resolution_clock;

namespace SexLabDefeat {
    namespace PapyrusInterface {

        class FunctionCallCallback {
        public:
            FunctionCallCallback(std::function<void()> callback, time_point expiration)
                : _callback(callback), _expiration(expiration) {};

            void execute() {
                if (high_resolution_clock::now() <= _expiration) {
                    _callback();
                }
            };

        protected:
            std::function<void()> _callback;
            time_point _expiration;
        };

        class FunctionCallResponse {};
       
        class ExpiringFunctionCallQueue : public SpinLock {
        public:

            ExpiringFunctionCallQueue(milliseconds expirationMs = 0ms,
                                      milliseconds accessProlongationMs = 0ms)
                : _expiration(expirationMs), _accessProlongation(accessProlongationMs) {};
            ~ExpiringFunctionCallQueue();

            virtual bool sendRequest() = 0;
            virtual bool receiveResponse(FunctionCallResponse val) = 0;

            void functionCall(std::function<void()> callback, milliseconds timeoutMs);
            void functionResponse(FunctionCallResponse val);

        protected:
            enum StatusType { FREE, BUSY } Status = StatusType::FREE;

            std::queue<std::shared_ptr<FunctionCallCallback>> _callbackQueue;

            milliseconds _expiration;
            milliseconds _accessProlongation;
            time_point _expirationTime = emptyTime;

            bool isExpired();
            void accessTouch();
            void processCallbackStack();
        };
    }
}