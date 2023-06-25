#include "core/commands.h"
#include "core/components.h"

namespace core {

    boost::regex command_regex(R"((?i)^(?<full>(?<cmd>[^\s\/]+)(?<switches>(\/\w+){0,})?(?:\s+(?<args>(?<lsargs>[^=]+)(?:=(?<rsargs>.*))?))?))");

    std::unordered_map<std::string, std::string> parseCommand(std::string_view input) {
        std::unordered_map<std::string, std::string> out;
        boost::smatch match;
        auto input_str = std::string(input);
        if(boost::regex_match(input_str, match, command_regex)) {
            out["full"] = match["full"];
            out["cmd"] = match["cmd"];
            out["switches"] = match["switches"];
            out["args"] = match["args"];
            out["lsargs"] = match["lsargs"];
            out["rsargs"] = match["rsargs"];
        }
        return out;
    }

    OpResult<> Command::canExecute(entt::entity ent, std::unordered_map<std::string, std::string>& input) {
        return {true, std::nullopt};
    }

    std::set<std::string> BaseCommand::getKeys() {
        std::set<std::string> out;
        out.insert(std::string(getCmdName()));
        auto aliases = getAliases();
        out.insert(aliases.begin(), aliases.end());
        return out;
    }

    void Command::execute(entt::entity ent, std::unordered_map<std::string, std::string>& input) {
        logger->warn("Command {} not implemented", input["cmd"]);
    }

    std::unordered_map<unsigned long long, std::vector<std::pair<std::string, Command*>>> sortedCommandCache;
    std::vector<std::shared_ptr<Command>> commandRegistry;

    unsigned long long getCommandBitset(entt::entity ent) {
        std::bitset<5> out;
        // Bit-order: Character, NPC, Player, Item, Vehicle
        out.set(0, registry.any_of<Character>(ent));
        out.set(1, registry.any_of<NPC>(ent));
        out.set(2, registry.any_of<Player>(ent));
        out.set(3, registry.any_of<Item>(ent));
        out.set(4, registry.any_of<Vehicle>(ent));
        return out.to_ullong();
    }

    std::function<unsigned long long(entt::entity)> getCommandUll = getCommandBitset;

    OpResult<> registerCommand(const std::shared_ptr<Command>& entry) {
        if(entry->getCmdName().empty()) {
            return {false, "CommandEntry cmdName cannot be empty"};
        }
        commandRegistry.push_back(entry);
        return {true, std::nullopt};
    }

    OpResult<> registerLoginCommand(const std::shared_ptr<LoginCommand>& entry) {
        if(entry->getCmdName().empty()) {
            return {false, "CommandEntry cmdName cannot be empty"};
        }
        loginCommandRegistry[std::string(entry->getCmdName())] = entry;
        return {true, std::nullopt};
    }

    OpResult<> registerConnectCommand(const std::shared_ptr<ConnectCommand>& entry) {
        if(entry->getCmdName().empty()) {
            return {false, "CommandEntry cmdName cannot be empty"};
        }
        connectCommandRegistry[std::string(entry->getCmdName())] = entry;
        return {true, std::nullopt};
    }

    std::unordered_map<std::string, std::shared_ptr<ConnectCommand>> connectCommandRegistry, expandedConnectCommandRegistry;
    std::unordered_map<std::string, std::shared_ptr<LoginCommand>> loginCommandRegistry, expandedLoginCommandRegistry;

    OpResult<> ConnectCommand::canExecute(const std::shared_ptr<Connection>& connection, std::unordered_map<std::string, std::string>& input) {
        return {true, std::nullopt};
    }

    void ConnectCommand::execute(const std::shared_ptr<Connection>& connection, std::unordered_map<std::string, std::string>& input) {
        logger->warn("ConnectCommand {} not implemented", input["cmd"]);
    }

    OpResult<> LoginCommand::canExecute(const std::shared_ptr<Connection>& connection, std::unordered_map<std::string, std::string>& input) {
        return {true, std::nullopt};
    }

    void LoginCommand::execute(const std::shared_ptr<Connection>& connection, std::unordered_map<std::string, std::string>& input) {
        logger->warn("LoginCommand {} not implemented", input["cmd"]);
    }

    void expandCommands() {
        // sort the commandRegistry.
        std::sort(commandRegistry.begin(), commandRegistry.end(), [](const auto& a, const auto& b) {
            return a->getPriority() < b->getPriority();
        });

        // Expand the other categories into their caches.
        for(auto& [cmdName, cmd] : connectCommandRegistry) {
            for(auto& key : cmd->getKeys()) {
                expandedConnectCommandRegistry[boost::algorithm::to_lower_copy(key)] = cmd;
            }
        }
        for(auto& [cmdName, cmd] : loginCommandRegistry) {
            for(auto& key : cmd->getKeys()) {
                expandedLoginCommandRegistry[boost::algorithm::to_lower_copy(key)] = cmd;
            }
        }
    }



    std::vector<std::pair<std::string, Command*>>& defaultGetSortedCommands(entt::entity ent) {
        // Okay this one's a bit tricky. First, we will get the cmdbitset for this entity...
        auto ull = getCommandUll(ent);
        // We can't directly index a map or unordered_map by bitset; however, we can convert it to a uint64_t!
        // if the bitset is found in sortedCommandCache, return that.
        auto found = sortedCommandCache.find(ull);
        if(found != sortedCommandCache.end()) {
            return found->second;
        }
        // Oops, no cache was found. In this case, we are gonna need to iterate through the commandRegistry, and
        // generate a new cache entry.
        // We will do this by filling up a new std::unordered_map<std::string, Command*> with all the commands that
        // are allowed for this entity by checking Command::isAavailable(entt::entity). As the commandRegistry is sorted
        // by priority, commands with matching names will replace previous entries in the map.
        // After we have our unordered_map, we can then expand it by getKeys() into the sortedCommandCache. (and re-sort it.)
        std::unordered_map<std::string, Command*> out;
        for(auto& cmd : commandRegistry) {
            if(cmd->isAvailable(ent)) {
                out[cmd->getCmdName()] = cmd.get();
            }
        }
        auto &cache = sortedCommandCache[ull];
        // Now we can expand the keys into the sortedCommandCache.
        for(auto& [cmdName, cmd] : out) {
            for(auto& key : cmd->getKeys()) {
                cache.emplace_back(boost::algorithm::to_lower_copy(key), cmd);
            }
        }
        // Now we can sort the cache.
        std::sort(cache.begin(), cache.end(), [](const auto& a, const auto& b) {
            return a.second->getPriority() < b.second->getPriority();
        });
        // And return it!
        return cache;
    }
    std::function<std::vector<std::pair<std::string, Command*>>&(entt::entity)> getSortedCommands = defaultGetSortedCommands;

}