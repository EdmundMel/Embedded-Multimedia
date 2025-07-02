#include <chrono>
#include <iostream>
#include <optional>
#include <thread>

#include "core.h"
#include "db_access.h"
#include "ruleengine.h"
#include "statemachine.h"


/// @brief Core module that integrates FSM, Rule Engine, and DB access
Core::Core() : ruleEngine("rules/rules.yaml") {
}

/// @brief Main loop that runs the core logic
void Core::run() {
    while (true) {
        tick();
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}

/// @brief Processes recent sensor events, evaluates rules, and triggers actions
void Core::tick() {
    // Fetch recent sensor events from the database
    const auto sensorEvents = Database::getRecentSensorEvents();

    // Process each sensor event through the finite state machine
    for (const auto &event: sensorEvents) {
        std::optional<SystemState> newState;

        // Evaluate rules based on the current state and the event
        auto actions = ruleEngine.evaluate(fsm.getState(), event, newState);

        if (newState) {
            fsm.setState(*newState);
        }

        // Handle the actions returned by the rule engine
        for (const auto &action: actions) {
            std::string const idPart = action.id ? " [id=" + *action.id + "]" : "";
            switch (action.type) {
                case ActionType::TRIGGER_SIREN:
                    // TODO: Implement actual siren triggering logic
                    std::cout << "[ACTUATOR] Trigger siren" << idPart << '\n';
                    break;
                case ActionType::TAKE_PHOTO:
                    // TODO: Implement actual photo taking logic
                    std::cout << "[ACTUATOR] Take photo" << idPart << '\n';
                    break;
                case ActionType::SEND_NOTIFICATION:
                    // TODO: Implement actual notification sending logic
                    std::cout << "[NOTIFICATION] Send message" << idPart << '\n';
                    break;
            }
        }
    }
}
