// ===== alarm_system.h =====
#ifndef ALARM_SYSTEM_H
#define ALARM_SYSTEM_H

#include "sensor_data.h"
#include <atomic>
#include <mutex>
#include <thread>

class AlarmSystem {
public:
    enum class State { DISARMED, ARMED, CHECK, ALARM };

    AlarmSystem();
    ~AlarmSystem();

    // User‑initiated transitions — accessible from *any* current state
    void arm();
    void disarm();

    // Start / stop background polling loop
    void start();
    void stop();

private:
    // Continuously polls DB for the most‑recent events and feeds them to handleEvent()
    void pollLoop();

    // Implements the finite‑state‑machine transition logic
    void handleEvent(const SensorEvent &ev);

    std::atomic<bool> running_{false};
    std::thread        worker_;
    std::mutex         state_mtx_;   // protects current_state_
    State              current_state_{State::DISARMED};
};

#endif // ALARM_SYSTEM_H