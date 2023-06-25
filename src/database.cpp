#include "core/database.h"
#include "core/components.h"
#include "core/api.h"
#include "core/config.h"
#include "core/link.h"

namespace core {

    std::vector<std::function<void(entt::entity,bool,nlohmann::json&)>> serializeFuncs;
    nlohmann::json serializeEntity(entt::entity ent, bool asPrototype) {
        nlohmann::json j;

        auto name = registry.try_get<Name>(ent);
        if (name) {
            j["Name"] = name->data;
        }

        auto shortDescription = registry.try_get<ShortDescription>(ent);
        if (shortDescription) {
            j["ShortDescription"] = shortDescription->data;
        }

        auto roomDescription = registry.try_get<RoomDescription>(ent);
        if (roomDescription) {
            j["RoomDescription"] = roomDescription->data;
        }

        auto lookDescription = registry.try_get<LookDescription>(ent);
        if(lookDescription) {
            j["LookDescription"] = lookDescription->data;
        }

        if(!asPrototype) {
            // Nab relationships...
            auto location = registry.try_get<Location>(ent);
            if (location) {
                j["Location"] = registry.get<ObjectId>(location->data);
            }

            auto parent = registry.try_get<Parent>(ent);
            if (parent) {
                j["Parent"] = registry.get<ObjectId>(parent->data);
            }

            auto owner = registry.try_get<Owner>(ent);
            if (owner) {
                j["Owner"] = registry.get<ObjectId>(owner->data);
            }
        }




        auto area = registry.try_get<Area>(ent);
        if(area) {
            nlohmann::json rooms;
            for(auto &[rid, room] : area->data) {
                rooms.push_back(std::make_pair(rid, serializeEntity(room, asPrototype)));
            }
            j["Area"] = rooms;
        }

        auto expanse = registry.try_get<Expanse>(ent);
        if(expanse) {
            nlohmann::json e;
            e["minX"] = expanse->minX;
            e["minY"] = expanse->minY;
            e["minZ"] = expanse->minZ;
            e["maxX"] = expanse->maxX;
            e["maxY"] = expanse->maxY;
            e["maxZ"] = expanse->maxZ;

            for(auto &[coor, poi] : expanse->poi) {
                nlohmann::json p;
                p.push_back(coor.serialize());
                p.push_back(serializeEntity(poi, asPrototype));
                e["poi"].push_back(p);
            }
            j["Expanse"] = e;
        }

        auto map = registry.try_get<Map>(ent);
        if(map) {
            nlohmann::json e;
            e["minX"] = map->minX;
            e["minY"] = map->minY;
            e["minZ"] = map->minZ;
            e["maxX"] = map->maxX;
            e["maxY"] = map->maxY;
            e["maxZ"] = map->maxZ;

            for(auto &[coor, poi] : map->poi) {
                nlohmann::json p;
                p.push_back(coor.serialize());
                p.push_back(serializeEntity(poi, asPrototype));
                e["poi"].push_back(p);
            }
            j["Map"] = e;
        }

        auto space = registry.try_get<Space>(ent);
        if(space) {
            nlohmann::json e;
            e["minX"] = space->minX;
            e["minY"] = space->minY;
            e["minZ"] = space->minZ;
            e["maxX"] = space->maxX;
            e["maxY"] = space->maxY;
            e["maxZ"] = space->maxZ;

            for(auto &[coor, poi] : space->poi) {
                nlohmann::json p;
                p.push_back(coor.serialize());
                p.push_back(serializeEntity(poi, asPrototype));
                e["poi"].push_back(p);
            }
            j["Space"] = e;
        }

        auto gloc = registry.try_get<GridLocation>(ent);
        if(gloc) {
            j["GridLocation"] = gloc->data.serialize();
        }

        auto rloc = registry.try_get<RoomLocation>(ent);
        if(rloc) {
            j["RoomLocation"] = rloc->id;
        }

        auto player = registry.try_get<Player>(ent);
        if(player) {
            j["Player"]["accountId"] = player->accountId;
        }

        auto room = registry.try_get<Room>(ent);
        if(room) {
            nlohmann::json r;
            r["id"] = room->id;
            r["obj"] = room->obj;
            j["Room"] = r;
        }

        if(registry.any_of<Character>(ent)) j["Character"] = true;
        if(registry.any_of<NPC>(ent)) j["NPC"] = true;
        if(registry.any_of<Item>(ent)) j["Item"] = true;

        if(registry.any_of<Vehicle>(ent)) j["Vehicle"] = true;

        for(auto& func : serializeFuncs) func(ent, false, j);

        return j;
    }

