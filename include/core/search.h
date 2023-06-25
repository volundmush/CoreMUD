#pragma once
#include "core/base.h"

namespace core {

    enum class SearchContainer {
        Room = 0,
        Inventory = 1,
        Equipment = 2
    };

    enum class SearchType {
        Anything = 0, // Characters, Items, and Vehicles... anything, really.
        Characters = 1, // will also catch players and NPCs!
        Players = 2,
        NPCs = 3,
        Vehicles = 4,
        Items = 5
    };

    class Search {
    public:
        explicit Search(entt::entity ent);
        Search& in(entt::entity inventory);
        Search& eq(entt::entity equipment);
        Search& room(entt::entity room);
        Search& modes(uint64_t m);
        Search& useId(bool useId);
        Search& useSelf(bool useSelf);
        Search& useAll(bool useAll);
        Search& useHere(bool useHere);
        Search& useAsterisk(bool useAsterisk);
        Search& setType(SearchType t);

        virtual std::vector<entt::entity> find(std::string_view name);

    protected:
        OpResult<entt::entity> _simplecheck(std::string_view name);
        bool detect(entt::entity target);
        entt::entity ent;
        std::string txt;
        std::vector<std::pair<SearchContainer, entt::entity>> searchLocations;
        uint64_t useModes{0};
        SearchType type{SearchType::Anything};
        bool allowId{false};
        bool allowSelf{true};
        bool allowAll{true};
        bool allowHere{false};
        bool allowAsterisk{false};
    };
}