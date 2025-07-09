#pragma once
#include <QString>
#include <QDateTime>

class Database {
public:
    static void insertSensorEvent(const QString& sensorId, const QString& value, const QDateTime& timestamp);
};
