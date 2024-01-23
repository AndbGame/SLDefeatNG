#include "ExpiringFunctionCallQueue.h"

namespace SexLabDefeat {
    namespace PapyrusInterface {

        template <class T, class U>
        ExpiringFunctionCallQueue<T,U>::~ExpiringFunctionCallQueue() {
            SexLabDefeat::UniqueSpinLock locked(*this);
            while (!_callbackQueue.empty()) _callbackQueue.pop();
        }

        template <class T, class U>
        void ExpiringFunctionCallQueue<T, U>::functionCall(U param, std::function<void()> callback,
                                                        milliseconds timeoutMs) {
            SexLabDefeat::UniqueSpinLock locked(*this);
            if (isExpired()) {
                std::shared_ptr<FunctionCallCallback> _callback =
                    std::make_shared<FunctionCallCallback>(callback, clock::now() + timeoutMs);
                if (_callbackQueue.size() > 100) {
                    SKSE::log::critical("Unexpected size of _callbackQueue: '{}'. Check papyrus log",
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
        template <class T, class U>
        void ExpiringFunctionCallQueue<T, U>::functionResponse(T val) {
            SexLabDefeat::UniqueSpinLock locked(*this);
            _expirationTime = std::chrono::high_resolution_clock::now() + _expiration;
            if (receiveResponse(val)) {
                Status = StatusType::FREE;
            }
        }
        template <class T, class U>
        bool ExpiringFunctionCallQueue<T, U>::isExpired() {
            SexLabDefeat::UniqueSpinLock locked(*this);
            if (std::chrono::high_resolution_clock::now() > _expirationTime) {
                return true;
            }
            return false;
        }
        template <class T, class U>
        void ExpiringFunctionCallQueue<T, U>::accessProlongation() {
            SexLabDefeat::UniqueSpinLock locked(*this);
            if (_expirationTime > emptyTime &&
                std::chrono::high_resolution_clock::now() > (_expirationTime - _accessProlongation)) {
                _expirationTime += _accessProlongation;
            }
        }
        template <class T, class U>
        void ExpiringFunctionCallQueue<T, U>::processCallbackStack() {
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
                std::shared_ptr<FunctionCallCallback> clb = _callbackQueue.front();
                clb->execute();
                _callbackQueue.pop();
            }
        }

    }
}