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
        // They need this PURELY for access to that darned mpsc_channel.
        friend class Link;
        friend class LinkManager;
    public:
        Connection(int64_t connID, JsonChannel chan) : connID(connID), fromLink(std::move(chan)) {}
        virtual void sendMessage(const Message &msg);
        virtual void sendText(const std::string &messg);
        [[nodiscard]] const ProtocolCapabilities& getCapabilities() const;
        [[nodiscard]] uint64_t getConnID() const;

    protected:
        int64_t connID;

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
}