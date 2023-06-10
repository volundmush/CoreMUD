#pragma once
#include "core/base.h"

namespace core {

    enum class GameLoop {
        Running = 0,
        Shutdown = 1,
        Restart = 2
    };

    extern GameLoop gameLoopStatus;

    extern std::vector<std::function<async<void>()>> gameStartupFuncs;
    extern std::vector<std::function<async<void>(double)>> preHeartbeatFuncs, postHeartbeatFuncs;

    async<void> defaultGame();
    extern std::function<async<void>()> game;

    async<void> defaultRun();
    extern std::function<async<void>()> run;

    async<void> defaultHeartbeat(double deltaTime);
    extern std::function<async<void>(double)> heartbeat;

    async<void> defaultGameShutdown();
    extern std::function<async<void>()> gameShutdown;

    async<void> defaultGameRestart();
    extern std::function<async<void>()> gameRestart;
}