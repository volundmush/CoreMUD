#pragma once
#include "core/base.h"

namespace core {

        class System {
        public:
            virtual std::string getName() = 0;
            virtual int64_t getPriority() = 0;
            virtual async<bool> shouldRun(double deltaTime);
            virtual async<void> run(double deltaTime);
        };

        extern std::vector<std::shared_ptr<System>> sortedSystems;
        extern std::unordered_map<std::string, std::shared_ptr<System>> systemRegistry;
        void registerSystem(const std::shared_ptr<System>& system);
        void sortSystems();

}