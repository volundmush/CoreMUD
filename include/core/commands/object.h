#pragma once
#include "core/commands.h"
#include "core/components.h"

namespace core::cmd {

    // Object Commands.
    struct ObjCmd : Command {
        [[nodiscard]] bool isAvailable(entt::entity ent) override {return registry.any_of<Character,Item,Vehicle>(ent);};
    };

    /*
     * The look command is a staple of MUDs; it should show you where you are and what the room looks like.
     * It could also be used to look AT something, like a character or item in the room or in your inventory.
     */
    struct ObjLook : ObjCmd {
        [[nodiscard]] std::string getCmdName() override {return "look";};
        [[nodiscard]] std::set<std::string> getAliases() override {return {"l"};};
        void execute(entt::entity ent, std::unordered_map<std::string, std::string>& input) override;
    };

    /*
     * This help command will be able to get help on any command you have access to.
     */
    struct ObjHelp : ObjCmd {
        [[nodiscard]] std::string getCmdName() override {return "help";};
        [[nodiscard]] std::set<std::string> getAliases() override {return {"h"};};
        void execute(entt::entity ent, std::unordered_map<std::string, std::string>& input) override;
    };

    /*
     * This command will allow you to move from room to room.
     * This is an "object" command because many things should have this functionality
     * if an object is being possessed for normal control by a player or admin.
     * For example, controls might be passed to a spaceship or other vehicle.
     * Or, an admin might be possessing a statue (technically an item) and
     * wants to move to a different room...
     */
    struct ObjMove : ObjCmd {
        [[nodiscard]] std::string getCmdName() override { return "move"; };
        [[nodiscard]] std::set<std::string> getAliases() override { return {"mv", "go",
                                                                            "north", "n",
                                                                            "south", "s",
                                                                            "east", "e",
                                                                            "west", "w",
                                                                            "up", "u",
                                                                            "down", "d",
                                                                            "in", "inside",
                                                                            "out", "outside",
                                                                            "northwest", "nw",
                                                                            "northeast", "ne",
                                                                            "southwest", "sw",
                                                                            "southeast", "se"}; };
        void execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) override;
        OpResult<> canExecute(entt::entity ent, std::unordered_map<std::string, std::string> &input) override;
    };

    /*
     * The QUIT command is used to exit play and return to the Account menu.
     */
    struct ObjQuit : ObjCmd {
        [[nodiscard]] std::string getCmdName() override {return "quit";};
        void execute(entt::entity ent, std::unordered_map<std::string, std::string>& input) override;
    };

    /*
     * The SAY command is used to speak to other players in the same room.
     * It's an Object Command for the same reason as "move".
     */
    struct ObjSay : ObjCmd {
        [[nodiscard]] std::string getCmdName() override { return "say"; };
        void execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) override;
    };

    /*
     * The POSE command is used to emote.
     * It's an Object Command for the same reason as "move".
     *
     * This would show to others, "<actor> <pose>"
     */
    struct ObjPose : ObjCmd {
        [[nodiscard]] std::string getCmdName() override { return "pose"; };
        [[nodiscard]] std::set<std::string> getAliases() override {return {";", "emote"};};
        void execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) override;
    };

    /*
     * The SEMIPOSE command is used to emote, but slightly different in that it lacks the space.
     * This is good for slightly differently formatted things, like adding an apostrophe's.
     * It's an Object Command for the same reason as "move".
     *
     * This would show to others, "<actor><pose>"
     */
    struct ObjSemipose : ObjCmd {
        [[nodiscard]] std::string getCmdName() override { return "semipose"; };
        [[nodiscard]] std::set<std::string> getAliases() override {return {":"};};
        void execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) override;
    };

    /*
     * The WHISPER command is used to speak to other players in the same room.
     */
    struct ObjWhisper : ObjCmd {
        [[nodiscard]] std::string getCmdName() override { return "whisper"; };
        void execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) override;
    };

    /*
     * The SHOUT command is used to speak to other players in the same area.
     */
    struct ObjShout : ObjCmd {
        [[nodiscard]] std::string getCmdName() override { return "shout"; };
        [[nodiscard]] std::set<std::string> getAliases() override {return {"yell"};};
        void execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) override;
    };

    /*
     * The "get" command is used to retrieve an object from the room or, sometimes, an object in the room.
     */
    struct ObjGet : ObjCmd {
        [[nodiscard]] std::string getCmdName() override { return "get"; };
        void execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) override;
    };

    /*
     * The "take" command is used to retrieve an object from an item in your inventory or equipment,
     * and place it in your inventory.
     */
    struct ObjTake : ObjCmd {
        [[nodiscard]] std::string getCmdName() override { return "take"; };
        void execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) override;
    };

    /*
     * The "put" command is used to put an object into another object.
     */
    struct ObjPut : ObjCmd {
        [[nodiscard]] std::string getCmdName() override { return "put"; };
        void execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) override;
    };

    /*
     * The "give" command is used to give an object to another player.
     */
    struct ObjGive : ObjCmd {
        [[nodiscard]] std::string getCmdName() override { return "give"; };
        void execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) override;
    };

    /*
     * The "drop" command is used to drop an object from your inventory into the room.
     */
    struct ObjDrop : ObjCmd {
        [[nodiscard]] std::string getCmdName() override { return "drop"; };
        void execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) override;
    };

    /*
     * The "inventory" command is used to list the objects in your inventory.
     */
    struct ObjInventory : ObjCmd {
        [[nodiscard]] std::string getCmdName() override { return "inventory"; };
        [[nodiscard]] std::set<std::string> getAliases() override {return {"inv", "i"};};
        void execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) override;
    };


    struct ObjEquip : ObjCmd {
        [[nodiscard]] std::string getCmdName() override { return "equip"; };
        [[nodiscard]] std::set<std::string> getAliases() override {return {"eq", "wear", "wield", "hold"};};
        void execute(entt::entity ent, std::unordered_map<std::string, std::string> &input) override;
    };


    void registerObjectCommands();

}