#include "core/system.h"
#include "core/connection.h"
#include "core/session.h"

namespace core {

    std::vector<std::shared_ptr<System>> sortedSystems;
    std::unordered_map<std::string, std::shared_ptr<System>> systemRegistry;

    void registerSystem(const std::shared_ptr<System>& system) {
        auto name = std::string(system->getName());
        if(name.empty()) {
            throw std::runtime_error("System name cannot be empty");
        }
        systemRegistry[name] = system;
    }

    void sortSystems() {
        sortedSystems.clear();
        for(auto& [name, system] : systemRegistry) {
            sortedSystems.push_back(system);
        }
        std::sort(sortedSystems.begin(), sortedSystems.end(), [](std::shared_ptr<System> a, std::shared_ptr<System> b) {
            return a->getPriority() < b->getPriority();
        });
    }

    async<bool> System::shouldRun(double deltaTime) {
        co_return true;
    }

    async<void> System::run(double deltaTime) {
        co_return;
    }

    async<void> ProcessConnections::run(double deltaTime) {
        // First, handle any disconnected connections.
        auto disconnected = deadConnections;
        for (const auto &id : disconnected) {
            auto it = connections.find(id);
            if (it != connections.end()) {
                auto conn = it->second;
                conn->onNetworkDisconnected();
                connections.erase(it);
                deadConnections.erase(id);
            }
        }

        // Second, welcome any new connections!
        auto pending = pendingConnections;
        for(const auto& id : pending) {
            auto it = connections.find(id);
            if (it != connections.end()) {
                auto conn = it->second;
                // Need a proper welcoming later....
                conn->onWelcome();
                pendingConnections.erase(id);
            }
        }

        // Next, we must handle the heartbeat routine for each connection.
        for(auto& [id, c] : connections) {
            c->onHeartbeat(deltaTime);
        }

        co_return;
    }

    async<void> ProcessSessions::run(double deltaTime) {
        for(auto& [id, session] : sessions) {
            session->onHeartbeat(deltaTime);
        }
        co_return;
    }

    void registerSystems() {
        registerSystem(std::make_shared<ProcessConnections>());
        registerSystem(std::make_shared<ProcessSessions>());
        //registerSystem(std::make_shared<ProcessOutput>());
        //registerSystem(std::make_shared<ProcessCommands>());
    }

}