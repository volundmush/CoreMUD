#include "core/game.h"
#include "core/system.h"
#include "core/config.h"
#include "core/core.h"
#include "core/link.h"
#include "core/database.h"

namespace core {
    std::vector<std::function<async<void>()>> gameStartupFuncs;
    std::vector<std::function<async<void>(double)>> preHeartbeatFuncs, postHeartbeatFuncs;
    async<void> defaultGame() {
        logger->info("Starting game");
        logger->info("Sorting systems...");
        sortSystems();
        if(sortedSystems.empty()) {
            logger->warn("No systems registered.");
        }

        logger->info("Running game startup functions...");
        for(auto& func : gameStartupFuncs) {
            co_await func();
        }

        co_await run();
        logger->info("Exited game run()...");

        linkManager->stop();



        co_return;
    }
    std::function<async<void>()> game(defaultGame);

    GameLoop gameLoopStatus{GameLoop::Running};

    async<void> defaultRun() {
        try {
            auto startdb = std::chrono::high_resolution_clock::now();
            loadDatabase();
            auto enddb = std::chrono::high_resolution_clock::now();
            broadcast(fmt::format("Database loaded in {:.3f} seconds", std::chrono::duration<double>(enddb - startdb).count()));
        }
        catch(std::exception& e) {
            logger->critical("Exception while loading database: {}", e.what());
            shutdown(EXIT_FAILURE);
        }

        auto previousTime = boost::asio::steady_timer::clock_type::now();
        boost::asio::steady_timer timer(co_await boost::asio::this_coro::executor, config::heartbeatInterval);
        broadcast("Let the games begin!");
        while(gameLoopStatus == GameLoop::Running) {
            auto timeStart = boost::asio::steady_timer::clock_type::now();
            co_await timer.async_wait(boost::asio::use_awaitable);
            auto timeEnd = boost::asio::steady_timer::clock_type::now();

            auto deltaTime = timeEnd - timeStart;
            double deltaTimeInSeconds = std::chrono::duration<double>(deltaTime).count();

            try {
                for(auto& func : preHeartbeatFuncs) {
                    co_await func(deltaTimeInSeconds);
                }
                co_await heartbeat(deltaTimeInSeconds);
                for(auto& func : postHeartbeatFuncs) {
                    co_await func(deltaTimeInSeconds);
                }
            } catch(std::exception& e) {
                logger->critical("Exception during heartbeat: {}", e.what());
                broadcast("Critical error detected in game simulation, commencing emergency shutdown!");
                shutdown(EXIT_FAILURE);
            }

            auto timeAfterHeartbeat = boost::asio::steady_timer::clock_type::now();
            auto elapsed = timeAfterHeartbeat - timeStart;
            auto nextWait = config::heartbeatInterval - elapsed;

            // If heartbeat takes more than 100ms, default to a very short wait
            if(nextWait.count() < 0) {
                nextWait = std::chrono::milliseconds(1);
            }

            timer.expires_from_now(nextWait);
        }

        switch(gameLoopStatus) {
            case GameLoop::Shutdown:
                logger->info("Shutting down...");
                co_await gameShutdown();
                break;
            case GameLoop::Restart:
                logger->info("Restarting...");
                co_await gameRestart();
                break;
        }

        co_return;
    }
    std::function<async<void>()> run(defaultRun);

    async<void> defaultHeartbeat(double deltaTime) {
        for(auto &sys : sortedSystems) {
            if(co_await sys->shouldRun(deltaTime))
                co_await sys->run(deltaTime);
        }
        co_return;
    }
    std::function<async<void>(double)> heartbeat(defaultHeartbeat);

    async<void> defaultGameShutdown() {
        logger->info("Shutting down...");
        co_return;
    }
    std::function<async<void>()> gameShutdown(defaultGameShutdown);

    async<void> defaultGameRestart() {
        logger->info("Restarting...");
        co_return;
    }
    std::function<async<void>()> gameRestart(defaultGameRestart);

}