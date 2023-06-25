#include "core/api.h"
#include "core/components.h"
#include "core/color.h"

namespace core {

    void defaultSetName(entt::entity ent, const std::string &txt) {
        setBaseText<Name>(ent, txt);
    }
    //setName can be replaced in order to handle special checks for certain kinds of
    // entities, like player characters.
    std::function<void(entt::entity, const std::string &)> setName = defaultSetName;

    void setShortDescription(entt::entity ent, const std::string &txt) {
        setBaseText<ShortDescription>(ent, txt);
    }

    void setRoomDescription(entt::entity ent, const std::string &txt) {
        setBaseText<RoomDescription>(ent, txt);
    }

    void setLookDescription(entt::entity ent, const std::string &txt) {
        setBaseText<LookDescription>(ent, txt);
    }

    std::string defaultGetName(entt::entity ent) {
        auto name = getBaseText<Name>(ent);
        if(!name.empty()) return std::string(name);
        return "Unnamed Object";
    }
    std::function<std::string(entt::entity)> getName = defaultGetName;

    std::string getShortDescription(entt::entity ent) {
        return std::string(getBaseText<ShortDescription>(ent));
    }

    std::string getRoomDescription(entt::entity ent) {
        return std::string(getBaseText<RoomDescription>(ent));
    }

    std::string getLookDescription(entt::entity ent) {
        return std::string(getBaseText<LookDescription>(ent));
    }

    std::string defaultGetDisplayName(entt::entity ent, entt::entity looker) {
        if(registry.any_of<Item, NPC>(ent)) {
            return getShortDescription(ent);
        }

        if(registry.any_of<Character>(ent)) {
            if(registry.any_of<Player>(ent) && registry.any_of<Player>(looker)) {
                // We need to use the dub system here.
                return getName(ent);
            }
            return getName(ent);
        }
        return getName(ent);
    }
    std::function<std::string(entt::entity, entt::entity)> getDisplayName = defaultGetDisplayName;

    std::string defaultGetRoomLine(entt::entity ent, entt::entity looker) {
        if(registry.any_of<Item, NPC>(ent)) {
            return getRoomDescription(ent);
        }

        if(registry.any_of<Character>(ent)) {
            if(registry.any_of<Player>(ent) && registry.any_of<Player>(looker)) {
                // We need to use the dub system here... but a placeholder is used for now.
                return getDisplayName(ent, looker) + " is here.";
            }
            return getDisplayName(ent, looker) + " is here.";
        }
        return getDisplayName(ent, looker) + " is here.";
    }
    std::function<std::string(entt::entity, entt::entity)> getRoomLine = defaultGetRoomLine;

    // A proper implementation of this will probably use switching logic to run different functions
    // based on the type of entity. This is just a mockup.
    std::string defaultRenderAppearance(entt::entity ent, entt::entity looker) {
        std::vector<std::string> out;
        out.push_back(getDisplayName(ent, looker));
        auto rdesc = getRoomDescription(ent);
        if(!rdesc.empty()) out.push_back(rdesc);
        return boost::join(out, "\n");
    }
    std::function<std::string(entt::entity, entt::entity)> renderAppearance = defaultRenderAppearance;

    ObjectId getObjectId(entt::entity ent) {
        return registry.get<ObjectId>(ent);
    }

    OpResult<> setParent(entt::entity ent, entt::entity target) {
        if(registry.valid(target)) {
            // We must look up the parent chain to ensure no shenanigans.
            auto parent = target;
            while(registry.valid(parent)) {
                if(parent == ent) {
                    return {false, "That would cause a recursive relationship... very bad."};
                }
                parent = getParent(parent);
            }
        }

        entt::entity oldParent = getParent(ent);
        if(registry.valid(oldParent)) {
            removeFromChildren(oldParent, ent);
        }

        if(registry.valid(target)) {
            addToChildren(target, ent);
            auto &par = registry.get_or_emplace<Parent>(ent);
            par.data = target;
        } else {
            registry.remove<Parent>(ent);
        }
        return {true, std::nullopt};

    }

    entt::entity getParent(entt::entity ent) {
        if(auto par = registry.try_get<Parent>(ent)) {
            return par->data;
        }
        return entt::null;
    }