    std::vector<std::function<void(entt::entity, const nlohmann::json&)>> deserializeFuncs;
    void deserializeEntity(entt::entity ent, const nlohmann::json& j) {
        if(j.contains("Name")) {
            registry.emplace<Name>(ent, j["Name"]);
        }
        if(j.contains("ShortDescription")) {
            registry.emplace<ShortDescription>(ent, j["ShortDescription"]);
        }
        if(j.contains("RoomDescription")) {
            registry.emplace<RoomDescription>(ent, j["RoomDescription"]);
        }
        if(j.contains("LookDescription")) {
            registry.emplace<LookDescription>(ent, j["LookDescription"]);
        }

        if(j.contains("Location")) {
            ObjectId loc(j["Location"]);
            setLocation(ent, loc.getObject());
        }

        if(j.contains("Parent")) {
            ObjectId parent(j["Parent"]);
            setParent(ent, parent.getObject());
        }

        if(j.contains("Owner")) {
            ObjectId owner(j["Owner"]);
            setOwner(ent, owner.getObject());
        }

        if(j.contains("Area")) {
            auto &rooms = registry.get_or_emplace<Area>(ent);
            auto &o = registry.get<ObjectId>(ent);
            for(auto &exdata : j["Area"]) {
                auto r = exdata[0].get<RoomId>();
                auto room = registry.create();
                rooms.data.emplace(r, room);
                deserializeEntity(room, exdata[1]);
                auto &rm = registry.get_or_emplace<Room>(room);
                rm.obj = o;
                rm.id = r;
            }
        }

        if(j.contains("Expanse")) {
            auto &exp = registry.get_or_emplace<Expanse>(ent);
            auto &data = j["Expanse"];
            if(data.contains("minX")) exp.minX = data["minX"];
            if(data.contains("minY")) exp.minY = data["minY"];
            if(data.contains("minZ")) exp.minZ = data["minZ"];
            if(data.contains("maxX")) exp.maxX = data["maxX"];
            if(data.contains("maxY")) exp.maxY = data["maxY"];
            if(data.contains("maxZ")) exp.maxZ = data["maxZ"];
            if(data.contains("poi")) {
                for(auto &poi : data["poi"]) {
                    GridPoint gp(poi[0]);
                    auto p = registry.create();
                    exp.poi.emplace(gp, p);
                    deserializeEntity(p, poi[1]);
                }
            }
        }

        if(j.contains("Map")) {
            auto &exp = registry.get_or_emplace<Map>(ent);
            auto &data = j["Map"];
            if(data.contains("minX")) exp.minX = data["minX"];
            if(data.contains("minY")) exp.minY = data["minY"];
            if(data.contains("minZ")) exp.minZ = data["minZ"];
            if(data.contains("maxX")) exp.maxX = data["maxX"];
            if(data.contains("maxY")) exp.maxY = data["maxY"];
            if(data.contains("maxZ")) exp.maxZ = data["maxZ"];
            if(data.contains("poi")) {
                for(auto &poi : data["poi"]) {
                    GridPoint gp(poi[0]);
                    auto p = registry.create();
                    exp.poi.emplace(gp, p);
                    deserializeEntity(p, poi[1]);
                }
            }
        }

        if(j.contains("Space")) {
            auto &exp = registry.get_or_emplace<Space>(ent);
            auto &data = j["Space"];
            if(data.contains("minX")) exp.minX = data["minX"];
            if(data.contains("minY")) exp.minY = data["minY"];
            if(data.contains("minZ")) exp.minZ = data["minZ"];
            if(data.contains("maxX")) exp.maxX = data["maxX"];
            if(data.contains("maxY")) exp.maxY = data["maxY"];
            if(data.contains("maxZ")) exp.maxZ = data["maxZ"];
            if(data.contains("poi")) {
                for(auto &poi : data["poi"]) {
                    SectorPoint gp(poi[0]);
                    auto p = registry.create();
                    exp.poi.emplace(gp, p);
                    deserializeEntity(p, poi[1]);
                }
            }
        }

        if(j.contains("GridLocation")) {
            auto &gloc = registry.get_or_emplace<GridLocation>(ent, j["GridLocation"]);
        }

        if(j.contains("RoomLocation")) {
            auto &rloc = registry.get_or_emplace<RoomLocation>(ent);
            rloc.id = j["RoomLocation"];
            // TODO: Place ent in proper Room
        }

        if(j.contains("Item")) {
            registry.get_or_emplace<Item>(ent);
        }

        if(j.contains("Character")) {
            registry.get_or_emplace<Character>(ent);
        }

        if(j.contains("NPC")) {
            registry.get_or_emplace<NPC>(ent);
        }

        if(j.contains("Player")) {
            auto &pdata = j["Player"];
            auto &player = registry.get_or_emplace<Player>(ent);
            player.accountId = pdata["accountId"];
        }

        if(j.contains("Room")) {
            auto &rdata = j["Room"];
            auto &room = registry.get_or_emplace<Room>(ent);
            room.id = rdata["id"];
            room.obj = ObjectId(rdata["obj"]);
        }

        if(j.contains("Vehicle")) {
            registry.get_or_emplace<Vehicle>(ent);
        }

        for(auto &func : deserializeFuncs) func(ent, j);

    }

