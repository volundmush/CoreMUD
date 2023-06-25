#pragma once

// sane C++ standard imports.
#include <cstdint>
#include <optional>
#include <chrono>
#include <string>
#include <string_view>
#include <vector>
#include <list>
#include <memory>
#include <map>
#include <set>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <utility>
#include <random>
#include <unordered_set>
#include <bitset>
#include <variant>
#include <limits>

// Our own libraries...
#include <boost/asio.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include "fmt/format.h"
#include "SQLiteCpp/SQLiteCpp.h"
#include "entt/entt.hpp"

namespace core {
    using namespace std::chrono_literals;
    using namespace boost::asio::experimental::awaitable_operators;

    class Connection;
    class Link;
    class LinkManager;
    class Session;

    extern std::unique_ptr<boost::asio::io_context> executor;
    extern std::shared_ptr<spdlog::logger> logger;

    template<typename T>
    using mpmc_channel = boost::asio::experimental::concurrent_channel<void(boost::system::error_code, T)>;

    using JsonChannel = mpmc_channel<nlohmann::json>;

    template<typename T>
    using async = boost::asio::awaitable<T>;

    template<typename T = bool>
    using OpResult = std::pair<T, std::optional<std::string>>;

    enum class ColorType : uint8_t {
        NoColor = 0,
        Standard = 1,
        Xterm256 = 2,
        TrueColor = 3
    };

    extern entt::registry registry;

    using GridLength = int64_t;

    struct GridPoint {
        GridPoint() = default;
        GridPoint(GridLength x, GridLength y, GridLength z) : x(x), y(y), z(z) {};
        explicit GridPoint(const nlohmann::json& j);
        GridLength x, y, z;
        bool operator==(const GridPoint& other) const {
            return x == other.x && y == other.y && z == other.z;
        }
        nlohmann::json serialize() const;
    };

    using SectorLength = double;
    struct SectorPoint {
        SectorPoint() = default;
        SectorPoint(SectorLength x, SectorLength y, SectorLength z) : x(x), y(y), z(z) {};
        explicit SectorPoint(const nlohmann::json& j);
        SectorLength x, y, z;
        bool operator==(const SectorPoint& other) const {
            return x == other.x && y == other.y && z == other.z;
        }
        nlohmann::json serialize() const;
    };

    struct ObjectId {
        ObjectId() = default;
        ObjectId(std::size_t index, int64_t generation) : index(index), generation(generation) {};

        explicit ObjectId(const nlohmann::json &json) : index(json[0]), generation(json[1]) {};
        std::size_t index;
        int64_t generation;

        [[nodiscard]] std::string toString() const;

        [[nodiscard]] entt::entity getObject() const;

        bool operator==(const ObjectId &other) const {
            return index == other.index && generation == other.generation;
        }
    };

    extern boost::regex obj_regex;
    bool isObjRef(const std::string& str);
    bool isObjId(const std::string& str);
    entt::entity parseObjectId(const std::string& str);



}

namespace std {
    template <>
    struct hash<core::ObjectId> {
        std::size_t operator()(const core::ObjectId& id) const {
            return std::hash<std::size_t>()(id.index) ^ std::hash<int64_t>()(id.generation);
        }
    };

    template<>
    struct hash<core::GridPoint> {
        size_t operator()(const core::GridPoint& pt) const {
            return std::hash<core::GridLength>()(pt.x) ^ std::hash<core::GridLength>()(pt.y) ^ std::hash<core::GridLength>()(pt.z);
        }
    };

    template<>
    struct hash<core::SectorPoint> {
        size_t operator()(const core::SectorPoint& pt) const {
            return std::hash<core::SectorLength>()(pt.x) ^ std::hash<core::SectorLength>()(pt.y) ^ std::hash<core::SectorLength>()(pt.z);
        }
    };

}

namespace core {
// Any object which needs to be updated in the database - it was created, modified, or deleted,
    // its ObjectId must be in the dirty set by the time the syncer runs.
    extern std::unordered_set<ObjectId> dirty;

    // If this is set, many operations which would set the dirty flag will not.
    // Other safeguards of the Object API may also be released. Do remember to
    // set it to false after the game is done loading.
    extern bool gameIsLoading;

    void setDirty(entt::entity, bool override = false);
    void setDirty(const ObjectId& id, bool override = false);

    entt::entity getObject(std::size_t index, int64_t generation);
    entt::entity getObject(std::size_t index);

    using RoomId = std::size_t;

    // The objects vector serves as a generational arena for all objects in the game.
    // The first element of the pair is the generation (as a unix timestamp in seconds),
    // the second is the object itself.
    // A grave is an object with a generation of 0 and an empty pointer.
    // We COULD get away with just using a vector of shared_ptrs, but this way we can
    // use the ObjectId for serialization and not have to worry about the object being
    // deleted and the ID being reused.
    extern std::vector<std::pair<int64_t, entt::entity>> objects;

    std::size_t getFreeObjectId();

    int64_t getUnixTimestamp();

    ObjectId createObjectId();

    entt::entity createObject();

    extern std::unordered_set<std::string> stringPool;

    std::string_view intern(const std::string& str);
    std::string_view intern(std::string_view str);

    extern std::random_device randomDevice;
    extern std::default_random_engine randomEngine;

    // For backwards compatability with the old DBAT code, we have this map which effectively replicates
    // the old 'world' variable. It is filled with the rooms from Objects that are marked GLOBALROOM.
    // Beware of ID collisions when setting objects GLOBALROOM.
    extern std::unordered_map<RoomId, entt::entity> legacyRooms;
    extern std::unordered_map<RoomId, GridPoint> legacySpaceRooms;

    template <typename Iterator, typename Key = std::function<std::string(typename std::iterator_traits<Iterator>::value_type)>>
    Iterator partialMatch(
            const std::string& match_text,
            Iterator begin, Iterator end,
            bool exact = false,
            Key key = [](const auto& val){ return std::to_string(val); }
    )
    {
        // Use a multimap to automatically sort by the transformed key.
        using ValueType = typename std::iterator_traits<Iterator>::value_type;
        std::multimap<std::string, ValueType> sorted_map;
        std::for_each(begin, end, [&](const auto& val) {
            sorted_map.insert({key(val), val});
        });

        for (const auto& pair : sorted_map)
        {
            if (boost::iequals(pair.first, match_text))
            {
                return std::find(begin, end, pair.second);
            }
            else if (!exact && boost::istarts_with(pair.first, match_text))
            {
                return std::find(begin, end, pair.second);
            }
        }
        return end;
    }

    OpResult<> hashPassword(std::string_view password);

    OpResult<> checkPassword(std::string_view hash, std::string_view check);


}

namespace nlohmann {
    void to_json(json& j, const core::ObjectId& o);

    void from_json(const json& j, core::ObjectId& o);
}