    void addToChildren(entt::entity ent, entt::entity child) {
        if(registry.valid(ent)) {
            auto &children = registry.get_or_emplace<Children>(ent);
            children.data.push_back(child);
        }
    }

    void removeFromChildren(entt::entity ent, entt::entity child) {
        if(registry.valid(ent)) {
            auto &children = registry.get_or_emplace<Children>(ent);
            children.data.erase(std::remove(children.data.begin(), children.data.end(), child), children.data.end());
        }
    }

    std::vector<entt::entity> getChildren(entt::entity ent) {
        if(registry.valid(ent)) {
            if(auto children = registry.try_get<Children>(ent)) {
                return children->data;
            }
        }
        return {};
    }

    void atChildDeleted(entt::entity ent, entt::entity target) {
        removeFromChildren(ent, target);
    }

    void atParentDeleted(entt::entity ent, entt::entity target) {
        setParent(ent, entt::null);
    }

    OpResult<> setOwner(entt::entity ent, entt::entity target) {
        if(registry.valid(target)) {
            // We must look up the parent chain to ensure no shenanigans.
            auto parent = target;
            while(registry.valid(parent)) {
                if(parent == ent) {
                    return {false, "That would cause a recursive relationship... very bad."};
                }
                parent = getOwner(parent);
            }
        }

        entt::entity oldOwner = getOwner(ent);
        if(registry.valid(oldOwner)) {
            removeFromAssets(oldOwner, ent);
        }

        if(registry.valid(target)) {
            addToAssets(target, ent);
            auto &par = registry.get_or_emplace<Owner>(ent);
            par.data = target;
        } else {
            registry.remove<Owner>(ent);
        }
        return {true, std::nullopt};

    }

    entt::entity getOwner(entt::entity ent) {
        if(auto par = registry.try_get<Owner>(ent)) {
            return par->data;
        }
        return entt::null;
    }

    void addToAssets(entt::entity ent, entt::entity child) {
        if(registry.valid(ent)) {
            auto &children = registry.get_or_emplace<Assets>(ent);
            children.data.push_back(child);
        }
    }

    void removeFromAssets(entt::entity ent, entt::entity child) {
        if(registry.valid(ent)) {
            auto &children = registry.get_or_emplace<Assets>(ent);
            children.data.erase(std::remove(children.data.begin(), children.data.end(), child), children.data.end());
        }
    }

    std::vector<entt::entity> getAssets(entt::entity ent) {
        if(registry.valid(ent)) {
            if(auto children = registry.try_get<Assets>(ent)) {
                return children->data;
            }
        }
        return {};
    }

    void atAssetDeleted(entt::entity ent, entt::entity target) {
        removeFromAssets(ent, target);
    }

    void atOwnerDeleted(entt::entity ent, entt::entity target) {
        setOwner(ent, entt::null);
    }

    OpResult<> setLocation(entt::entity ent, entt::entity target) {
        if(registry.valid(target)) {
            // We must look up the parent chain to ensure no shenanigans.
            auto parent = target;
            while(registry.valid(parent)) {
                if(parent == ent) {
                    return {false, "That would cause a recursive relationship... very bad."};
                }
                parent = getLocation(parent);
            }
        }

        entt::entity oldLocation = getLocation(ent);
        if(registry.valid(oldLocation)) {
            removeFromContents(oldLocation, ent);
        }

        if(registry.valid(target)) {
            addToContents(target, ent);
            auto &par = registry.get_or_emplace<Location>(ent);
            par.data = target;
        } else {
            registry.remove<Location>(ent);
        }
        return {true, std::nullopt};

    }

    entt::entity getLocation(entt::entity ent) {
        if(auto par = registry.try_get<Location>(ent)) {
            return par->data;
        }
        return entt::null;
    }

    void addToContents(entt::entity ent, entt::entity child) {
        if(registry.valid(ent)) {
            auto &children = registry.get_or_emplace<Contents>(ent);
            children.data.push_back(child);
        }
    }

