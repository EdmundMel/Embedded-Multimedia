#include "pin_checker.h"
#include <QCoreApplication>
#include <QDebug>
#include <QProcess>
#include <QFile>

PinChecker::PinChecker(QObject* parent)
    : QObject(parent),
      serial(new QSerialPort(this)),
      attempts(0)
{
    currentInput = "";

    serial->setPortName("/dev/ttyAMA0");
    serial->setBaudRate(QSerialPort::Baud9600);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);

    pythonPath = "/home/pi/Embedded-Multimedia/qt_pin_check_project/venv/bin/python3";
    scriptPath = "/home/pi/Embedded-Multimedia/qt_pin_check_project/show_on_display.py";

    QProcess::execute(pythonPath, QStringList() << scriptPath << "----");

    if (serial->open(QIODevice::ReadOnly)) {
        connect(serial, &QSerialPort::readyRead, this, &PinChecker::handleKeypad);
    } else {
        qDebug() << "Failed to open /dev/ttyAMA0";
    }
}

PinChecker::~PinChecker() {
    serial->close();
    QProcess::execute(pythonPath, QStringList() << scriptPath << "clean");
}

void PinChecker::handleKeypad() {
    QByteArray data = serial->readAll();
    //QString hexString;
    for (const auto &ch : data) {
        //QChar c(ch);
        switch (ch) {
            case 0xE1:
                currentInput += "1";
                break;
            case 0xE2:
                currentInput += "2";
                break;
            case 0xE3:
                currentInput += "3";
                break;
            case 0xE4:
                currentInput += "4";
                break;
            case 0xE5:
                currentInput += "5";
                break;
            case 0xE6:
                currentInput += "6";
                break;
            case 0xE7:
                currentInput += "7";
                break;
            case 0xE8:
                currentInput += "8";
                break;
            case 0xE9:
                currentInput += "9";
                break;
            case 0xEA: // * used as clear
                cleanDisplay();
                return;
                break;
            case 0xEB:
                currentInput += "0";
                break;
            case 0xEC: // # used as clear
                cleanDisplay();
                return;
                break;
            default:
                break;
        }

        QProcess::execute(pythonPath, QStringList() << scriptPath << currentInput);
        if (currentInput.size() == 4) {
            checkPin();
            currentInput.clear();
        }
    }
}

void PinChecker::checkPin()
{
    QString correctPin;
    QFile file("/home/pi/Embedded-Multimedia/top_secret_password.txt");

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Could not open PIN file:" << file.errorString();
    } else {
        correctPin = QString::fromUtf8(file.readAll()).trimmed();
    }

    if (currentInput == correctPin) {
        QProcess::execute(pythonPath, QStringList() << scriptPath << "pass");
        emit finished(12);
        return;
    }

    attempts++;
    QProcess::execute(pythonPath, QStringList() << scriptPath << "fail");

    if (attempts >= maxAttempts) {
        emit finished(13);
    }
}

void PinChecker::cleanDisplay() {
    currentInput.clear();
    QProcess::execute(pythonPath, QStringList() << scriptPath << "----");
}
