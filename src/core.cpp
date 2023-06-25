#include "core/core.h"
#include "core/config.h"
#include "core/link.h"
#include "core/game.h"
#include "sodium.h"

namespace core {

    std::vector<std::function<async<void>()>> services;

    void defaultSetupLogger() {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::info); // Set the console to output info level and above messages
        console_sink->set_pattern("[%^%l%$] %v"); // Example pattern: [INFO] some message

        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logfile", 1024 * 1024 * 5, 3);
        file_sink->set_level(spdlog::level::trace); // Set the file to output all levels of messages

        std::vector<spdlog::sink_ptr> sinks {console_sink, file_sink};

        logger = std::make_shared<spdlog::logger>("logger", begin(sinks), end(sinks));
        logger->set_level(spdlog::level::trace); // Set the logger to trace level
        spdlog::register_logger(logger);
    }

    std::function<void()> setupLogger(defaultSetupLogger);

    void defaultSetup() {
        try {
            std::cout << "Attempting to setup logger..." << std::endl;
            setupLogger();
        } catch (const spdlog::spdlog_ex& ex) {
            std::cout << "CRITICAL ERROR: Failed to setup logger: " << ex.what() << std::endl;
            shutdown(EXIT_FAILURE);
        } catch(...) {
            std::cout << "CRITICAL ERROR: Failed to setup logger: Unknown exception" << std::endl;
            shutdown(EXIT_FAILURE);
        }
        logger->info("Logger setup successfully");

        logger->info("Initializing libsodium...");
        if (sodium_init() == -1) {
            logger->critical("Could not initialize libsodium!");
            shutdown(EXIT_FAILURE);
        }

        logger->info("Setting up executor...");
        executor = std::make_unique<boost::asio::io_context>();

        // Next, we need to create the config::thermiteEndpoint from config::thermiteAddress and config::thermitePort
        logger->info("Setting up thermite endpoint...");
        try {
            config::thermiteEndpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(config::thermiteAddress), config::thermitePort);
        } catch (const boost::system::system_error& ex) {
            logger->critical("Failed to create thermite endpoint: {}", ex.what());
            shutdown(EXIT_FAILURE);
        } catch(...) {
            logger->critical("Failed to create thermite endpoint: Unknown exception");
            shutdown(EXIT_FAILURE);
        }

        logger->info("Setting up link manager...");
        linkManager = std::make_unique<LinkManager>();

    }
    std::function<void()> setup(defaultSetup);

    void defaultStartup() {
        logger->info("Beginning startup sequence...");

        // use config::thermiteAddress and config::thermitePort to intitialize config::thermiteEndpoint.
        logger->info("Initializing thermite endpoint...");
        try {
            config::thermiteEndpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(config::thermiteAddress), config::thermitePort);
        } catch (const boost::system::system_error& ex) {
            logger->critical("Failed to create thermite endpoint: {}", ex.what());
            shutdown(EXIT_FAILURE);
        } catch(...) {
            logger->critical("Failed to create thermite endpoint: Unknown exception");
            shutdown(EXIT_FAILURE);
        }

        logger->info("Starting the LinkManager...");
        boost::asio::co_spawn(boost::asio::make_strand(*executor), linkManager->run(), boost::asio::detached);

        logger->info("Preparing the game loop...");
        boost::asio::co_spawn(boost::asio::make_strand(*executor), game(), boost::asio::detached);
        
        // Co_spawn each service on a strand
        logger->info("Starting services...");
        for (auto &service: services) {
            boost::asio::co_spawn(boost::asio::make_strand(*executor), service(), boost::asio::detached);
        }

        // Run the io_context
        logger->info("Entering main loop...");
        // This part is a little tricky. if config::enableMultithreading is true, want to
        // run the io_context on multiple threads. Otherwise, we want to run it on a single thread.
        // Additionally, if it's true, we need to check config::threadsCount to see how many threads
        // to run on. If it's <1, we want to run on the number of cores on the machine.
        // The current thread should, of course, be considered one of those threads, so we subtract 1.
        // The best way to do this is to simply create a vector of threads, decide how many it should contain,
        // and start them. Then, we run the io_context on the current thread.

        unsigned int threadCount = 0;
        if(config::enableMultithreading) {
            logger->info("Multithreading is enabled.");
            if(config::threadsCount < 1) {
                threadCount = std::thread::hardware_concurrency() - 1;
                logger->info("Using {} threads. (Automatic detection)", threadCount+1);
            } else {
                threadCount = config::threadsCount - 1;
                logger->info("Using {} threads. (Manual override)", threadCount+1);
            }
        } else {
            threadCount = 0;
        }

        std::vector<std::thread> threads;
        if(threadCount) {
            logger->info("Starting {} threads...", threadCount);
            config::usingMultithreading = true;
        }
        for (int i = 0; i < threadCount; ++i) {
            threads.emplace_back([&]() {
                executor->run();
            });
        }
        logger->info("Main thread running executor...");
        executor->run();
        logger->info("Main loop exited.");

        if(threadCount) {
            // Join all threads.
            // This should happen without any incident, since the only thing they're doing is
            // the io_executor which has been stopped.
            logger->info("Joining threads...");
            for (auto &thread: threads) {
                thread.join();
            }
            logger->info("All threads joined.");
            threads.clear();
        }

        switch(gameLoopStatus) {
            case GameLoop::Running:
                logger->critical("PANIC: Executor terminated with GameLoop RUNNING.");
                shutdown(EXIT_FAILURE);
                break;
            case GameLoop::Shutdown:
                logger->info("Game Loop requested a shutdown.");
                handleGameShutdown();
                break;
            case GameLoop::Restart:
                logger->info("Game loop requested a restart.");
                handleGameRestart();
                break;
        }

        // This shouldn't happen, but just in case...
        logger->critical("PANIC: Executor terminated without GameLoop status.");
        shutdown(EXIT_FAILURE);

    }
    std::function<void()> startup(defaultStartup);

    void defaultShutdown(int exitCode) {
        std::cout << "Process exiting with exit code " << exitCode << std::endl;
        std::exit(exitCode);
    }

    std::function<void(int)> shutdown(defaultShutdown);

    void defaultHandleGameShutdown() {
        logger->info("Shutting down...");
        shutdown(EXIT_SUCCESS);
    }
    std::function<void()> handleGameShutdown(defaultHandleGameShutdown);

    void defaultHandleGameRestart() {
        logger->info("Restart feature not implemented, shutting down...");
        shutdown(EXIT_SUCCESS);
    }
    std::function<void()> handleGameRestart(defaultHandleGameRestart);

}