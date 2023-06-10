#include "core/system.h"

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
}