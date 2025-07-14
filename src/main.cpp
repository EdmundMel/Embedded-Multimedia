#include "alarm_system.h"
#include "httplib.h"  // HTTP server

#include <iostream>
#include <fstream>
#include <thread>
#include <cstdlib>

static std::string stateToString(AlarmSystem::State s) {
    switch (s) {
        case AlarmSystem::State::DISARMED: return "disarmed";
        case AlarmSystem::State::ARMED:    return "armed";
        case AlarmSystem::State::CHECK:    return "check";
        case AlarmSystem::State::ALARM:    return "alarm";
    }
    return "unknown";
}

int main() {
    AlarmSystem system;
    system.start();

    // ---------------- HTTP server (port 8081) ------------------
    httplib::Server api;

    api.Get("/arm", [&](const httplib::Request&, httplib::Response& res) {
        system.arm();
        res.set_content("armed", "text/plain");
    });

    api.Get("/disarm", [&](const httplib::Request&, httplib::Response& res) {
        system.disarm();
        res.set_content("disarmed", "text/plain");
    });

    api.Get("/status", [&](const httplib::Request&, httplib::Response& res) {
        res.set_content(stateToString(system.getState()), "text/plain");
    });

    api.Get("/change_pin", [&](const httplib::Request& req, httplib::Response& res) {
        if (!req.has_param("pin")) {
            res.status = 400;
            res.set_content("missing pin param", "text/plain");
            return;
        }
        const auto pin = req.get_param_value("pin");
        std::ofstream ofs("/home/pi/Embedded-Multimedia/top_secret_password.txt", std::ios::trunc);
        if (!ofs) {
            res.status = 500;
            res.set_content("cannot open pin file", "text/plain");
            return;
        }
        ofs << pin << '\n';
        res.set_content("pin changed", "text/plain");
    });

    // Run server in its own thread so CLI still works
    std::thread server_thread([&]() {
        api.listen("127.0.0.1", 8081);
    });

    // ---------------- simple CLI (optional) --------------------
    std::cout << "Commands: arm | disarm | quit\n> ";
    for (std::string cmd; std::cin >> cmd;) {
        if (cmd == "arm")        system.arm();
        else if (cmd == "disarm") system.disarm();
        else if (cmd == "quit")   break;
        std::cout << "> ";
    }

    // Graceful shutdown
    api.stop();
    if (server_thread.joinable()) server_thread.join();
    system.stop();
    return 0;
}