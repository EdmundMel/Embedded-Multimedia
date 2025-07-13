#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H
#include <string>
#include <chrono>

struct SensorEvent {
    std::string sensor_id;                         // e.g. "door", "window", "motion"
    std::string value;                             // raw value from DB, compare to "1"
    std::chrono::system_clock::time_point timestamp;
};

#endif // SENSOR_DATA_H
