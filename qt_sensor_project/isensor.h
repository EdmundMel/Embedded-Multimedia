#ifndef ISENSOR_H
#define ISENSOR_H

#include <QString>
#include <QDateTime>

class ISensor
{
public:
    virtual ~ISensor() = default;

    virtual QString name() const = 0;
    virtual int readValue() = 0;
    virtual QDateTime measurmentTime() const = 0;
};

#endif // ISENSOR_H
