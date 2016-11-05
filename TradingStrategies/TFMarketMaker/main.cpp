#include <QCoreApplication>
#include "CTFMMController.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    HESS::CTFMMController tfmmCon;

    return a.exec();
}
