#include <QCoreApplication>
#include "CTFMMController.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    HESS::CTFMMController tfmmCon;
//    HESS::CTFMMControllerForBackTest tfmmConBT;
//    HESS::CTFMMControllerForBackTestSingleTrd tfmmConBTSingleTrd;

    return a.exec();
}
