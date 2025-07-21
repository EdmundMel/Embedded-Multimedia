#include "alarm_system.h"
#include "db_access.h"      // <--- keep your existing DB layer *unchanged*

#define CPPHTTPLIB_FORM_DATA
#include "httplib.h"        // cpp-httplib single‑header library

#include <iostream>
#include <chrono>
#include <cstdlib>     // std::system
#include <sys/wait.h>  // WEXITSTATUS, WIFEXITED
#include <thread>
#include <signal.h>
#include <sys/types.h>
#include <fstream>
#include <vector>
#include <sstream>
#include <ctime>

using namespace std::chrono_literals;

AlarmSystem::AlarmSystem()  = default;
AlarmSystem::~AlarmSystem() { stop(); }

inline void executeAudioAsync(const std::string& filepath)
{
    std::thread([filepath] {
        std::string cmd = "aplay " + filepath;
        int rc = std::system(cmd.c_str());

        if (rc != 0)
            std::cerr << "[WARN] command failed (" << rc << "): " << cmd << '\n';
    }).detach();   // fire-and-forget
}

namespace 
{   
    void sendVideoFile(); 
    constexpr char kVideoPath[] = "/home/pi/Embedded-Multimedia/alarm.mp4";

    // ---------- continuous recording ----------
    pid_t startContinuousRecording()
    {
        pid_t pid = fork();
        if (pid == 0) {               // child → becomes ffmpeg
            execlp("ffmpeg",
                   "ffmpeg",
                   "-loglevel", "error",
                   "-f",        "v4l2",
                   "-i",        "/dev/video0",
                   "-vcodec",   "libx264",
                   "-preset",   "ultrafast",
                   "-y",        kVideoPath,
                   nullptr);
            _exit(127);               // only if exec failed
        }
        return pid;                   // parent keeps the PID
    }

    void stopRecording(pid_t pid)
    {
        if (pid <= 0) return;
        kill(pid, SIGINT);            // polite ⌃C → lets ffmpeg flush & close
        int status = 0;
        waitpid(pid, &status, 0);     // reap the zombie
    }

    // ---------- short clip ----------
    void recordClipAsync(std::chrono::seconds dur)
    {
        std::thread([dur] {
            std::ostringstream cmd;
            cmd << "ffmpeg -loglevel error -f v4l2 "
                   "-i /dev/video0 -vcodec libx264 -preset ultrafast "
                   "-t " << dur.count() << " -y " << kVideoPath;
            std::system(cmd.str().c_str());

            sendVideoFile();
        }).detach();
    }
}

namespace {
    void sendVideoFile()
    {
        std::ifstream file(kVideoPath, std::ios::binary);
        if (!file) {
            std::cerr << "[WARN] Video file not found; nothing sent\n";
            return;
        }
        std::vector<char> data((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());

        // ---- build minimal multipart body ----
        const std::string boundary = "----alarm-" + std::to_string(std::time(nullptr));

        std::ostringstream body;
        body << "--" << boundary << "\r\n"
            << "Content-Disposition: form-data; name=\"file\"; filename=\"alarm.mp4\"\r\n"
            << "Content-Type: video/mp4\r\n\r\n";
        body.write(data.data(), data.size());
        body << "\r\n--" << boundary << "--\r\n";

        httplib::Client cli("127.0.0.1", 8080);
        cli.set_read_timeout(5, 0);

        httplib::Headers hdr = {
            { "Content-Type", "multipart/form-data; boundary=" + boundary }
        };

        auto res = cli.Post("/video", hdr, body.str(), "multipart/form-data; boundary=" + boundary);
    }
}

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

AlarmSystem::State AlarmSystem::getState() {
    std::lock_guard<std::mutex> lk(state_mtx_);
    return current_state_;
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

void AlarmSystem::sendVideoToBot() { sendVideoFile(); }

void AlarmSystem::handleEvent(const SensorEvent &ev) {
    std::lock_guard<std::mutex> lk(state_mtx_);

    switch (current_state_) {
        case State::DISARMED:
            // Ignore sensor noise while system is disarmed
            return;

        case State::ARMED: {
            if (ev.value != "1") return;   // We only transition on active/high values

            if (ev.sensor_id == "door") {
                active_rec_pid_ = startContinuousRecording();
                executeAudioAsync("/home/pi/Embedded-Multimedia/pin.wav");
                current_state_ = State::CHECK;
                std::cout << "Checking pin\n";

                bool pinOk = runPinCheck();

                stopRecording(active_rec_pid_);
                active_rec_pid_ = 0;
                if (pinOk) {
                    current_state_ = State::DISARMED;
                    std::cout << "[PIN OK] Disarmed\n";
                    sendDisarmNotification();
                    executeAudioAsync("/home/pi/Embedded-Multimedia/welcome_traveler.wav");
                } else {
                    current_state_ = State::ALARM;
                    std::cout << "ALARM!\n";
                    sendAlarmNotification();
                    sendVideoFile();
                    executeAudioAsync("/home/pi/Embedded-Multimedia/alarm.wav");
                }
            } else if (ev.sensor_id == "window" || ev.sensor_id == "motion") {
                recordClipAsync(10s);
                current_state_ = State::ALARM;
                std::cout << "ALARM!\n";
                sendAlarmNotification();
                executeAudioAsync("/home/pi/Embedded-Multimedia/alarm.wav");
            }
            return;
        }

        case State::CHECK:
            // Should never stay here; handled immediately after door trigger
            return;

        case State::ALARM:
            // Stay in ALARM until user disarms
            return;
    }
}

bool AlarmSystem::runPinCheck() {
    int status = std::system(kPinCheckerPath);

    if (status == -1) {
        std::cerr << "[ERROR] Failed to launch PIN checker at " << kPinCheckerPath << "\n";
        return false;
    }

    if (WIFEXITED(status)) {
        int code = WEXITSTATUS(status);
        return code == 12;           // 12 = success, anything else triggers ALARM
    }

    return false; // Abnormal termination counts as failure
}

void AlarmSystem::sendAlarmNotification() {
    try {
        httplib::Client cli("127.0.0.1", 8080);
        cli.set_read_timeout(1, 0); // 1 second
        auto res = cli.Get("/alarm");
        if (!res || res->status >= 400) {
            std::cerr << "[WARN] Alarm notification failed\n";
        }
    } catch (...) {
        std::cerr << "[WARN] Exception during alarm notification\n";
    }
}

void AlarmSystem::sendDisarmNotification() {
    try {
        httplib::Client cli("127.0.0.1", 8080);
        cli.set_read_timeout(1, 0); // 1 second
        auto res = cli.Get("/success");
        if (!res || res->status >= 400) {
            std::cerr << "[WARN] Disarm notification failed\n";
        }
    } catch (...) {
        std::cerr << "[WARN] Exception during disarm notification\n";
    }
}