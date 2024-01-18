#include "Defeat.h"

namespace SexLabDefeat {

    template class DeferredExpiringValue<ActorExtraData>;
    template class DeferredExpiringValue<int>;
    template class DeferredExpiringValue<float>;
    template class DeferredExpiringValue<bool>;
    template class DeferredExpiringValue<std::string>;

    template <class T>
    DeferredExpiringValue<T>::DeferredExpiringValue(int expirationMs, int accessProlongationExpireMs) {
        _minTime = std::chrono::high_resolution_clock::time_point::min();
        _expirationTime = _minTime;
        _expiration = std::chrono::milliseconds(expirationMs);
        _accessProlongationExpireMs = std::chrono::milliseconds(accessProlongationExpireMs);
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
}