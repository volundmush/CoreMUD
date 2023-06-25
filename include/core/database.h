#pragma once
#include "core/base.h"


namespace core {

    extern std::unique_ptr<SQLite::Database> db;

    extern std::vector<std::string> schema;

    template<size_t N>
    nlohmann::json bitsetToJson(const std::bitset<N>& bits) {
        nlohmann::json json = nlohmann::json::array();
        if(bits.none()) return json;
        for (size_t i = 0; i < N; ++i) {
            if (bits[i]) {
                json.push_back(i);
            }
        }
        return json;
    }

    template<size_t N>
    void jsonToBitset(const nlohmann::json& json, std::bitset<N>& bits) {
        bits.reset(); // Clear all bits
        for (auto& element : json) {
            bits.set(element);
        }
    }

    extern std::vector<std::function<void(entt::entity,bool, nlohmann::json& j)>> serializeFuncs;

    nlohmann::json serializeEntity(entt::entity ent, bool asPrototype = false);

    extern std::vector<std::function<void(entt::entity, const nlohmann::json&)>> deserializeFuncs;
    void deserializeEntity(entt::entity ent, const nlohmann::json& j);

    void processDirty();

    void readyDatabase();

    void loadDatabase();

    extern std::vector<std::function<void()>> preLoadFuncs, postLoadFuncs;
    void loadObjects();

    void savePrototype(const std::string& name, const nlohmann::json& j);

    std::optional<nlohmann::json> getPrototype(const std::string &name);

}