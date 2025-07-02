#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H
#include <string>
#include <chrono>

struct SensorEvent {
    std::string sensor_id;
    std::string type;
    std::chrono::system_clock::time_point timestamp;
};

#endif
