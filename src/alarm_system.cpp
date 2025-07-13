#include "alarm_system.h"
#include "db_access.h"      // <‑‑‑ keep your existing DB layer *unchanged*

#include <iostream>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

AlarmSystem::AlarmSystem()  = default;
AlarmSystem::~AlarmSystem() { stop(); }

// -------- public API --------
void AlarmSystem::arm() {
    std::lock_guard<std::mutex> lk(state_mtx_);
    current_state_ = State::ARMED;
    std::cout << "[SYSTEM] Armed\n";
}

void AlarmSystem::disarm() {
    std::lock_guard<std::mutex> lk(state_mtx_);
    current_state_ = State::DISARMED;
    std::cout << "[SYSTEM] Disarmed\n";
}

void AlarmSystem::start() {
    if (running_) return;              // already running
    running_ = true;
    worker_  = std::thread(&AlarmSystem::pollLoop, this);
}

void AlarmSystem::stop() {
    running_ = false;
    if (worker_.joinable()) worker_.join();
}

// -------- internal helpers --------
void AlarmSystem::pollLoop() {
    while (running_) {
        try {
            // Fetch the 10 most‑recent sensor events
            auto events = Database::getRecentSensorEvents();
            for (const auto &ev : events) {
                handleEvent(ev);
            }
        } catch (const std::exception &ex) {
            std::cerr << "[ERROR] " << ex.what() << '\n';
        }

        std::this_thread::sleep_for(1s);  // poll once per second
    }
}

void AlarmSystem::handleEvent(const SensorEvent &ev) {
    std::lock_guard<std::mutex> lk(state_mtx_);

    switch (current_state_) {
        case State::DISARMED:
            // Ignore sensor noise while system is disarmed
            return;

        case State::ARMED: {
            if (ev.value != "1") return;   // We only transition on active/high values

            if (ev.sensor_id == "door") {
                current_state_ = State::CHECK;
                std::cout << "Checking pin\n";   // TODO: implement PIN verification later
            } else if (ev.sensor_id == "window" || ev.sensor_id == "motion") {
                current_state_ = State::ALARM;
                std::cout << "ALARM\n";          // TODO: trigger siren / notification
            }
            return;
        }

        case State::CHECK:
            // Placeholder — stay in CHECK until external input disarms or arms again
            return;

        case State::ALARM:
            // Placeholder — stay in ALARM until disarmed
            return;
    }
}