    void removeFromContents(entt::entity ent, entt::entity child) {
        if(registry.valid(ent)) {
            auto &children = registry.get_or_emplace<Contents>(ent);
            children.data.erase(std::remove(children.data.begin(), children.data.end(), child), children.data.end());
        }
    }

    std::vector<entt::entity> getContents(entt::entity ent) {
        if(registry.valid(ent)) {
            if(auto children = registry.try_get<Contents>(ent)) {
                return children->data;
            }
        }
        return {};
    }

    void atContentDeleted(entt::entity ent, entt::entity target) {
        removeFromContents(ent, target);
    }

    void atLocationDeleted(entt::entity ent, entt::entity target) {
        setLocation(ent, entt::null);
        registry.erase<GridLocation>(ent);
        registry.erase<RoomLocation>(ent);
        registry.erase<SectorLocation>(ent);
        // TODO: punt people in a deleted zone somewhere safe.
        // They SHOULD be evacuated first, by a higher-level
        // routine, but we need a failsafe for any stragglers.
    }

    void defaultAtDeleteObject(entt::entity ent) {
        if(auto par = registry.try_get<Parent>(ent)) {
            atChildDeleted(par->data, ent);
        }
        if(auto par = registry.try_get<Children>(ent)) {
            for(auto child : par->data) {
                atParentDeleted(child, ent);
            }
        }
        if(auto par = registry.try_get<Owner>(ent)) {
            atAssetDeleted(par->data, ent);
        }
        if(auto par = registry.try_get<Assets>(ent)) {
            for(auto child : par->data) {
                atOwnerDeleted(child, ent);
            }
        }
        if(auto par = registry.try_get<Location>(ent)) {
            atContentDeleted(par->data, ent);
        }
        if(auto par = registry.try_get<Contents>(ent)) {
            for(auto child : par->data) {
                atLocationDeleted(child, ent);
            }
        }
    }
    std::function<void(entt::entity)> atDeleteObject = defaultAtDeleteObject;

    std::set<std::string> defaultGetSearchWords(entt::entity ent, entt::entity looker) {
        auto name = stripAnsi(getDisplayName(ent, looker));
        std::set<std::string> words;
        boost::split(words, name, boost::algorithm::is_space());
        return words;
    }
    std::function<std::set<std::string>(entt::entity, entt::entity)> getSearchWords = defaultGetSearchWords;


    bool defaultCheckSearch(entt::entity ent, std::string_view term, entt::entity looker) {
        auto w = getSearchWords(ent, looker);
        std::vector<std::string> words;
        // Sort w into words by string length, smallest first and longest last.
        std::transform(w.begin(), w.end(), std::back_inserter(words), [](const std::string& s) { return s; });
        std::sort(words.begin(), words.end(), [](const std::string& a, const std::string& b) { return a.size() < b.size(); });

        for(auto& word : words) {
            if(boost::istarts_with(word, term)) return true;
        }
        return false;
    }
    std::function<bool(entt::entity, std::string_view, entt::entity)> checkSearch = defaultCheckSearch;


    bool defaultCanDetect(entt::entity ent, entt::entity target, uint64_t modes) {
        return true;
    }
    std::function<bool(entt::entity, entt::entity, uint64_t)> canDetect = defaultCanDetect;

    // It's up to each game to determine their own scheme for how equipment works.
    // But this API being present in Core helps a lot of other things work nicely.
    bool defaultIsEquipped(entt::entity ent) {
        return false;
    }
    std::function<bool(entt::entity)> isEquipped = defaultIsEquipped;

    bool defaultIsInventory(entt::entity ent) {
        return !isEquipped(ent) && !registry.any_of<RoomLocation, GridLocation, SectorLocation>(ent);
    }
    std::function<bool(entt::entity)> isInventory = defaultIsInventory;

    std::vector<entt::entity> defaultGetInventory(entt::entity ent) {
        // A default inventory is the contents of the object where the object is not equipped, and has no
        // RoomLocation, GridLocation, or SectorLocation.
        auto contents = getContents(ent);
        if(contents.empty()) return {};

        // Filter contents by isInventory.
        std::vector<entt::entity> inventory;
        std::copy_if(contents.begin(), contents.end(), std::back_inserter(inventory), isInventory);
        return inventory;
    }
    std::function<std::vector<entt::entity>(entt::entity)> getInventory = defaultGetInventory;

