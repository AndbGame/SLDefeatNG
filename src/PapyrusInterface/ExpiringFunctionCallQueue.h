#pragma once

#include <Defeat.h>

namespace SexLabDefeat {
    namespace PapyrusInterface {

        class FunctionCallCallback {
        public:
            FunctionCallCallback(std::function<void()> callback, time_point expiration)
                : _callback(callback), _expiration(expiration){};

            void execute() {
                if (clock::now() <= _expiration) {
                    _callback();
                }
            };

        protected:
            std::function<void()> _callback;
            time_point _expiration;
        };

        template <class T, class U>
        class ExpiringFunctionCallQueue : public SpinLock {
        public:
            ExpiringFunctionCallQueue(milliseconds expirationMs = 0ms, milliseconds accessProlongationMs = 0ms)
                : _expiration(expirationMs), _accessProlongation(accessProlongationMs){};
            ~ExpiringFunctionCallQueue() {
                SexLabDefeat::UniqueSpinLock locked(*this);
                while (!_callbackQueue.empty()) _callbackQueue.pop();
            }

            virtual bool sendRequest(U param) = 0;
            virtual bool receiveResponse(T val) = 0;

            void functionCall(U param, std::function<void()> callback, milliseconds timeoutMs) {
                SexLabDefeat::UniqueSpinLock locked(*this);
                if (isExpired()) {
                    std::shared_ptr<FunctionCallCallback> _callback =
                        std::make_shared<FunctionCallCallback>(callback, clock::now() + timeoutMs);
                    if (_callbackQueue.size() > 100) {
                        SKSE::log::critical("ExpiringFunctionCallQueue: Unexpected size of _callbackQueue: '{}'. Check papyrus log",
                                            _callbackQueue.size());
                    } else {
                        _callbackQueue.push(_callback);
                    }
                    if (Status == StatusType::FREE && sendRequest(param)) {
                        Status = StatusType::BUSY;
                    }
                } else {
                    accessProlongation();
                    processCallbackStack();
                    callback();
                }
            }
            void functionResponse(T val) {
                SexLabDefeat::UniqueSpinLock locked(*this);
                _expirationTime = std::chrono::high_resolution_clock::now() + _expiration;
                if (receiveResponse(val)) {
                    Status = StatusType::FREE;
                } else {
                    SKSE::log::critical("ExpiringFunctionCallQueue: Receive response failed");
                }
            }

        protected:
            enum StatusType { FREE, BUSY } Status = StatusType::FREE;

            std::queue<std::shared_ptr<FunctionCallCallback>> _callbackQueue;

            milliseconds _expiration;
            milliseconds _accessProlongation;
            time_point _expirationTime = emptyTime;

            bool isExpired() {
                SexLabDefeat::UniqueSpinLock locked(*this);
                if (std::chrono::high_resolution_clock::now() > _expirationTime) {
                    return true;
                }
                return false;
            }
            void accessProlongation() {
                SexLabDefeat::UniqueSpinLock locked(*this);
                if (_expirationTime > emptyTime &&
                    std::chrono::high_resolution_clock::now() > (_expirationTime - _accessProlongation)) {
                    _expirationTime += _accessProlongation;
                }
            }
            void processCallbackStack() {
                std::queue<std::shared_ptr<FunctionCallCallback>> callbackToProcess;

                {
                    SexLabDefeat::UniqueSpinLock locked(*this);
                    if (!_callbackQueue.empty()) {
                        SKSE::log::trace("processCallbackStack start processing {} delayed callbacks",
                                         _callbackQueue.size());
                        while (!_callbackQueue.empty()) {
                            callbackToProcess.push(_callbackQueue.front());
                            _callbackQueue.pop();
                        }
                    }
                }

                while (!callbackToProcess.empty()) {
                    std::shared_ptr<FunctionCallCallback> clb = callbackToProcess.front();
                    clb->execute();
                    callbackToProcess.pop();
                }
            }
        };
    }
}