//#include "mainwindow.h"
#include "gpiosensor.h"
#include "database.h"

#include <QApplication>
#include <QTimer>
#include <QThread>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //MainWindow w;
    //w.show();
    QTimer *timer = new QTimer(&a);
    QList<ISensor*> sensors;

    // add sensors here - name can be any, gpio must be configured
    sensors.append(new GPIOSensor("motion", 26));
    sensors.append(new GPIOSensor("window", 16)); // watch out: pin must be pull up: pinctrl set 16 ip pu

    // controls for checking if the sensor has triggered
    const int samplesBurst = 10;
    const int delayBurst = 100;
    int iDetected;
    QString sDetected;
    QObject::connect(timer, &QTimer::timeout, [&]() {
        for (auto sensor : sensors) {
            iDetected = 0;
            for (int i = 0; i < samplesBurst; ++i) {
                if (sensor->readValue() == 1) {
                    iDetected = 1;
                    break;
                }
                QThread::msleep(delayBurst);
            }
            sDetected = QString::number(iDetected);
            qDebug() << sensor->name() << "Value:" << sDetected << "Time:" << sensor->measurmentTime().toString();

            try {
                Database::insertSensorEvent(sensor->name(), sDetected, sensor->measurmentTime());
            } catch (const std::exception& e) {
                qWarning() << "Database insert failed:" << e.what();
            }
        }
    });

    timer->start(10000);

    return a.exec();
}
