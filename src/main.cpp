#include "alarm_system.h"
#include <iostream>

int main() {
    AlarmSystem system;

    system.start();

    std::cout << "Commands: arm | disarm | quit\n> ";
    for (std::string cmd; std::cin >> cmd;) {
        if (cmd == "arm")       system.arm();
        else if (cmd == "disarm") system.disarm();
        else if (cmd == "quit")  break;
        std::cout << "> ";
    }

    system.stop();
    return 0;
}

/*
Build example (PostgreSQL + pthread):
    g++ -std=c++20 alarm_system.cpp db_access.cpp -lpq -pthread -o alarm_demo
*/
