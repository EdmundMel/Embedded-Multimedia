#include "db_access.h"
#include <libpq-fe.h>

auto Database::getRecentSensorEvents() -> std::vector<SensorEvent> {
    std::vector<SensorEvent> events;

    if (conn_->status() != CONNECTION_OK) {
        throw std::runtime_error("Connection to database failed: " + std::string(conn_->errorMessage()));
    }

    auto res = conn_->exec(
        "SELECT sensor_id, value, timestamp "
        "FROM sensor_events "
        "ORDER BY timestamp DESC "
        "LIMIT 10"
    );

    if (res->status() != PGRES_TUPLES_OK) {
        throw std::runtime_error("Query failed: " + std::string(conn_->errorMessage()));
    }

    int n = res->ntuples();
    for (int i = 0; i < n; ++i) {
        std::string sensor_id = res->value(i, 0);
        std::string value     = res->value(i, 1);
        std::string timestampStr = res->value(i, 2);

        std::tm tm = {};
        std::istringstream ss(timestampStr);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));

        events.push_back(SensorEvent{std::move(sensor_id), std::move(value), tp});
    }

    res->clear();
    conn_->finish();
    return events;
}