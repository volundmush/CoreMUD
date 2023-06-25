#include "core/connection.h"
#include "core/color.h"
#include "core/commands.h"
#include "core/session.h"
#include "core/database.h"
#include "core/api.h"
#include "core/components.h"

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
        j["id"] = this->connId;

        d["cmd"] = msg.cmd;
        d["args"] = msg.args;
        d["kwargs"] = msg.kwargs;

        j["data"].push_back(d);

        linkManager->linkChan.try_send(boost::system::error_code{}, j);
    }


    void Connection::sendText(const std::string &text) {
        Message m;
        m.cmd = "text";
        m.args = {renderAnsi(text, capabilities.colorType)};
        sendMessage(m);
    }

    void Connection::onNetworkDisconnected() {
        if(session) {
            session->removeConnection(connId);
        }
    }

    void Connection::handleBadMatch(const std::string& text, std::unordered_map<std::string, std::string>& matches) {
        sendText("Sorry, that's not a command.\r\n");
    }

    void Connection::handleConnectCommand(const std::string& text) {
        auto match_map = parseCommand(text);

        if(match_map.empty()) {
            handleBadMatch(text, match_map);
            return;
        }
        auto self = std::static_pointer_cast<Connection>(shared_from_this());
        auto ckey = match_map["cmd"];
        for(auto &[key, cmd] : expandedConnectCommandRegistry) {
            if(!cmd->isAvailable(self))
                continue;
            if(boost::iequals(ckey, key)) {
                auto [can, err] = cmd->canExecute(self, match_map);
                if(!can) {
                    sendText(fmt::format("Sorry, you can't do that: {}\r\n", err.value()));
                    return;
                }
                cmd->execute(self, match_map);
                return;
            }
        }
        handleBadMatch(text, match_map);
    }

    void Connection::handleLoginCommand(const std::string& text) {
        auto match_map = parseCommand(text);
        if(match_map.empty()) {
            handleBadMatch(text, match_map);
            return;
        }
        auto self = std::static_pointer_cast<Connection>(shared_from_this());
        auto ckey = match_map["cmd"];
        for(auto &[key, cmd] : expandedLoginCommandRegistry) {
            if(!cmd->isAvailable(self))
                continue;
            if(boost::iequals(ckey, key)) {
                auto [can, err] = cmd->canExecute(self, match_map);
                if(!can) {
                    sendText(fmt::format("Sorry, you can't do that: {}\r\n", err.value()));
                    return;
                }
                cmd->execute(self, match_map);
                return;
            }
        }
        handleBadMatch(text, match_map);
    }

    void Connection::handleText(const std::string &str) {
        // the IDLE command should be intercepted here.
        // It's a convention which allows clients behind wonky NAS
        // to keep their connections alive.
        if(str == "IDLE") {
            return;
        }

        // If we have a connected session, then we relay this to it. Else,
        // we treat it as a login Command.

        if(session)
            session->handleText(str);
        else if(account != -1)
            handleLoginCommand(str);
        else
            handleConnectCommand(str);
    }

    void Connection::onHeartbeat(double deltaTime) {
        // Every time the heartbeat runs, we want to pull everything out of fromLink
        // first of all.

        // We need to do this in a loop, because we may have multiple messages in the
        // channel.

        std::error_code ec;
        nlohmann::json value;
        while (fromLink.ready()) {
            if(!fromLink.try_receive([&](std::error_code ec2, nlohmann::json value2) {
                ec = ec2;
                value = value2;
            })) {
                break;
            }
            if (ec) {
                // TODO: Handle any errors..
            } else {
                // If we got a json value, it should look like this.
                // {"kind": "client_data", "id": 123, "data": [{"cmd": "text", "args": ["hello world!"], "kwargs": {}}]}
                // We're only interested in iterating over the contents of the "data" array.
                // Now we must for-each over the contents of jarr, extract the cmd, args, and kwargs data, and call
                // the appropriate handle routine.
                logger->info("Received data from link: {}", value.dump());
                for (auto &jval : value["data"]) {
                    logger->info("Processing data from link: {}", jval.dump());
                    std::string cmd = jval["cmd"];
                    nlohmann::json jargs = jval["args"];
                    nlohmann::json jkwargs = jval["kwargs"];

                    if (cmd == "text") {
                        // Treat each argument in jargs as a separate string.
                        // We need to for-each jargs and handleText(each).

                        for (auto &jarg : jargs) {
                            std::string text = jarg;
                            handleText(text);
                        }

                    } else if (cmd == "mssp") {
                        // We should never be receiving mssp like this.
                    } else {
                        // Anything that's not text or mssp is generic GMCP data.
                        // TODO: handle GMCP.
                    }
                }
                lastActivity = std::chrono::steady_clock::now();
            }
        }
    }

    OpResult<int64_t> Connection::createAccount(std::string_view userName, std::string_view password) {
        // First, get the IP without the port from this->capabilities.
        // Then, get the set of accounts created recently for that IP.

        auto &created = accountsCreatedRecently[this->capabilities.hostAddress];
        // If the set is empty, then we can create an account.
        if (!created.empty())
            return {-1, "You have created too many accounts recently. Please try again later."};

        auto [ent, err] = core::createAccount(userName, password);
        if(err.has_value())
            return {-1, err};

        created.insert(ent);

        onCreateAccount(userName, password, ent);
        return {ent, std::nullopt};
    }

    void Connection::onCreateAccount(std::string_view userName, std::string_view password, int64_t acc) {
        sendText(fmt::format("Account created successfully. Welcome, {}!\r\n", userName));
        sendText(fmt::format("Please keep your password safe and secure. If you forget it, contact staff.\r\n"));
        loginToAccount(acc);
    }

    void Connection::loginToAccount(int64_t acc) {
        account = acc;
        onLogin();
    }

    void Connection::onLogin() {
        SQLite::Statement q(*db, "UPDATE accounts SET lastLogin = datetime('now') WHERE id = ?");
        q.bind(1, account);
        q.exec();

        SQLite::Statement q2(*db, "SELECT username,adminLevel FROM accounts WHERE id = ?");
        q2.bind(1, account);
        std::string name;
        while(q2.executeStep()) {
            name = q2.getColumn(0).getString();
            adminLevel = q2.getColumn(1).getInt();
        }

        sendText(fmt::format("Welcome back, {}!\r\n", name));
        displayAccountMenu();
    }

    void Connection::onWelcome() {
        sendText("Welcome to the game!\r\n");
    }

    void Connection::displayAccountMenu() {
        std::string username, email;
        int level;
        SQLite::Statement q(*db, "SELECT username, email, adminLevel FROM accounts WHERE id = ?");
        q.bind(1, account);
        while(q.executeStep()) {
            username = q.getColumn(0).getString();
            email = q.getColumn(1).getString();
            level = q.getColumn(2).getInt();
        }
        sendText("                 @RAccount Menu@n\n");
        sendText("=============================================\n");
        sendText(fmt::format("|@g{:<14}@n:  {:<27}|\n", "Username", username));
        if(!email.empty())
            sendText(fmt::format("|@g{:<14}@n:  {:<27}|\n", "Email Address", email));
        if(level > 0)
            sendText(fmt::format("|@g{:<14}@n:  {:<27}|\n", "Admin Level", level));
        sendText("=============================================\n\n");

        std::vector<ObjectId> characters;
        SQLite::Statement q2(*db, "SELECT character,lastLogin,lastLogout,totalPlayTime FROM playerCharacters WHERE account = ?");
        q2.bind(1, account);
        while(q2.executeStep()) {
            auto character = q2.getColumn(0).getInt64();
            auto lastLogin = q2.getColumn(1).getString();
            auto lastLogout = q2.getColumn(2).getString();
            auto totalPlayTime = q2.getColumn(3).getString();

            characters.emplace_back(character, objects[character].first);
        }

        if(!characters.empty()) {
            sendText("[@y----@YAvailable Characters@y----@n]\n");
            for(auto c : characters) {
                auto ent = c.getObject();
                if(!registry.valid(ent)) continue;
                sendText(getName(ent) + "\n");
            }
        }

        sendText("\n[@y----@YCommands@y----@n]\n");
        sendText("@Wcreate <character>@n - Create a new character.\n");
        sendText("@Wplay <character>@n - Log in to a character.\n");
        sendText("@Wdelete <character>@n - Delete a character.\n");
        sendText("@Wpassword <old password>=<new password>@n - Change your password.\n");
        sendText("@Wemail <email address>@n - Change your email address.\n");
        sendText("@Wquit@n - Quit the game.\n");
    }

    OpResult<> Connection::handleLogin(const std::string &userName, const std::string &password) {
        SQLite::Statement q(*db, "SELECT id,password FROM accounts WHERE username = ?");
        q.bind(1, userName);
        while(q.executeStep()) {
            auto id = q.getColumn(0).getInt64();
            auto pass = q.getColumn(1).getString();
            auto [check, err] = core::checkPassword(pass, password);
            if(!check) {
                return {false, err};
            }
            loginToAccount(id);
            return {true, std::nullopt};
        }
        return {false, "No such account.\n"};
    }

    void Connection::createOrJoinSession(entt::entity ent) {
        auto sessholder = registry.try_get<SessionHolder>(ent);

        if(sessholder) {
            // the character already has a session, so we need to join this ClientConnection
            // to it.
            session = sessholder->data;
        } else {
            auto id = getObjectId(ent);
            // The character has no session, so we need to create one and join the clientConnection to it.
            session = makeSession(id, account, ent);
            sessions[id] = session;
            auto &sholder = registry.emplace<SessionHolder>(ent);
            sholder.data = session;
            sholder.sessionMode = 1;
        }
        session->addConnection(shared_from_this());
    }

    std::unordered_map<std::string, std::set<int64_t>> accountsCreatedRecently;

    // Validator functions for checking whether a new/renamed account/character string is valid.
    // For generating new accounts/characters, set ent = entt::null. For renames, enter the target's entt::entity.
    std::vector<std::function<OpResult<>(std::string_view, ObjectId)>> playerCharacterNameValidators;
    std::vector<std::function<OpResult<>(std::string_view, int64_t)>> accountUsernameValidators;
    OpResult<> validateAccountUsername(std::string_view username, int64_t id) {
        for (auto &validator : accountUsernameValidators) {
            auto [res, err] = validator(username, id);
            if (!res) return {res, err};
        }
        return {true, std::nullopt};
    }

    OpResult<> validatePlayerCharacterName(std::string_view name, ObjectId id) {
        for (auto &validator : playerCharacterNameValidators) {
            auto [res, err] = validator(name, id);
            if (!res) return {res, err};
        }
        return {true, std::nullopt};
    }

    OpResult<int64_t> createAccount(std::string_view username, std::string_view password) {
        auto [res, err] = validateAccountUsername(username, -1);
        if (!res) return {-1, err};
        auto [res2, err2] = hashPassword(password);
        if (!res2) return {-1, err2};

        SQLite::Statement q(*db, "INSERT INTO accounts (username, password) VALUES (?, ?)");
        q.bind(1, std::string(username));
        q.bind(2, err2.value());
        q.exec();
        auto id = db->getLastInsertRowid();
        return {id, std::nullopt};
    }
}