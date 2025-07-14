#ifndef ALARM_SYSTEM_H
#define ALARM_SYSTEM_H

#include "sensor_data.h"
#include <atomic>
#include <mutex>
#include <thread>
#include <string>

class AlarmSystem {
public:
    enum class State { DISARMED, ARMED, CHECK, ALARM };

    AlarmSystem();
    ~AlarmSystem();

    // User‑initiated transitions — accessible from *any* current state
    void arm();
    void disarm();

    // Exposed for HTTP server
    [[nodiscard]] State getState();

    // Start / stop background polling loop
    void start();
    void stop();

private:
    // ---------------- helper routines ----------------
    void pollLoop();                 // background polling worker
    void handleEvent(const SensorEvent &ev); // FSM core

    // Executes external Qt pin checker; returns true if exit‑code == 12
    bool runPinCheck();

    // Notify localhost:8080/alarm when ALARM triggers; best‑effort
    void sendAlarmNotification();
    // Notify localhost:8080/success when PIN was correctly given; best‑effort
    void sendDisarmNotification();

    void sendVideoToBot(); 

    // ---------------- data members -------------------
    std::atomic<bool> running_{false};
    std::thread        worker_;
    std::mutex         state_mtx_;
    State              current_state_{State::DISARMED};
    std::atomic<pid_t> active_rec_pid_{0};

    // Constants
    inline static constexpr const char *kPinCheckerPath =
        "/home/pi/Embedded-Multimedia/build-qt_pin_check_project-Desktop-Release/qt_pin_check_project";
};

#endif // ALARM_SYSTEM_H