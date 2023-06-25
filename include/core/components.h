#pragma once

#include "core/base.h"


namespace core {

    struct StringView {
        StringView() = default;
        explicit StringView(const std::string& txt) { setData(txt); };
        std::string_view data;
        std::string_view clean;
        void setData(const std::string& txt);
    };

    struct Name : StringView {
        using StringView::StringView;
    };
    struct ShortDescription : StringView {
        using StringView::StringView;
    };
    struct RoomDescription : StringView {
        using StringView::StringView;
    };
    struct LookDescription : StringView {
        using StringView::StringView;
    };

    struct Entity {
        entt::entity data{entt::null};
    };

    struct ReverseEntity {
        std::vector<entt::entity> data{};
    };

    struct Location : Entity {
        int8_t locationType{0};
        double x{0}, y{0}, z{0};
    };
    struct Contents : ReverseEntity {};
    struct Parent : Entity {};
    struct Children : ReverseEntity {};
    struct Owner : Entity {};
    struct Assets : ReverseEntity {};

    struct SessionHolder {
        std::shared_ptr<Session> data;
        uint8_t sessionMode{0};
    };

    using DestinationType = std::variant<RoomId, GridPoint, SectorPoint>;
    struct Destination {
        Destination() = default;
        explicit Destination(const nlohmann::json& j);
        // if ent is entt::null, then it's assumed to be a destination on the same Object.
        entt::entity ent{entt::null};
        DestinationType destination;
        nlohmann::json serialize();
    };

    // An Area is a collection of Rooms indexed by their RoomId, used for very legacy
    // style MUD designs where rooms are linked by exits.
    // We don't actually define Exits as a component type here given how each MUD
    // may do those differently, but they are likely a Component attached to the entity
    // in the unordered_map of Area.
    struct Area {
        std::unordered_map<RoomId, entt::entity> data{};
    };

    // Meant to be used by the entt::entity within the Area component map.
    struct Room {
        ObjectId obj;
        RoomId id;
    };

    struct RoomLocation {
        RoomId id;
    };

    struct RoomContents {
        std::vector<entt::entity> data{};
    };

    // Used for Maps and Expanses to limit the size of their coordinates.
    struct AbstractGrid {
        GridLength minX = std::numeric_limits<GridLength>::min();
        GridLength maxX = std::numeric_limits<GridLength>::max();
        GridLength minY = std::numeric_limits<GridLength>::min();
        GridLength maxY = std::numeric_limits<GridLength>::max();
        GridLength minZ = std::numeric_limits<GridLength>::min();
        GridLength maxZ = std::numeric_limits<GridLength>::max();
    };

    // An Expanse is a kind of Grid, so it will use GridContents and GridLocation.
    // An Expanse is a kind of Grid where all points within the min/max coordinates
    // are valid locations for someone to be in. This allows for the illusion of a vast
    // area that's mostly empty but does have some cool things here and there in it.
    struct Expanse : AbstractGrid {
        std::unordered_map<GridPoint, entt::entity> poi{};
    };

    // A Map is a kind of Grid, so it will use GridContents and GridLocation.
    // It is the opposite of an Expanse; only points which are in poi are valid
    // locations. This system doesn't dictate how the points of interest are connected.
    // That's up to the MUD to decide. A common one might be to simply use compass point
    // directions such that if here.x +1 exists, one can go north.
    struct Map : AbstractGrid {
        std::unordered_map<GridPoint, entt::entity> poi{};
    };

    struct GridLocation {
        GridLocation() = default;
        explicit GridLocation(const GridPoint& d) : data(d) {};
        explicit GridLocation(const nlohmann::json& j) : data(j) {};
        GridPoint data;
    };

    struct GridContents {
        std::unordered_map<GridPoint, std::vector<entt::entity>> data{};
    };

    // Used for Space.
    struct AbstractSector {
        SectorLength minX = std::numeric_limits<SectorLength>::min();
        SectorLength maxX = std::numeric_limits<SectorLength>::max();
        SectorLength minY = std::numeric_limits<SectorLength>::min();
        SectorLength maxY = std::numeric_limits<SectorLength>::max();
        SectorLength minZ = std::numeric_limits<SectorLength>::min();
        SectorLength maxZ = std::numeric_limits<SectorLength>::max();
    };

    // Space is like an Expanse, but it uses floating point (double) logic for its
    // coordinates. This allows for more complex positioning of things, but is harder
    // to administrate in a text-based game. The usefulness of this may be limited,
    // but it's provided because some games love 3D spaceship games moving around
    // complex coordinate planes even if that's hard to represent in text.
    struct Space : AbstractSector {
        std::unordered_map<SectorPoint, entt::entity> poi{};
    };

    struct SectorContents {
        std::unordered_map<SectorPoint, std::vector<entt::entity>> data{};
    };

    struct SectorLocation {
        SectorLocation() = default;
        explicit SectorLocation(const SectorPoint& d) : data(d) {};
        explicit SectorLocation(const nlohmann::json& j) : data(j) {};
        SectorPoint data;
    };

    struct Item {

    };
    struct Character {

    };
    struct NPC {

    };

    struct Player {
        int64_t accountId{-1};
    };

    struct Prototype {
        std::string_view data;
    };

    struct Vehicle {};

}