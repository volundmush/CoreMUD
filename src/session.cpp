#include "core/session.h"
#include "core/connection.h"

namespace core {

    Session::Session(core::ObjectId id, int64_t account, entt::entity character) {
        this->id = id;
        this->account = account;
        this->character = character;
        created = std::chrono::system_clock::now();
        lastActivity = std::chrono::steady_clock::now();
    }

    void Session::sendLine(const std::string &txt) {
        if(!connections.empty()) {
            if(txt.ends_with("\n")) {
                sendText(txt);
            } else {
                sendText(txt + "\n");
            }
        }
    }

    void Session::sendText(const std::string &txt) {
        outText += txt;
    }

    void Session::send(const Message &msg) {

    }

    void Session::atObjectDeleted(entt::entity ent) {

    }

    void Session::changePuppet(entt::entity ent) {
        puppet = ent;
    }

    void Session::handleText(const std::string &text) {
        lastActivity = std::chrono::steady_clock::now();
        if(text == "--") {
            // clear the queue.
            inputQueue.clear();
            sendText("Your input queue has been cleared of all pending commands.\n");
            return;
        }
        inputQueue.push_back(text);
    }

    void Session::onNetworkDisconnected(int64_t connId) {

        auto conn = clients.find(connId);
        if (conn != clients.end()) {
            removeConnection(conn->second);
        }
        if(clients.empty()) {
            onLinkDead();
        }
    }

    void Session::onLinkDead() {
        // Should the last remaining session be removed due to loss of a network connection,
        // we should declare the character link-dead and begin a process of removal from the world.
        // This may need to operate as some sort of timer, so that if the player reconnects within
        // the limit they can rejoin their Session, and to avoid potential abuse of any link-dead
        // mechanics.
    }

    void Session::addConnection(int64_t connID) {
        auto found = connections.find(connID);
        if (found != connections.end()) {
            addConnection(found->second);
        }
    }

    void Session::addConnection(const std::shared_ptr<Connection> &conn) {
        clients[conn->getConnId()] = conn;
        onAddConnection(conn);
        totalConnections++;
    }

    void Session::onAddConnection(const std::shared_ptr<Connection>& conn) {
        // If we have only one connection then we need to launch a special hook.
        if(totalConnections == 0) {
            onFirstConnection();
        }
    }

    void Session::onFirstConnection() {
        start();
    }

    void Session::removeConnection(int64_t connID) {
        auto found = clients.find(connID);
        if (found != clients.end()) {
            removeConnection(found->second);
        }
    }

    void Session::removeConnection(const std::shared_ptr<Connection> &conn) {
        clients.erase(conn->getConnId());
        onRemoveConnection(conn);
    }

    void Session::removeAllConnections() {
        auto tempClients = clients;
        for (auto& [clientId, conn] : tempClients) {
            removeConnection(conn);
        }
    }

    void Session::onRemoveConnection(const std::shared_ptr<Connection>& conn) {

    }

    void Session::onHeartbeat(double deltaTime) {
        /*
        if(inputQueue.empty()) {
            return;
        }
        auto& input = inputQueue.front();
        auto &cmd = registry.get_or_emplace<PendingCommand>(puppet);
        cmd.input = input;
        // Now we remove input from inputQueue.
        inputQueue.pop_front();
        */
    }

    void Session::sendOutput(double deltaTime) {
        // This will later need to handle prompts.
        if (!outText.empty()) {
            for(auto &[cid, c] : clients) {
                c->sendText(outText);
            }
            outText.clear();
        }
    }

    void Session::start() {

    }

    void Session::end() {

    }

    std::shared_ptr<Session> defaultMakeSession(ObjectId id, int64_t, entt::entity character) {
        return std::make_shared<Session>(id, 0, character);
    }
    std::function<std::shared_ptr<Session>(ObjectId, int64_t, entt::entity)> makeSession = defaultMakeSession;

    std::unordered_map<ObjectId, std::shared_ptr<Session>> sessions;
}