#include "statemachine.h"

// TODO: should DISARMED really be the initial state?
StateMachine::StateMachine() : state(SystemState::DISARMED) {
}


/// @brief Returns the current state of the system
auto StateMachine::getState() const -> SystemState {
    return state;
}

/// @brief Sets the current state of the system to a new state
void StateMachine::setState(const SystemState newState) {
    state = newState;
}
