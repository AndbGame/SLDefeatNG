#pragma once

namespace SexLabDefeat {
    
    using time_point = std::chrono::high_resolution_clock::time_point;
    using milliseconds = std::chrono::milliseconds;
    using clock = std::chrono::high_resolution_clock;

    static const std::chrono::high_resolution_clock::time_point emptyTime =
        std::chrono::high_resolution_clock::time_point::min();

    static inline bool randomChanse(float chanse, float min = 1, float max = 100) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> distr(min, max);

        return distr(gen) < chanse;
    }
        
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