#pragma once
#include "core/base.h"

namespace core {
    
    class BaseStat {
    public:
        [[nodiscard]] virtual double getBase(entt::entity ent) {
            return 0.0;
        };
        [[nodiscard]] virtual double getDefault() const {return 0.0;};
        // this function will be called by iterating through all stats when a new thing is created.
        void setDefault(entt::entity ent) {setBase(ent, getDefault());};
        [[nodiscard]] virtual std::optional<double> getMinimumBase() const {return std::nullopt;};
        [[nodiscard]] virtual std::optional<double> getMaximumBase() const {return std::nullopt;};
        [[nodiscard]] virtual std::optional<double> getMinimum() const {return std::nullopt;};
        [[nodiscard]] virtual std::optional<double> getMaximum() const {return std::nullopt;};
        // Set the value in storage.
        [[nodiscard]] double setBase(entt::entity ent, double value) {

            if(auto min = getMinimumBase()) {
                if(value < *min) value = *min;
            }

            if(auto max = getMaximumBase()) {
                if(value > *max) value = *max;
            }

            doSetBase(ent, value);
            setDirty(ent);
            return value;
        }
        // Increase/decrease the value in storage by a certain amount.
        [[nodiscard]] double modBase(entt::entity ent, double value) {
            auto newVal = getBase(ent) + value;
            return setBase(ent, newVal);
        }
        // This will be used for things like buffs and status effects that modify the current value used by game
        // calculations.
        [[nodiscard]] virtual double calcEffective(entt::entity ent, std::unordered_map<std::size_t, double>& cache) {
            auto value = getBase(ent);
            return value;
        };

        [[nodiscard]] virtual double clampEffective(entt::entity ent, std::unordered_map<std::size_t, double>& cache) {
            auto value = calcEffective(ent, cache);
            if(auto min = getMinimum()) {
                if(value < *min) value = *min;
            }

            if(auto max = getMaximum()) {
                if(value > *max) value = *max;
            }
            return value;
        }

        // Cache the value in the cache map. The cache will be cleared when the getStat() request on the Object finishes...
        [[nodiscard]] virtual double cacheEffective(entt::entity ent, std::unordered_map<std::size_t, double>& cache) {
            auto val = clampEffective(ent, cache);
            cache[getId()] = val;
            return val;
        }
        // Retrieve the value from the cache map. If it's not there, calculate it and cache it.
        // This allows for complex derived values that depend on other stats and may require multiple calculations to avoid
        // repeat calculations for the same stat.
        [[nodiscard]] virtual double getEffective(entt::entity ent, std::unordered_map<std::size_t, double>& cache) {
            auto it = cache.find(getId());
            if (it != cache.end()) {
                return it->second;
            }
            return cacheEffective(ent, cache);
        }
        // The id / index of the stat in the storage array.
        [[nodiscard]] virtual std::size_t getId() const = 0;
        // The name, for text searching.
        [[nodiscard]] virtual std::string getName() const = 0;
    protected:
        virtual void doSetBase(entt::entity ent, double val) = 0;
    };

}