#pragma once
//#include <Windows.h>
#include <thread>

namespace SexLabDefeat {

    class SpinLock {

        //RE::BSSpinLock lock;

    public:
        enum {
            kFastSpinThreshold = 10000,
        };

        SpinLock() : _owningThread(0), _lockCount(0){};

        void spinLock(std::uint32_t a_pauseAttempts = 0) {
            //lock.Lock();
            std::uint32_t myThreadID = std::this_thread::get_id()._Get_underlying_id();

            _mm_lfence();
            if (_owningThread == myThreadID) {
                ::_InterlockedIncrement(&_lockCount);
            } else {
                std::uint32_t attempts = 0;
                std::uint32_t counter = 0;
                if (::_InterlockedCompareExchange(&_lockCount, 1, 0)) {
                    do {
                        ++attempts;
                        ++counter;
                        if (counter >= 100) {
                            SKSE::log::error("SpinLock {} attempts!", counter);
                            counter = 0;
                        }
                        _mm_pause();
                        if (attempts >= a_pauseAttempts) {
                            std::uint32_t spinCount = 0;
                            while (::_InterlockedCompareExchange(&_lockCount, 1, 0)) {
                                ++counter;
                                if (counter >= 100) {
                                    SKSE::log::error("SpinLock {} attempts!", counter);
                                    counter = 0;
                                }
                                //Sleep(++spinCount < kFastSpinThreshold ? 0 : 1);
                                std::this_thread::sleep_for(++spinCount < kFastSpinThreshold ? 0ms : 1ms);
                            }
                            break;
                        }

                    } while (::_InterlockedCompareExchange(&_lockCount, 1, 0));
                    _mm_lfence();
                }

                _owningThread = myThreadID;
                _mm_sfence();
            }
        }
        void spinUnlock() { 
            //lock.Unlock();
            std::uint32_t myThreadID = std::this_thread::get_id()._Get_underlying_id();

            _mm_lfence();
            if (_owningThread == myThreadID) {
                if (_lockCount == 1) {
                    _owningThread = 0;
                    _mm_mfence();
                    ::_InterlockedCompareExchange(&_lockCount, 0, 1);
                } else {
                    ::_InterlockedDecrement(&_lockCount);
                }
            }
        }

    private:
        // members
        volatile std::uint32_t _owningThread;  // 0
        volatile long _lockCount;     // 4
    };
    static_assert(sizeof(SpinLock) == 0x8);
}