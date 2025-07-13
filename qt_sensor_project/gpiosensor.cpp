#include "gpiosensor.h"
#include <QDebug>

GPIOSensor::GPIOSensor(const QString &sensorName, int gpioPin)
    : m_name(sensorName), m_gpioPin(gpioPin) {

    chip = gpiod_chip_open_by_name("gpiochip0");
    if (!chip) {
        qFatal("Failed to open gpiochip0");
    }

    line = gpiod_chip_get_line(chip, m_gpioPin);
    if (!line) {
        qFatal("Failed to get GPIO line %d", m_gpioPin);
    }

    if (gpiod_line_request_input(line, "qt-gpiod-app") < 0) {
        qFatal("Failed to request line %d as input", m_gpioPin);
    }
}

QString GPIOSensor::name() const {
    return m_name;
}

QDateTime GPIOSensor::measurmentTime() const {
    return m_measurmentTime;
}

int GPIOSensor::readValue() {
    m_measurmentTime = QDateTime::currentDateTime();

    int value = gpiod_line_get_value(line);
    if (value < 0) {
        qWarning() << "Failed to read GPIO line" << m_gpioPin;
        return -1;
    }

    return value;
}

void GPIOSensor::cleanupGpio() {
    if (line) {
        gpiod_line_release(line);
        line = nullptr;
    }
    if (chip) {
        gpiod_chip_close(chip);
        chip = nullptr;
    }
}

GPIOSensor::~GPIOSensor() {
    cleanupGpio();
}
