#include "core/link.h"
#include "core/connection.h"
#include "core/config.h"

namespace core {

    std::unique_ptr<LinkManager> linkManager;
    std::unique_ptr<Link> link;

    LinkManager::LinkManager() : linkChan(*executor), is_stopped(false) {}

    async<void> LinkManager::run() {
        bool do_standoff = false;
        boost::asio::steady_timer standoff(co_await boost::asio::this_coro::executor);
        while (!is_stopped) {
            if(do_standoff) {
                standoff.expires_after(5s);
                co_await standoff.async_wait(boost::asio::use_awaitable);
                do_standoff = false;
            }
            try {
                logger->info("LinkManager: Connecting to {}:{}...", endpoint.address().to_string(), endpoint.port());
                // Create a new TCP socket
                boost::beast::websocket::stream<boost::beast::tcp_stream> ws(co_await boost::asio::this_coro::executor);

                // Connect to the endpoint
                co_await boost::beast::get_lowest_layer(ws).async_connect(endpoint, boost::asio::use_awaitable);
                // Initialize a WebSocket using the connected socket

                // Perform the WebSocket handshake
                co_await ws.async_handshake(endpoint.address().to_string() + ":" + std::to_string(endpoint.port()), "/", boost::asio::use_awaitable);

                // Construct a Link using the WebSocket
                link = std::make_unique<Link>(std::move(ws));

                // Run the Link
                logger->info("LinkManager: Link established! Running Link...");
                co_await link->run();
                link.reset();

            } catch (const boost::system::system_error& error) {
                // If there was an error, handle it (e.g., log the error message)
                logger->error("Error in LinkManager::run(): {}", error.what());

                // You might want to add a delay before attempting to reconnect, e.g.,
                do_standoff = true;
            }
        }
        co_return;
    }

    void LinkManager::stop() {
        is_stopped = true;
    }

    Link::Link(boost::beast::websocket::stream<boost::beast::tcp_stream> ws)
            : conn(std::move(ws)), is_stopped(false) {}

    boost::asio::awaitable<void> Link::run() {
        try {
            co_await (runReader() || runWriter());
        } catch (const boost::system::system_error& e) {
            logger->error("Error in Link::run(): {}", e.what());
            // Handle exceptions here if necessary
        } catch(...) {
            logger->error("Unknown error in Link::run()");
        }
        if(conn.is_open()) {
            try {
                conn.close(boost::beast::websocket::close_code::normal);
            } catch(...) {
                logger->error("Error closing WebSocket gracefully...");
            }
        }
        co_return;
    }

    void Link::stop() {
        is_stopped = true;
    }

    async<void> Link::createUpdateClient(const nlohmann::json &j) {
        auto id = j["id"].get<int64_t>();
        auto addr = j["addr"].get<std::string>();
        const auto& capabilities = j["capabilities"];

        // Do something with id, addr, and capabilities
        auto it = connections.find(id);
        if (it == connections.end()) {
            // Create a new ClientConnection
            JsonChannel clientChan(co_await boost::asio::this_coro::executor, 50);
            auto cc = makeConnection(id, std::move(clientChan));
            cc->capabilities.deserialize(capabilities);
            if(config::usingMultithreading) {
                std::lock_guard<std::mutex> lock(connectionsMutex);
                connections[id] = cc;
            } else connections[id] = cc;

            if(config::usingMultithreading) {
                std::lock_guard<std::mutex> lock(pendingConnectionsMutex);
                pendingConnections.insert(id);
            } else pendingConnections.insert(id);
        } else {
            // Update the existing ClientConnection
            auto& client_connection = it->second;
            client_connection->capabilities.deserialize(capabilities);
        }
        co_return;
    }

    boost::asio::awaitable<void> Link::runReader() {
        while (true) {
            try {
                // Read a message from the WebSocket
                boost::beast::flat_buffer buffer;
                co_await conn.async_read(buffer, boost::asio::use_awaitable);

                // Deserialize the JSON string
                auto ws_str = boost::beast::buffers_to_string(buffer.data());
                //std::cout << "Received: " << ws_str << std::endl;
                auto j = nlohmann::json::parse(ws_str);


                // Access the "kind" field in the JSON object
                std::string kind = j["kind"];

                // Implement your routing logic here

                if (kind == "client_list") {
                    // This message is sent by Thermite when the game establishes a fresh connection with it.
                    // It should be the first thing a Link sees.
                    logger->info("Link: Received client_list message");
                    // Get the "data" object
                    auto &data = j["data"];

                    // Iterate over the contents of the "data" object
                    for (const auto &entry : data) {
                        co_await createUpdateClient(entry);
                    }

                } else if (kind == "client_ready") {
                    // This message is sent by Thermite when a new client has connected.
                    auto &data = j["protocol"];
                    co_await createUpdateClient(data);

                } else {
                    // Extract the "id" field from the JSON object
                    int64_t id = j["id"];

                    // Look up the specific ClientConnection in the std::map
                    auto it = connections.find(id);
                    if(it == connections.end()) {
                        logger->info("Link: Received message for unknown client: {}", id);
                        continue;
                    }

                    // Found the client connection
                    auto &client_connection = it->second;

                    if (kind == "client_capabilities") {
                        auto &capabilities = j["capabilities"];
                        client_connection->capabilities.deserialize(capabilities);

                    } else if (kind == "client_data") {
                        try {
                            co_await client_connection->fromLink.async_send(boost::system::error_code{}, j, boost::asio::use_awaitable);
                        } catch (const boost::system::system_error &e) {
                            // Handle exceptions (e.g., WebSocket close or error)
                        }

                    } else if (kind == "client_disconnected") {
                        logger->info("Link: Received client_disconnected message for client: {}", id);
                        deadConnections.insert(id);
                    }
                }
            } catch (const boost::system::system_error &e) {
                logger->error("Link RunReader flopped at: {}", e.what());
                // Handle exceptions (e.g., WebSocket close or error)
            } catch (...) {
                logger->error("Unknown error in Link RunReader");
            }
        }
        co_return;
    }

    boost::asio::awaitable<void> Link::runWriter() {
        while (true) {
            try {
                // Receive a message from the channel asynchronously
                auto message = co_await linkManager->linkChan.async_receive(boost::asio::use_awaitable);

                try {
                    // Serialize the JSON message to text
                    std::string serialized_msg = message.dump(-1, ' ', false, nlohmann::json::error_handler_t::ignore);

                    // Send the message across the WebSocket
                    co_await conn.async_write(boost::asio::buffer(serialized_msg), boost::asio::use_awaitable);
                } catch (const boost::system::system_error& e) {
                    logger->error("Link runWriter flopped 1: {}", e.what());
                    // Handle exceptions (e.g., WebSocket close or error) when sending the message
                } catch (...) {
                    logger->error("Unknown error in Link runWriter 1");
                }
            } catch (const boost::system::system_error& e) {
                logger->error("Link runWriter flopped 2: {}", e.what());
                // Handle exceptions (e.g., error receiving the message from the channel)
            } catch (...) {
                logger->error("Unknown error in Link runWriter 2");
            }
        }
        co_return;
    }

    std::shared_ptr<Connection> defaultMakeConnection(int64_t conn_id, JsonChannel chan) {
        return std::make_shared<Connection>(conn_id, std::move(chan));
    }
    std::function<std::shared_ptr<Connection>(int64_t, JsonChannel)> makeConnection(defaultMakeConnection);


}