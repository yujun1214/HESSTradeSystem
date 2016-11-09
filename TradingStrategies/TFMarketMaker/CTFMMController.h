#ifndef CTFMMCONTROLLER_H
#define CTFMMCONTROLLER_H

#include <QObject>
#include "MarketData/CCTPMdSpi.h"
#include "TradeModule/CCTPTraderSpi.h"
#include "Utility/CTimeController.h"
#include "BackTest/CMktDistributorApi.h"
#include "BackTest/CTradeMatchApi.h"
#include <QList>
#include <QThread>

namespace HESS
{

class CTFMMController : public QObject
{
    Q_OBJECT

public:
    CTFMMController();

    ~CTFMMController();

private:
    CThostFtdcMdApi* ptrCTPMDApi;
    CCTPMdSpi* ptrCTPMDSpi;

    CThostFtdcTraderApi* ptrCTPTraderApi;
    CCTPTraderSpi* ptrCTPTraderSpi;

    CTimeController* ptrTimeController;
    QList<QThread*> lstTFMMWorkerThreads;
    QThread* ptrDispatcherTrd;
    QThread* ptrMktBufferTrd;
    QThread* ptrLogSysTrd;
};

}

#endif // CTFMMCONTROLLER_H
