#ifndef RULEENGINE_H
#define RULEENGINE_H
#include <string>
#include <vector>

#include "sensor_data.h"
#include "statemachine.h"

// NOTE: Remember to adjust `RuleEngine::parseActionType` when changing enum variants here!!!
enum class ActionType : std::uint8_t {
    TRIGGER_SIREN,
    TAKE_PHOTO,
    SEND_NOTIFICATION,
};

struct Action {
    ActionType type{};
    std::optional<std::string> id;
};

// NOTE: Remember to adjust `RuleEngine::parseEventType` when changing enum variants here!!!
enum class EventType : std::uint8_t {
    ARMING,
    DISARMING,
    MOTION,
    DOOR_OPEN,
    WINDOW_OPEN,
    GLASS_BREAK,
};

struct EventMatch {
    std::string type;
    std::optional<std::string> id;
};

struct ParsedRule {
    std::optional<SystemState> state;
    std::optional<EventMatch> event;
    std::optional<int> elapsed_gt;
    std::optional<std::string> time_start; // in HH:MM
    std::optional<std::string> time_end; // in HH:MM
    std::vector<Action> actions;
    std::optional<SystemState> next_state;
};

class RuleEngine {
public:
    explicit RuleEngine(const std::string &ruleFilePath);

    [[nodiscard]] auto evaluate(SystemState currentState, const SensorEvent &event,
                                 std::optional<SystemState> &resultingState) const -> std::vector<Action>;

private:
    std::vector<ParsedRule> rules;

    [[nodiscard]] static auto parseActionType(const std::string &str) -> ActionType;

    [[nodiscard]] static auto parseState(const std::string &str) -> std::optional<SystemState>;

    [[nodiscard]] static auto parseEventType(const std::string &str) -> EventType;

    [[nodiscard]] static auto isInTimeRange(const ParsedRule &rule) -> bool;
};

#endif
