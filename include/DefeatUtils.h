#pragma once

namespace SexLabDefeat {

    static const std::chrono::high_resolution_clock::time_point emptyTime =
        std::chrono::high_resolution_clock::time_point::min();

    /* class DeferredExpiringValueInitializer : public SpinLock {
    public:
        enum StatusType { FREE, BUSY } Status = StatusType::FREE;

        DeferredExpiringValueInitializer(std::function<void()> callback) { _callback = callback; };
        ~DeferredExpiringValueInitializer() = default;

        void initialize() {
            spinLock();
            Status = StatusType::BUSY;
            spinUnlock();
            _callback();
        };

        void setFree() {
            spinLock();
            Status = StatusType::FREE;
            spinUnlock();
        }

    protected:
        std::function<void()> _callback;
    };*/
        
    class WornVisitor : public RE::InventoryChanges::IItemChangeVisitor {
    public:
        WornVisitor(std::function<RE::BSContainer::ForEachResult(RE::InventoryEntryData*)> a_fun) : _fun(a_fun){};
        ~WornVisitor() = default;

        virtual RE::BSContainer::ForEachResult Visit(RE::InventoryEntryData* a_entryData) override {
            return _fun(a_entryData);
        }

    private:
        std::function<RE::BSContainer::ForEachResult(RE::InventoryEntryData*)> _fun;
    };

}