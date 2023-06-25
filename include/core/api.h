#pragma once

#include "core/base.h"
#include "core/components.h"


namespace core {

    ObjectId getObjectId(entt::entity ent);

    OpResult<> setParent(entt::entity ent, entt::entity target);
    entt::entity getParent(entt::entity ent);
    void addToChildren(entt::entity ent, entt::entity target);
    void removeFromChildren(entt::entity ent, entt::entity target);
    void atChildDeleted(entt::entity ent, entt::entity target);
    void atParentDeleted(entt::entity ent, entt::entity target);
    std::vector<entt::entity> getChildren(entt::entity ent);

    OpResult<> setOwner(entt::entity ent, entt::entity target);
    entt::entity getOwner(entt::entity ent);
    void addToAssets(entt::entity ent, entt::entity target);
    void removeFromAssets(entt::entity ent, entt::entity target);
    void atAssetDeleted(entt::entity ent, entt::entity target);
    void atOwnerDeleted(entt::entity ent, entt::entity target);
    std::vector<entt::entity> getAssets(entt::entity ent);

    OpResult<> setLocation(entt::entity ent, entt::entity target);
    entt::entity getLocation(entt::entity ent);
    void addToContents(entt::entity ent, entt::entity target);
    void removeFromContents(entt::entity ent, entt::entity target);
    void atContentDeleted(entt::entity ent, entt::entity target);
    void atLocationDeleted(entt::entity ent, entt::entity target);
    std::vector<entt::entity> getContents(entt::entity ent);

    template<typename T>
    void setBaseText(entt::entity ent, const std::string& txt) {
        auto &comp = registry.get_or_emplace<T>(ent);
        comp.setData(txt);
    }

    template<typename T>
    std::string_view getBaseText(entt::entity ent) {
        auto comp = registry.try_get<T>(ent);
        if(comp) return comp->data;
        return "";
    }

    std::string defaultGetName(entt::entity ent);
    extern std::function<std::string(entt::entity)> getName;
    void defaultSetName(entt::entity ent, const std::string& txt);
    extern std::function<void(entt::entity, const std::string&)> setName;

    std::string getShortDescription(entt::entity ent);
    void setShortDescription(entt::entity ent, const std::string& txt);

    std::string getRoomDescription(entt::entity ent);
    void setRoomDescription(entt::entity ent, const std::string& txt);

    std::string getLookDescription(entt::entity ent);
    void setLookDescription(entt::entity ent, const std::string& txt);

    std::string defaultGetDisplayName(entt::entity ent, entt::entity looker);
    extern std::function<std::string(entt::entity, entt::entity)> getDisplayName;
    std::string defaultGetRoomLine(entt::entity ent, entt::entity looker);
    extern std::function<std::string(entt::entity, entt::entity)> getRoomLine;
    std::string defaultRenderAppearance(entt::entity ent, entt::entity looker);
    extern std::function<std::string(entt::entity, entt::entity)> renderAppearance;

    void defaultAtDeleteObject(entt::entity ent);
    extern std::function<void(entt::entity)> atDeleteObject;

    extern std::function<std::set<std::string>(entt::entity, entt::entity)> getSearchWords;
    std::set<std::string> defaultGetSearchWords(entt::entity ent, entt::entity looker);

    extern std::function<bool(entt::entity, std::string_view, entt::entity)> checkSearch;
    bool defaultCheckSearch(entt::entity ent, std::string_view term, entt::entity looker);

    extern std::function<bool(entt::entity, entt::entity, uint64_t)> canDetect;
    bool defaultCanDetect(entt::entity ent, entt::entity target, uint64_t modes);

    extern std::function<bool(entt::entity)> isEquipped;
    bool defaultIsEquipped(entt::entity ent);

    extern std::function<bool(entt::entity)> isInventory;
    bool defaultIsInventory(entt::entity ent);

    extern std::function<std::vector<entt::entity>(entt::entity)> getInventory;
    std::vector<entt::entity> defaultGetInventory(entt::entity ent);

    extern std::function<std::vector<entt::entity>(entt::entity)> getEquipment;
    std::vector<entt::entity> defaultGetEquipment(entt::entity ent);

    std::optional<std::vector<double>> parseCoordinates(const std::string& str);

    std::optional<Destination> defaultValidDestination(entt::entity ent, const std::string& str);
    extern std::function<std::optional<Destination>(entt::entity, const std::string&)> validDestination;

    std::vector<entt::entity> defaultGetRoomContents(entt::entity ent);
    extern std::function<std::vector<entt::entity>(entt::entity)> getRoomContents;

}