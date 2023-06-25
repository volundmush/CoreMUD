#include "core/config.h"

namespace core::config {
    bool enableMultithreading{true};
    int threadsCount{0};
    bool usingMultithreading{false};
    std::chrono::milliseconds heartbeatInterval{100ms};
    std::string thermiteAddress{"127.0.0.1"};
    uint16_t thermitePort{7000};
    boost::asio::ip::tcp::endpoint thermiteEndpoint;
    std::string dbName = "coremud.sqlite3";
}