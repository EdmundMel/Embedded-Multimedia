#include "db_access.h"
#include "sensor_data.h"

#include <chrono>
#include <vector>

/// @brief Fetches recent sensor events from the database
auto Database::getRecentSensorEvents() -> std::vector<SensorEvent> {
    // TODO: Implement actual database access logic to fetch recent sensor events
    // Dummy data until real DB implemented
    return {
        SensorEvent{
            .sensor_id = "door_sensor_1",
            .type = "door_open",
            .timestamp = std::chrono::system_clock::now(),
        }
    };
}
