#include "core/base.h"

namespace core {
    std::unique_ptr<boost::asio::io_context> executor;
    std::shared_ptr<spdlog::logger> logger;
}