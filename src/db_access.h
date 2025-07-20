#ifndef DB_ACCESS_H
#define DB_ACCESS_H
#include <vector>

#include "sensor_data.h"

class Database {
public:
    [[nodiscard]] static auto getRecentSensorEvents() -> std::vector<SensorEvent>;
};

#endif