    std::unique_ptr<SQLite::Database> db;

    std::vector<std::string> schema = {
            "CREATE TABLE IF NOT EXISTS objects ("
            "   id INTEGER PRIMARY KEY,"
            "   generation INTEGER NOT NULL,"
            "   data TEXT NOT NULL,"
            "   UNIQUE(id, generation)"
            ");",

            "CREATE TABLE IF NOT EXISTS prototypes ("
            "   id INTEGER PRIMARY KEY,"
            "   name TEXT NOT NULL UNIQUE COLLATE NOCASE,"
            "   data TEXT NOT NULL"
            ");",

            "CREATE TABLE IF NOT EXISTS accounts ("
            "   id INTEGER PRIMARY KEY,"
            "   username TEXT NOT NULL UNIQUE COLLATE NOCASE,"
            "   password TEXT NOT NULL DEFAULT '',"
            "   email TEXT NOT NULL DEFAULT '',"
            "   created INTEGER NOT NULL DEFAULT (strftime('%s','now')),"
            "   lastLogin INTEGER NOT NULL DEFAULT (strftime('%s','now')),"
            "   lastLogout INTEGER NOT NULL DEFAULT (strftime('%s','now')),"
            "   lastPasswordChanged INTEGER NOT NULL DEFAULT (strftime('%s','now')),"
            "   totalPlayTime REAL NOT NULL DEFAULT 0,"
            "   totalLoginTime REAL NOT NULL DEFAULT 0,"
            "   disabledReason TEXT NOT NULL DEFAULT '',"
            "   disabledUntil INTEGER NOT NULL DEFAULT 0,"
            "   adminLevel INTEGER NOT NULL DEFAULT 0"
            ");",

            "CREATE TABLE IF NOT EXISTS playerCharacters ("
            "   character INTEGER NOT NULL,"
            "   account INTEGER NOT NULL,"
            "   lastLogin INTEGER NOT NULL DEFAULT (strftime('%s','now')),"
            "   lastLogout INTEGER NOT NULL DEFAULT (strftime('%s','now')),"
            "   totalPlayTime REAL NOT NULL DEFAULT 0,"
            "   FOREIGN KEY(account) REFERENCES accounts(id) ON UPDATE CASCADE ON DELETE CASCADE,"
            "   FOREIGN KEY(character) REFERENCES objects(id) ON UPDATE CASCADE ON DELETE CASCADE,"
            "   PRIMARY KEY(character)"
            ");",
    };

