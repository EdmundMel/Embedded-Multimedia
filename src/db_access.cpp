#include "db_access.h"
#include "sensor_data.h"

#include <chrono>
#include <vector>
#include <string>
#include <stdexcept>

#include <libpq-fe.h>

auto Database::getRecentSensorEvents() -> std::vector<SensorEvent> {
    std::vector<SensorEvent> events;

    // Connection string: adjust user, password, dbname as needed
    const char* conninfo = "host=localhost port=5432 dbname=sensordb user=dbuser password=secret";

    PGconn* conn = PQconnectdb(conninfo);
    if (PQstatus(conn) != CONNECTION_OK) {
        std::string err = PQerrorMessage(conn);
        PQfinish(conn);
        throw std::runtime_error("Connection to database failed: " + err);
    }

    // Example query: last 10 sensor events
    PGresult* res = PQexec(conn,
        "SELECT sensor_id, type, timestamp FROM sensor_events ORDER BY timestamp DESC LIMIT 10");

    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::string err = PQerrorMessage(conn);
        PQclear(res);
        PQfinish(conn);
        throw std::runtime_error("Query failed: " + err);
    }

    int n = PQntuples(res);
    for (int i = 0; i < n; ++i) {
        std::string sensor_id = PQgetvalue(res, i, 0);
        std::string type = PQgetvalue(res, i, 1);
        std::string timestamp_str = PQgetvalue(res, i, 2);

        // Parse ISO 8601 timestamp string to std::chrono
        std::tm tm = {};
        std::istringstream ss(timestamp_str);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S"); // adjust format if needed

        auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));

        events.push_back(SensorEvent{
            .sensor_id = sensor_id,
            .type = type,
            .timestamp = tp,
        });
    }

    PQclear(res);
    PQfinish(conn);
    return events;
}
