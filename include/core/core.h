#pragma once
#include "core/base.h"

namespace core {
    extern std::function<void()> setupLogger;
    void defaultSetupLogger();

    extern std::function<void()> setup;
    void defaultSetup();

    extern std::function<void()> startup;
    void defaultStartup();

    void defaultShutdown(int exitCode);
    extern std::function<void(int)> shutdown;

    extern std::vector<std::function<async<void>()>> services;

    void defaultHandleGameShutdown();
    extern std::function<void()> handleGameShutdown;

    void defaultHandleGameRestart();
    extern std::function<void()> handleGameRestart;
}