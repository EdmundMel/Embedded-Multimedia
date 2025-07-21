#ifndef PIN_CHECKER_H
#define PIN_CHECKER_H

#include <QSerialPort>
#include <QTimer>
#include <QObject>

class PinChecker : public QObject
{
    Q_OBJECT
public:
    explicit PinChecker(QObject* parent = nullptr);
    ~PinChecker() override;

signals:
    void finished(int code); // exit code


private slots:
    void handleKeypad();
    void cleanDisplay();

private:
    void checkPin();
    QSerialPort *serial;
    QString currentInput;
    int     attempts = 0;
    const int maxAttempts = 3;
    QString pythonPath;
    QString scriptPath;
    QTimer* timeoutTimer;
    const int timeout = 30000; // timout for pincode input in ms
};


#endif // PIN_CHECKER_H
