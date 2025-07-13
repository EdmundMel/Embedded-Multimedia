#include <chrono>
#include <ctime>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>
#include <yaml-cpp/yaml.h>

#include "ruleengine.h"
#include "sensor_data.h"
#include "statemachine.h"

/// @brief Constructor that loads rules from a YAML file
RuleEngine::RuleEngine(const std::string &ruleFilePath) {
    YAML::Node config = YAML::LoadFile(ruleFilePath);

    // Iterate over each rule in the YAML configuration under "rules"
    for (const auto &ruleNode: config["rules"]) {
        ParsedRule rule;

        // Parse the "if" block of the rule
        if (auto ifBlock = ruleNode["if"]) {
            // parse the system state condition, if it is specified
            rule.state = ifBlock["state"] ? parseState(ifBlock["state"].as<std::string>()) : std::nullopt;

            // Parse the event condition, if it is specified
            if (auto eventNode = ifBlock["event"]) {
                if (eventNode.IsMap()) {
                    rule.event = EventMatch{
                        .type = eventNode["type"]
                                    ? eventNode["type"].as<std::string>()
                                    : throw std::runtime_error("Event type is required"),
                        .id = eventNode["id"] ? std::make_optional(eventNode["id"].as<std::string>()) : std::nullopt
                    };
                } else if (eventNode.IsScalar()) {
                    rule.event = EventMatch{
                        .type = eventNode.as<std::string>(),
                    };
                }
            }

            // Parse the elapsed time condition, if it is specified
            // TODO: abort if the system has been disarmed in the meantime
            rule.elapsed_gt = ifBlock["elapsed_gt"]
                                  ? std::make_optional(ifBlock["elapsed_gt"].as<int>())
                                  : std::nullopt;

            // Parse the time range condition, if it is specified
            if (auto timeRange = ifBlock["time_range"]) {
                rule.time_start = timeRange["start"]
                                      ? timeRange["start"].as<std::string>()
                                      : throw std::runtime_error("Time range start is required");
                rule.time_end = timeRange["end"]
                                    ? timeRange["end"].as<std::string>()
                                    : throw std::runtime_error("Time range end is required");
            }
        }

        // Parse the "then" block of the rule
        if (auto thenBlock = ruleNode["then"]) {
            // Parse the actions from the "then" block, if they are specified
            if (auto actionsNode = thenBlock["actions"]) {
                for (const auto &actNode: actionsNode) {
                    rule.actions.push_back({
                        .type = parseActionType(actNode["type"].as<std::string>()),
                        .id = actNode["id"] ? std::make_optional(actNode["id"].as<std::string>()) : std::nullopt
                    });
                }
            }
            // Parse the state transition from the "then" block, if it is specified
            rule.next_state = thenBlock["next_state"]
                                  ? parseState(thenBlock["next_state"].as<std::string>())
                                  : std::nullopt;
        }

        rules.push_back(rule);
    }
}

/// @brief Parses system state from string
auto RuleEngine::parseState(const std::string &str) -> std::optional<SystemState> {
    static const std::unordered_map<std::string, SystemState> stateMap = {
        {"DISARMED", SystemState::DISARMED},
        {"ARMED", SystemState::ARMED},
        {"HOLIDAY", SystemState::HOLIDAY},
        {"ALARM", SystemState::ALARM}
    };

    const auto it = stateMap.find(str);
    return it != stateMap.end() ? std::make_optional(it->second) : std::nullopt;
}

/// @brief Parses event type from string
auto RuleEngine::parseEventType(const std::string &str) -> EventType {
    static const std::unordered_map<std::string, EventType> eventMap = {
        {"MOTION", EventType::MOTION},
        {"DOOR_OPEN", EventType::DOOR_OPEN},
        {"WINDOW_OPEN", EventType::WINDOW_OPEN},
        {"GLASS_BREAK", EventType::GLASS_BREAK}
    };

    const auto it = eventMap.find(str);
    if (it != eventMap.end()) {
        return it->second;
    }
    throw std::runtime_error("Unknown event type: " + str);
}

/// @brief Parses action type from string
auto RuleEngine::parseActionType(const std::string &str) -> ActionType {
    static const std::unordered_map<std::string, ActionType> actionMap = {
        {"TRIGGER_SIREN", ActionType::TRIGGER_SIREN},
        {"TAKE_PHOTO", ActionType::TAKE_PHOTO},
        {"SEND_NOTIFICATION", ActionType::SEND_NOTIFICATION}
    };

    const auto it = actionMap.find(str);
    if (it != actionMap.end()) {
        return it->second;
    }
    throw std::runtime_error("Unknown action type: " + str);
}

/// @brief Checks if the current time is within the specified time range in the rule
auto RuleEngine::isInTimeRange(const ParsedRule &rule) -> bool {
    // If no time range is specified, the rule is always valid
    if (!rule.time_start || !rule.time_end) {
        return true;
    }

    const auto now = std::chrono::system_clock::now();
    std::time_t const now_c = std::chrono::system_clock::to_time_t(now);
    std::tm const local_tm = *std::localtime(&now_c);
    int const currentMinutes = (local_tm.tm_hour * 60) + local_tm.tm_min;

    auto toMinutes = [](const std::string &timeStr) -> int {
        int h;
        int m;
        char sep;
        std::istringstream iss(timeStr);
        iss >> h >> sep >> m;
        return h * 60 + m;
    };

    int const start = toMinutes(*rule.time_start);
    int const end = toMinutes(*rule.time_end);
    return currentMinutes < start || currentMinutes >= end;
}


/// @brief Evaluates rules based on the current system state and sensor event
auto RuleEngine::evaluate(const SystemState currentState, const SensorEvent &event,
                          std::optional<SystemState> &resultingState) const -> std::vector<Action> {
    std::vector<Action> actions;
    resultingState = std::nullopt;

    // Go through each rule and check conditions
    for (const auto &rule: rules) {
        // If the rule has a state condition, check if it matches the current state, else skip
        if (rule.state && *rule.state != currentState) {
            continue;
        }

        // If the rule has an event condition, check if it matches the event type and possibly the event id, else skip
        if (rule.event) {
            if (rule.event->type != event.type) {
                continue;
            }
            if (rule.event->id && rule.event->id != event.sensor_id) {
                // If the rule has an event ID condition, it must match the event's sensor_id
                continue;
            }
        }

        // If the rule has an elapsed_gt condition, check if the event is recent enough, else skip
        if (rule.elapsed_gt) {
            auto now = std::chrono::system_clock::now();
            // Note: explicit casting to silence warning here:
            const int elapsedSec = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(
                now - event.timestamp).count());
            if (elapsedSec <= *rule.elapsed_gt) {
                continue;
            }
        }

        // If the rule has a time range condition, check if the current time is within that range, else skip
        if (!isInTimeRange(rule)) {
            continue;
        }

        // If we reach here, the rule matches the current state and event

        // set the resulting state if specified
        resultingState = rule.next_state;

        // Add all actions from the rule to the actions list
        for (const auto &act: rule.actions) {
            actions.push_back(act);
        }
    }

    return actions;
}
