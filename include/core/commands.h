#pragma once
#include "core/base.h"

namespace core {

    /*
     * The base Command class used for all commands that'll be used by
     * game objects.
     */

    struct BaseCommand {
        [[nodiscard]] virtual std::set<std::string> getAliases() {return {};};
        [[nodiscard]] virtual std::set<std::string> getKeys();
        [[nodiscard]] virtual std::string getCmdName() = 0;
        [[nodiscard]] virtual std::string getHelp() {return "";};
        [[nodiscard]] virtual std::string getHelpCategory() {return "Uncategorized";};
        [[nodiscard]] virtual int getPriority() {return 0;};
    };

    struct Command : BaseCommand {
        [[nodiscard]] virtual OpResult<> canExecute(entt::entity ent, std::unordered_map<std::string, std::string>& input);
        virtual void execute(entt::entity ent, std::unordered_map<std::string, std::string>& input);
        [[nodiscard]] virtual bool isAvailable(entt::entity ent) {return true;};
    };

    // To maximize flexibility, CoreMUD assumes that each object can be scanned and generate an associated unsigned
    // long long which represents some fairly static aspect about the object and the commands it can access.
    // It is encouraged that this be a bitset representing the presence of relevant components relevant to object
    // types. Example: A bitset which represents the presence of Character, NPC, Player, Item, and Vehicle components,
    // in that order, which is then turned into a ULL for the indexing.
    // Then, when a combination of components hasn't been encountered yet, the getSortedCommands function will handle
    // generating them and returning them.
    // However, this isn't the only way of going about it at all. The getSortedCommands function can be overridden.

    extern std::unordered_map<unsigned long long, std::vector<std::pair<std::string, Command*>>> sortedCommandCache;
    extern std::function<unsigned long long(entt::entity)> getCommandUll;

    unsigned long long getCommandBitset(entt::entity ent);

    std::vector<std::pair<std::string, Command*>>& defaultGetSortedCommands(entt::entity ent);
    extern std::function<std::vector<std::pair<std::string, Command*>>&(entt::entity)> getSortedCommands;
    extern std::vector<std::shared_ptr<Command>> commandRegistry;

    OpResult<> registerCommand(const std::shared_ptr<Command>& entry);

    extern boost::regex command_regex;

    std::unordered_map<std::string, std::string> parseCommand(std::string_view input);

    /*
     * The base Command class used for all commands that'll be used by
     * connections at the welcome screen, including logging in and creating
     * accounts.
     */
    struct ConnectCommand : BaseCommand {
        [[nodiscard]] virtual OpResult<> canExecute(const std::shared_ptr<Connection>& connection, std::unordered_map<std::string, std::string>& input);
        virtual void execute(const std::shared_ptr<Connection>& connection, std::unordered_map<std::string, std::string>& input);
        [[nodiscard]] virtual bool isAvailable(const std::shared_ptr<Connection>& connection) {return true;};
    };

    extern std::unordered_map<std::string, std::shared_ptr<ConnectCommand>> connectCommandRegistry, expandedConnectCommandRegistry;
    OpResult<> registerConnectCommand(const std::shared_ptr<ConnectCommand>& entry);
    /*
     * After logging into an Account, players will have access to these
     * commands.
     */
    struct LoginCommand : BaseCommand {
        [[nodiscard]] virtual OpResult<> canExecute(const std::shared_ptr<Connection>& connection, std::unordered_map<std::string, std::string>& input);
        virtual void execute(const std::shared_ptr<Connection>& connection, std::unordered_map<std::string, std::string>& input);
        [[nodiscard]] virtual bool isAvailable(const std::shared_ptr<Connection>& connection) {return true;};
    };

    extern std::unordered_map<std::string, std::shared_ptr<LoginCommand>> loginCommandRegistry, expandedLoginCommandRegistry;
    OpResult<> registerLoginCommand(const std::shared_ptr<LoginCommand>& entry);

    void expandCommands();
}