#ifndef STATEMACHINE_H
#define STATEMACHINE_H
#include <cstdint>

enum class SystemState: std::uint8_t {
    DISARMED,
    ARMED,
    HOLIDAY,
    ALARM,
};

class StateMachine {
public:
    StateMachine();

    [[nodiscard]] auto getState() const -> SystemState;

    void setState(SystemState newState);

private:
    SystemState state;
};

#endif
