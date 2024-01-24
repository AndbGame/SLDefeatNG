#pragma once

namespace SexLabDefeat {
    
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