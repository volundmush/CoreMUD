#pragma once

// sane C++ standard imports.
#include <cstdint>
#include <optional>
#include <chrono>
#include <string>
#include <string_view>
#include <vector>
#include <list>
#include <memory>
#include <map>
#include <set>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <utility>

// Our own libraries...
#include <boost/asio.hpp>
#include <boost/asio/experimental/concurrent_channel.hpp>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/asio/experimental/awaitable_operators.hpp>


namespace core {
    using namespace std::chrono_literals;
    using namespace boost::asio::experimental::awaitable_operators;

    class Connection;
    class Link;
    class LinkManager;

    extern std::unique_ptr<boost::asio::io_context> executor;
    extern std::shared_ptr<spdlog::logger> logger;

    template<typename T>
    using mpmc_channel = boost::asio::experimental::concurrent_channel<void(boost::system::error_code, T)>;

    using JsonChannel = mpmc_channel<nlohmann::json>;

    template<typename T>
    using async = boost::asio::awaitable<T>;

    template<typename T = bool>
    using OpResult = std::pair<T, std::optional<std::string>>;

    enum class ColorType : uint8_t {
        NoColor = 0,
        Standard = 1,
        Xterm256 = 2,
        TrueColor = 3
    };


}