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

    class ProcessConnections : public System {
    public:
        std::string getName() override {return "ProcessConnections";};
        int64_t getPriority() override {return -10000;};
        async<void> run(double deltaTime) override;
    };

    class ProcessSessions : public System {
    public:
        std::string getName() override {return "ProcessSessions";};
        int64_t getPriority() override {return -9000;};
        async<void> run(double deltaTime) override;
    };

    class ProcessOutput : public System {
    public:
        std::string getName() override {return "ProcessOutput";};
        int64_t getPriority() override {return 10000;};
        async<void> run(double deltaTime) override;
    };

    class ProcessCommands : public System {
    public:
        std::string getName() override {return "ProcessCommands";};
        int64_t getPriority() override {return 1000;};
        async<void> run(double deltaTime) override;
        virtual bool checkHooks(entt::entity ent, std::unordered_map<std::string, std::string>& input);
        virtual bool checkCommands(entt::entity ent, std::unordered_map<std::string, std::string>& input);
        virtual void handleNotFound(entt::entity ent, std::unordered_map<std::string, std::string>& input);
        virtual void handleBadMatch(entt::entity ent, std::unordered_map<std::string, std::string>& input);
    };

    extern std::vector<std::shared_ptr<System>> sortedSystems;
    extern std::unordered_map<std::string, std::shared_ptr<System>> systemRegistry;
    void registerSystem(const std::shared_ptr<System>& system);
    void sortSystems();

    void registerSystems();

}