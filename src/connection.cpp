#include "core/connection.h"


namespace core {

    std::unordered_map<int64_t, std::shared_ptr<Connection>> connections;
    std::set<int64_t> pendingConnections, deadConnections;
    std::mutex connectionsMutex, pendingConnectionsMutex, deadConnectionsMutex;

    void ProtocolCapabilities::deserialize(const nlohmann::json &j) {

        std::string protocol_str = j["protocol"];
        if (protocol_str == "Telnet") {
            protocol = Protocol::Telnet;
        } else if (protocol_str == "WebSocket") {
            protocol = Protocol::WebSocket;
        }

        encryption = j["encryption"];
        clientName = j["client_name"];
        clientVersion = j["client_version"];
        hostAddress = j["host_address"];
        hostPort = j["host_port"];
        for(auto &hn : j["host_names"]) {
            hostNames.emplace_back(hn);
        }
        encoding = j["encoding"];
        utf8 = j["utf8"];

        std::string color_str = j["color"];
        if (color_str == "NoColor") {
            colorType = ColorType::NoColor;
        } else if (color_str == "Standard") {
            colorType = ColorType::Standard;
        } else if (color_str == "Xterm256") {
            colorType = ColorType::Xterm256;
        } else if (color_str == "TrueColor") {
            colorType = ColorType::TrueColor;
        }

        width = j["width"];
        height = j["height"];
        gmcp = j["gmcp"];
        msdp = j["msdp"];
        mssp = j["mssp"];
        mxp = j["mxp"];
        mccp2 = j["mccp2"];
        mccp3 = j["mccp3"];
        ttype = j["ttype"];
        naws = j["naws"];
        sga = j["sga"];
        linemode = j["linemode"];
        force_endline = j["force_endline"];
        oob = j["oob"];
        tls = j["tls"];
        screen_reader = j["screen_reader"];
        mouse_tracking = j["mouse_tracking"];
        vt100 = j["vt100"];
        osc_color_palette = j["osc_color_palette"];
        proxy = j["proxy"];
        mnes = j["mnes"];
    }

    void Connection::sendMessage(const core::Message &msg) {
        nlohmann::json j, d;
        j["kind"] = "client_data";
        j["id"] = this->connID;

        d["cmd"] = msg.cmd;
        d["args"] = msg.args;
        d["kwargs"] = msg.kwargs;

        j["data"].push_back(d);

        linkManager->linkChan.try_send(boost::system::error_code{}, j);
    }


    void Connection::sendText(const std::string &text) {
        Message m;
        m.cmd = "text";
        m.args = {text};
        sendMessage(m);
    }

    const ProtocolCapabilities& Connection::getCapabilities() const {
        return capabilities;
    }

    uint64_t Connection::getConnID() const {
        return connID;
    }

}