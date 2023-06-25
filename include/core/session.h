#pragma once
#include "core/connection.h"

namespace core {
    // The Session class represents a specific session of play while a Character is online.
    // A Session is created when a Character logs in, and is destroyed when the Character logs out.
    // It acts as a middle-man and also a repository for data relevant to just this session, and
    // as such it also allows multiple connections to be linked to the same Character simultaneously,
    // all seeing the same thing and accepting input from all of them in first-come-first-served order.
    // This is useful for allowing a player to be logged in from multiple devices at once, or from
    // multiple locations.
    class Session {
    public:
        Session(ObjectId id, int64_t account, entt::entity character);
        ~Session() = default;

        virtual void start();
        virtual void end();

        // Send a message to the linked connections.
        virtual void send(const Message &msg);
        virtual void atObjectDeleted(entt::entity ent);

        // Add a connection to this session.
        void addConnection(int64_t connId);
        void addConnection(const std::shared_ptr<Connection>& conn);
        virtual void onAddConnection(const std::shared_ptr<Connection>& conn);
        // Remove a connection from this session.
        void removeConnection(int64_t connId);
        void removeConnection(const std::shared_ptr<Connection>& conn);
        virtual void onRemoveConnection(const std::shared_ptr<Connection>& conn);
        // Remove all connections from this session.
        void removeAllConnections();

        virtual void onFirstConnection();

        virtual void onLinkDead();

        virtual void onNetworkDisconnected(int64_t connId);

        virtual void changePuppet(entt::entity ent);

        virtual void handleText(const std::string& text);

        virtual void sendText(const std::string& txt);
        virtual void sendLine(const std::string& txt);
        virtual void onHeartbeat(double deltaTime);
        virtual void sendOutput(double deltaTime);

        entt::entity getCharacter() const { return character;};
        entt::entity getPuppet() const { return puppet;};
        int64_t getAccount() const { return account; };
        int16_t getAdminLevel() const { return adminLevel; };
        std::chrono::steady_clock::time_point getLastActivity() const { return lastActivity; };
        std::chrono::system_clock::time_point getCreated() const { return created; };
        ObjectId getId() const { return id; };

    protected:
        // The ID of the character is the ID of the session.
        ObjectId id;
        // The character this Session is linked to.
        entt::entity character;
        // The object this session is currently controlling. That's USUALLY going to be the Character,
        // but it might not be. For example, if the Character is in a vehicle, the vehicle might be
        // the puppet.
        entt::entity puppet;
        // The account we are using the permissions of.
        int64_t account;
        int64_t adminLevel;

        // This is a map of all the connections that are currently linked to this session.
        std::unordered_map<int64_t, std::shared_ptr<Connection>> clients;

        std::chrono::steady_clock::time_point lastActivity{};
        std::chrono::system_clock::time_point created{};
        std::list<std::string> inputQueue;
        std::string outText;
        int totalConnections{0};
    };

    extern std::function<std::shared_ptr<Session>(ObjectId, int64_t, entt::entity)> makeSession;
    std::shared_ptr<Session> defaultMakeSession(ObjectId id, int64_t, entt::entity character);

    extern std::unordered_map<ObjectId, std::shared_ptr<Session>> sessions;

}