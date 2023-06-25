#pragma once

#include "core/base.h"
#include "core/link.h"

namespace core {

    enum class Protocol : uint8_t {
        Telnet = 0,
        WebSocket = 1
    };

    struct ProtocolCapabilities {
        Protocol protocol{Protocol::Telnet};
        bool encryption = false;
        std::string clientName = "UNKNOWN", clientVersion = "UNKNOWN";
        std::string hostAddress = "UNKNOWN";
        int16_t hostPort{0};
        std::vector<std::string> hostNames{};
        std::string encoding;
        bool utf8 = false;
        ColorType colorType = ColorType::NoColor;
        int width = 80, height = 52;
        bool gmcp = false, msdp = false, mssp = false, mxp = false;
        bool mccp2 = false, mccp2_active = false, mccp3 = false, mccp3_active = false;
        bool ttype = false, naws = true, sga = true, linemode = false;
        bool force_endline = false, oob = false, tls = false;
        bool screen_reader = false, mouse_tracking = false, vt100 = false;
        bool osc_color_palette = false, proxy = false, mnes = false;
        void deserialize(const nlohmann::json &j);
    };

    struct Message {
        std::string cmd;
        nlohmann::json args = nlohmann::json::array();
        nlohmann::json kwargs = nlohmann::json::object();
    };

    class Connection : public std::enable_shared_from_this<Connection> {
    public:
        Connection(int64_t connId, JsonChannel chan) : connId(connId), fromLink(std::move(chan)) {}
        virtual void sendMessage(const Message &msg);
        virtual void sendText(const std::string &messg);
        virtual void onNetworkDisconnected();
        [[nodiscard]] ProtocolCapabilities& getCapabilities() {return capabilities;};
        [[nodiscard]] int64_t getConnId() const { return connId;};
        virtual void onWelcome();
        virtual void onHeartbeat(double deltaTime);
        virtual void handleText(const std::string& text);
        virtual void handleConnectCommand(const std::string& text);
        virtual void handleBadMatch(const std::string& text, std::unordered_map<std::string, std::string>& matches);
        virtual OpResult<int64_t> createAccount(std::string_view userName, std::string_view password);
        virtual void onCreateAccount(std::string_view userName, std::string_view password, int64_t acc);
        virtual OpResult<> handleLogin(const std::string &userName, const std::string &password);
        virtual void loginToAccount(int64_t acc);
        virtual void onLogin();
        virtual void handleLoginCommand(const std::string& text);
        virtual void createOrJoinSession(entt::entity ent);
        virtual void displayAccountMenu();
        [[nodiscard]] int64_t getAccount() const { return account; };
        int64_t getAdminLevel() const { return adminLevel; };
        JsonChannel& getChannel() { return fromLink; }

    protected:
        int64_t connId;
        int64_t account{-1};
        int64_t adminLevel{0};
        std::shared_ptr<Session> session;
        std::list<std::string> pendingCommands;

        // Some time structs to handle when we received connections.
        // These probably need some updating on this and Thermite side...
        std::chrono::system_clock::time_point connected{};
        std::chrono::steady_clock::time_point connectedSteady{}, lastActivity{}, lastMsg{};

        // This is embedded for ease of segmentation but this struct isn't
        // actually used anywhere else.
        ProtocolCapabilities capabilities;

        JsonChannel fromLink;

    };

    extern std::shared_ptr<Connection> defaultMakeConnection(int64_t conn_id, JsonChannel chan);
    extern std::function<std::shared_ptr<Connection>(int64_t, JsonChannel)> makeConnection;
    extern std::unordered_map<int64_t, std::shared_ptr<Connection>> connections;
    extern std::set<int64_t> pendingConnections, deadConnections;
    // I'm gonna need a mutex for each of those sets, aren't I?
    extern std::mutex connectionsMutex, pendingConnectionsMutex, deadConnectionsMutex;

    extern std::unordered_map<std::string, std::set<int64_t>> accountsCreatedRecently;
    extern std::vector<std::function<OpResult<>(std::string_view, ObjectId)>> playerCharacterNameValidators;
    extern std::vector<std::function<OpResult<>(std::string_view, int64_t)>> accountUsernameValidators;
    OpResult<> validateAccountUsername(std::string_view username, int64_t id);
    OpResult<> validatePlayerCharacterName(std::string_view name, ObjectId id);
    OpResult<int64_t> createAccount(std::string_view username, std::string_view password);

}