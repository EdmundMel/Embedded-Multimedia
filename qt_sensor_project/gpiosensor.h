#ifndef GPIOSENSOR_H
#define GPIOSENSOR_H

#include "isensor.h"
#include <gpiod.h>

class GPIOSensor : public ISensor
{
public:
    GPIOSensor(const QString &sensorName, int gpioPin);
    ~GPIOSensor();

    QString name() const override;
    QDateTime measurmentTime() const override;
    int readValue() override;

    void cleanupGpio();

private:
    QString m_name;
    int m_gpioPin;
    QDateTime m_measurmentTime;

    gpiod_chip *chip;
    gpiod_line *line;

};

#endif // GPIOSENSOR_H
