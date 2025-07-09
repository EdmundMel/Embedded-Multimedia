#include "database.h"
#include <libpq-fe.h>
#include <stdexcept>

void Database::insertSensorEvent(const QString& sensorId, const QString& value, const QDateTime &timestamp) {
    const char* conninfo = "host=localhost port=5432 dbname=sensordb user=dbuser password=secret";
    PGconn* conn = PQconnectdb(conninfo);

    if (PQstatus(conn) != CONNECTION_OK) {
        std::string err = PQerrorMessage(conn);
        PQfinish(conn);
        throw std::runtime_error("Connection to database failed: " + err);
    }

    QString query = QString(
        "INSERT INTO sensor_events (sensor_id, value, timestamp) VALUES ('%1', '%2', '%3')")
        .arg(sensorId)
        .arg(value)
        .arg(timestamp.toString("yyyy-MM-dd HH:mm:ss"));

    PGresult* res = PQexec(conn, query.toUtf8().constData());

    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::string err = PQerrorMessage(conn);
        PQclear(res);
        PQfinish(conn);
        throw std::runtime_error("Insert failed: " + err);
    }

    PQclear(res);
    PQfinish(conn);
}
