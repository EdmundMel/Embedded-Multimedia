#include <QApplication>
#include "pin_checker.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PinChecker pinChecker;
    QObject::connect(&pinChecker, &PinChecker::finished, &a, &QApplication::exit);
    return a.exec();
}