    // It's up to an individual game to determine what counts as equipment, how equipment slots work, etc,
    // but this function should still return a vector of equipment entities.
    std::vector<entt::entity> defaultGetEquipment(entt::entity ent) {
        // There may be better logic in the future, but for default we can do the opposite of defaultGetInventory, kinda.
        auto contents = getContents(ent);
        if(contents.empty()) return {};

        std::vector<entt::entity> equipped;
        std::copy_if(contents.begin(), contents.end(), std::back_inserter(equipped), isEquipped);
        return equipped;
    }
    std::function<std::vector<entt::entity>(entt::entity)> getEquipment = defaultGetEquipment;

    std::optional<std::vector<double>> parseCoordinates(const std::string& str) {
        std::vector<double> result;
        std::istringstream ss(str);
        std::string token;

        while (std::getline(ss, token, ',')) {
            try {
                double d = std::stod(token);
                result.push_back(d);
            } catch (const std::exception& e) {
                // Failed to parse token as double
                return std::nullopt;
            }
        }

        return result;
    }

    std::optional<Destination> defaultValidDestination(entt::entity ent, const std::string& str) {
        if(!registry.valid(ent)) return std::nullopt;

        std::vector<double> coordinates;
        if(str.empty()) {
            coordinates = {0.0, 0.0, 0.0};
        } else {
            auto parsed = parseCoordinates(str);
            if(!parsed) return std::nullopt;
            coordinates = *parsed;
        }
        // if coordinates has less than 3 elements, fill remaining with 0.0.
        coordinates.resize(3, 0.0);
        Destination dest;
        dest.ent = ent;

        if(auto area = registry.try_get<Area>(ent)) {
            // check if coordinates[0] is a RoomId in area->data
            // but first we must turn it into a RoomId from a double.
            RoomId id = coordinates[0];
            if(area->data.contains(id)) {
                dest.destination = id;
                return dest;
            } else {
                return std::nullopt;
            }
        }
        else if(auto expanse = registry.try_get<Expanse>(ent)) {
            GridLength x = coordinates[0];
            GridLength y = coordinates[1];
            GridLength z = coordinates[2];
            // check if x y z is within the expanse minX/Y/Z and maxX/Y/Z
            if(x >= expanse->minX && x <= expanse->maxX
                && y >= expanse->minY && y <= expanse->maxY
                && z >= expanse->minZ && z <= expanse->maxZ) {
                GridPoint gp(x, y, z);
                dest.destination = gp;
                return dest;
            } else {
                return std::nullopt;
            }

        }
        else if(auto map = registry.try_get<Map>(ent)) {
            GridLength x = coordinates[0];
            GridLength y = coordinates[1];
            GridLength z = coordinates[2];
            // check if x y z is within the map minX/Y/Z and maxX/Y/Z
            if(x >= map->minX && x <= map->maxX
                && y >= map->minY && y <= map->maxY
                && z >= map->minZ && z <= map->maxZ) {
                GridPoint gp(x, y, z);
                dest.destination = gp;
                return dest;
            } else {
                return std::nullopt;
            }
        }
        else if(auto space = registry.try_get<Space>(ent)) {
            SectorLength x = coordinates[0];
            SectorLength y = coordinates[1];
            SectorLength z = coordinates[2];
            // check if x y z is within the space minX/Y/Z and maxX/Y/Z
            if(x >= space->minX && x <= space->maxX
                && y >= space->minY && y <= space->maxY
                && z >= space->minZ && z <= space->maxZ) {
                SectorPoint sp(x, y, z);
                dest.destination = sp;
                return dest;
            } else {
                return std::nullopt;
            }
        }
        return std::nullopt;

    }
    std::function<std::optional<Destination>(entt::entity, const std::string&)> validDestination = defaultValidDestination;

    std::vector<entt::entity> defaultGetRoomContents(entt::entity ent) {
        auto rcon = registry.try_get<RoomContents>(ent);
        if(!rcon) return {};
        return rcon->data;
    }
    std::function<std::vector<entt::entity>(entt::entity)> getRoomContents = defaultGetRoomContents;

}