    // Presumably this will only be called if there ARE any dirties.
    void processDirty() {

        SQLite::Statement q1(*db, "INSERT OR REPLACE INTO objects (id, generation, data) VALUES (?, ?, ?);");
        SQLite::Statement q2(*db, "DELETE FROM objects WHERE id = ? AND generation = ?;");

        for(auto &obj : dirty) {
            auto ent = obj.getObject();
            if(registry.valid(ent)) {
                q1.bind(1, static_cast<int64_t>(obj.index));
                q1.bind(2, obj.generation);
                q1.bind(3, serializeEntity(ent).dump(4, ' ', false, nlohmann::json::error_handler_t::ignore));
                q1.exec();
                q1.reset();
            } else {
                q2.bind(1, static_cast<int64_t>(obj.index));
                q2.bind(2, obj.generation);
                q2.exec();
                q2.reset();
            }
        }

        dirty.clear();

    }

    void readyDatabase() {
        db = std::make_unique<SQLite::Database>(config::dbName, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

        SQLite::Transaction trans(*db);
        for(auto &s : schema) {
            db->exec(s);
        }

        trans.commit();
    }

    void loadObjects() {
        // Step 1: get max id of objects table.
        SQLite::Statement q(*db, "SELECT MAX(id) FROM objects;");
        q.executeStep();
        auto count = q.getColumn(0).getInt64();

        // Reserve RAM for a single easy allocation to make this simple...
        objects.resize(count + 50, {0, entt::null});

        std::size_t counter = 0;
        broadcast("Preparing objects for loading...");
        SQLite::Statement q1(*db, "SELECT id, generation FROM objects;");
        while(q1.executeStep()) {
            auto id = q1.getColumn(0).getInt64();
            auto gen = q1.getColumn(1).getInt64();
            auto ent = registry.create();
            registry.emplace<ObjectId>(ent, id, gen);
            objects[id] = {gen, ent};
            counter++;
        }
        broadcast(fmt::format("Prepared {} objects.", counter));

        std::size_t hydrated = 0;
        broadcast("Hydrating objects...");
        // Now we're gonna need to select id and data from objects to deserialize.
        SQLite::Statement q2(*db, "SELECT id, data FROM objects;");
        while(q2.executeStep()) {
            auto id = q2.getColumn(0).getInt64();
            auto data = q2.getColumn(1).getText();
            auto ent = objects[id].second;
            deserializeEntity(ent, nlohmann::json::parse(data));
            hydrated++;
            if(hydrated % 100 == 0) {
                broadcast(fmt::format("Hydrated {}/{} objects.", hydrated, counter));
            }
        }
        broadcast(fmt::format("Hydrated {} objects.", hydrated));

    }

    std::vector<std::function<void()>> preLoadFuncs, postLoadFuncs;

    void loadDatabase() {
        broadcast("Loading game database... please wait warmly...");
        for(auto &func : preLoadFuncs) func();
        loadObjects();
        broadcast("Loaded Objects.");
        for(auto &func : postLoadFuncs) func();
    }

    void savePrototype(const std::string& name, const nlohmann::json& j) {
        SQLite::Statement q(*db, "INSERT OR REPLACE INTO prototypes (name, data) VALUES (?, ?) ON CONFLICT(name) DO UPDATE set data=VALUES(data)");
        q.bind(1, name);
        q.bind(2, j.dump(4, ' ', false, nlohmann::json::error_handler_t::ignore));
        q.exec();
        q.reset();
    }

    std::optional<nlohmann::json> getPrototype(const std::string& name) {
        SQLite::Statement q(*db, "SELECT data FROM prototypes WHERE name = ?;");
        q.bind(1, name);
        if(q.executeStep()) {
            auto data = q.getColumn(0).getText();
            return nlohmann::json::parse(data);
        }
        return std::nullopt;
    }

}