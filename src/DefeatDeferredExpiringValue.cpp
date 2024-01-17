#include "Defeat.h"

namespace SexLabDefeat {

    template class DeferredExpiringValue<ActorExtraData>;

    template <class T>
    DeferredExpiringValue<T>::DeferredExpiringValue(std::unique_ptr<DeferredExpiringValueInitializer> initializer,
                                                    int expirationMs, int accessProlongationExpireMs) {
        _minTime = std::chrono::high_resolution_clock::time_point::min();
        _expirationTime = _minTime;
        _expiration = std::chrono::milliseconds(expirationMs);
        _accessProlongationExpireMs = std::chrono::milliseconds(accessProlongationExpireMs);
        _initializer = std::move(initializer);
    };

    template <class T>
    DeferredExpiringValue<T>::~DeferredExpiringValue() {
        spinLock();
        while (!_callbackQueue.empty()) _callbackQueue.pop();
    };

    template <class T>
    void DeferredExpiringValue<T>::getCallback(std::function<void()> callback) {
        if (!isActualValue()) {
            spinLock();
            std::shared_ptr<DeferredExpiringValueCallback> _callback =
                std::make_shared<DeferredExpiringValueCallback>(callback);
            _callbackQueue.push(_callback);
            spinUnlock();
            _initializer.get()->spinLock();
            if (_initializer.get()->Status == DeferredExpiringValueInitializer::StatusType::FREE) {
                _initializer.get()->spinUnlock();
                _initializer.get()->initialize();
            } else {
                _initializer.get()->spinUnlock();
            }
        } else {
            accessTouch();
            processCallbackStack();
            callback();
        }
    };

    template <class T>
    bool DeferredExpiringValue<T>::isActualValue() {
        spinLock();
        if (std::chrono::high_resolution_clock::now() > _expirationTime) {
            spinUnlock();
            return false;
        }
        spinUnlock();
        return true;
    };

    template <class T>
    void DeferredExpiringValue<T>::initializeValue(T val) {
        spinLock();
        _value = val;
        _expirationTime = std::chrono::high_resolution_clock::now() + _expiration;
        _initializer.get()->setFree();
        spinUnlock();
        processCallbackStack();
    };

    template <class T>
    void DeferredExpiringValue<T>::accessTouch() {
        spinLock();
        if (std::chrono::high_resolution_clock::now() > (_expirationTime - _accessProlongationExpireMs)) {
            _expirationTime += _accessProlongationExpireMs;
        }
        spinUnlock();
    };

    template <class T>
    void DeferredExpiringValue<T>::processCallbackStack() {
        spinLock();
        DeferredExpiringValueCallback* clb = nullptr;
        if (!_callbackQueue.empty()) {
            SKSE::log::trace("processCallbackStack start processing {} delayed callbacks", _callbackQueue.size());
            clb = _callbackQueue.front().get();
        }
        spinUnlock();

        while (clb != nullptr) {
            clb->execute();
            clb = nullptr;
            spinLock();
            _callbackQueue.pop();
            if (!_callbackQueue.empty()) {
                clb = _callbackQueue.front().get();
            }
            spinUnlock();
        }
    };
}