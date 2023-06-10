#pragma once
#include "core/base.h"
#include <boost/beast/websocket.hpp>
#include <boost/beast.hpp>


namespace core {

    // This class manages a repeating connector to Thermite. It is launched via co_spawn
    // using run() as its entry point, and it will connect to Thermite using the provided
    // endpoint. If the connection is established then it will create a Link object
    // and pass control to that Link object's run() via co_await. If the connection drops,
    // it will attempt to reconnect endlessly until it re-establishes connection.
    class LinkManager {
    public:
        JsonChannel linkChan;

        LinkManager();
        async<void> run();
        void stop();

    protected:
        boost::asio::ip::tcp::endpoint endpoint;
        bool is_stopped;
    };

    extern std::unique_ptr<LinkManager> linkManager;


    // this class is a link to the Thermite server, which accepts a WebSocket connection.
    // It is responsible for communicating with Thermite to handle incoming client events and
    // to relay game events to the client.
    class Link {
    public:
        explicit Link(boost::beast::websocket::stream<boost::beast::tcp_stream> ws);

        async<void> run();
        void stop();

    protected:
        async<void> runReader();
        async<void> runWriter();
        async<void> createUpdateClient(const nlohmann::json &j);
        boost::beast::websocket::stream<boost::beast::tcp_stream> conn;
        bool is_stopped;
    };

    extern std::unique_ptr<Link> link;



}