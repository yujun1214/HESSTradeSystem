#include "CTFMMController.h"
#include <QCoreApplication>
#include "Utility/CConfig.h"
#include <iostream>
#include "CMMDispatcher.h"
#include <QStringList>
#include "CTFMarketMaker.h"
#include "MarketData/CMktDataBuffer.h"
#include "Utility/COrderRef.h"
#include "Utility/CRequestID.h"
#include <string>
#include "Utility/cstr.h"
#include "Utility/CLogSys.h"
#include <QDate>

namespace HESS
{

CTFMMController::CTFMMController()
{
    // ��ʼ����־��
    std::cout << "Initializing LogSys..." << std::endl;
    CLogSys::getLogSysInstance()->init();
    // ��ȡ������Ϣ
    std::map<std::string,std::string> mpConfig;
    CConfig TFMMConfig("Config.ini");
    std::cout << "Reading config info..." << std::endl;
    if(!TFMMConfig.getConfig(&mpConfig))
    {
        std::cout << "Failed to read config file." << std::endl;
        QCoreApplication::exit(-1);
    }

    QString strSecuCode;
    if(mpConfig.find("SecuCode") != mpConfig.end())
        strSecuCode = QString::fromStdString(mpConfig["SecuCode"]);
    else
    {
        std::cout << "Could not find config value of 'SecuCode'." << std::endl;
        QCoreApplication::exit(-1);
    }
    // ����������ί�м۸�����࣬��������ز���
    std::cout << "Create MarketMaker dispather, and config..." << std::endl;
    CMMDispatcher* ptrDispatcher = new CMMDispatcher(strSecuCode);
    if(mpConfig.find("PriceTick") != mpConfig.end())
        ptrDispatcher->setPriceTick(atoi(mpConfig["PriceTick"].c_str()));
    else
    {
        std::cout << "Could not find config value of 'PriceTick'." <<std::endl;
        QCoreApplication::exit(-1);
    }
    if(mpConfig.find("PricePrecision") != mpConfig.end())
        ptrDispatcher->setPricePrecision(atoi(mpConfig["PricePrecision"].c_str()));
    else
    {
        std::cout << "Could not find config value of 'PricePrecision'." << std::endl;
        QCoreApplication::exit(-1);
    }
    if(mpConfig.find("Intervals") != mpConfig.end())
    {
        QStringList lstIntervals = QString::fromStdString(mpConfig["Intervals"]).split(",");
        foreach(const QString &str,lstIntervals)
            ptrDispatcher->addOrderInterval(str.toInt());
    }
    else
    {
        std::cout << "Could not find config value of 'Intervals'." << std::endl;
        QCoreApplication::exit(-1);
    }

    std::string strBrokerID;
    if(mpConfig.find("BrokerID") != mpConfig.end())
        strBrokerID = mpConfig["BrokerID"];
    else
    {
        std::cout << "Could not find config value of 'BrokerID'." << std::endl;
        QCoreApplication::exit(-1);
    }
    std::string strUserID;
    if(mpConfig.find("UserID") != mpConfig.end())
        strUserID = mpConfig["UserID"];
    else
    {
        std::cout << "Could not find config value of 'UserID'." << std::endl;
        QCoreApplication::exit(-1);
    }
    std::string strPassword;
    if(mpConfig.find("Password") != mpConfig.end())
        strPassword = mpConfig["Password"];
    else
    {
        std::cout << "Could not find config value of 'Password'." << std::endl;
        QCoreApplication::exit(-1);
    }
    // ����CTPMDApi��CTPMDSpi
    std::cout << "Create CTPMDApi and CTPMDSpi..." << std::endl;
    ptrCTPMDApi = CThostFtdcMdApi::CreateFtdcMdApi();
    ptrCTPMDSpi = new CCTPMdSpi(ptrCTPMDApi);
//    ptrCTPMDSpi->setBrokerID(strBrokerID.c_str());
    ptrCTPMDSpi->setBrokerID(QString::fromStdString(strBrokerID));
//    ptrCTPMDSpi->setUserID(strUserID.c_str());
    ptrCTPMDSpi->setUserID(QString::fromStdString(strUserID));
//    ptrCTPMDSpi->setPassword(strPassword.c_str());
    ptrCTPMDSpi->setPassword(QString::fromStdString(strPassword));
    ptrCTPMDSpi->addSecuCode(strSecuCode);
    ptrCTPMDApi->RegisterSpi(ptrCTPMDSpi);
    // ����CTPTradeApi��CTPTradeSpi
    ptrCTPTraderApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
    ptrCTPTraderSpi = new CCTPTraderSpi(ptrCTPTraderApi,PRODUCTION);
    ptrCTPTraderSpi->setBrokerID(QString::fromStdString(strBrokerID));
    ptrCTPTraderSpi->setInvestorID(QString::fromStdString(strUserID));
    ptrCTPTraderSpi->setPassword(QString::fromStdString(strPassword));
    ptrCTPTraderApi->RegisterSpi(ptrCTPTraderSpi);
    // ������ծ�ڻ��������࣬��������ز���
    std::cout << "Create TF MarketMaker, and config..." << std::endl;
    QList<HESS::CTFMarketMaker*> lstTFMMPtr;

//    ptrTimeController = new CCFFEXTimeController();    // time controller
    ptrTimeController = new CNightTradingTimeController();
    if(mpConfig.find("Intervals") != mpConfig.end())
    {
        QStringList lstIntervals = QString::fromStdString(mpConfig["Intervals"]).split(",");
        for(int i = 0;i < lstIntervals.size();++i)
        {
            CTFMarketMaker* ptrLongTFMM = new CTFMarketMaker(strSecuCode,LONGTRADING,ptrDispatcher);
            ptrLongTFMM->setPriceTick(atoi(mpConfig["PriceTick"].c_str()));
            ptrLongTFMM->setPricePrecision(atoi(mpConfig["PricePrecision"].c_str()));
            if(mpConfig.find("StopLossTicks") != mpConfig.end())
                ptrLongTFMM->setStoplossTicks(atoi(mpConfig["StopLossTicks"].c_str()));
            else
            {
                std::cout << "Could not find config value of 'StopLossTicks'." << std::endl;
                QCoreApplication::exit(-1);
            }
            ptrLongTFMM->setUserTradeApi(ptrCTPTraderApi);
            ptrLongTFMM->setTimeController(ptrTimeController);
            ptrLongTFMM->setBrokerID(strBrokerID.c_str());
            ptrLongTFMM->setInvestorID(strUserID.c_str());
            lstTFMMPtr.push_back(ptrLongTFMM);

            CTFMarketMaker* ptrShortTFMM = new CTFMarketMaker(strSecuCode,SHORTTRADING,ptrDispatcher);
            ptrShortTFMM->setPriceTick(atoi(mpConfig["PriceTick"].c_str()));
            ptrShortTFMM->setPricePrecision(atoi(mpConfig["PricePrecision"].c_str()));
            ptrShortTFMM->setStoplossTicks(atoi(mpConfig["StopLossTicks"].c_str()));
            ptrShortTFMM->setUserTradeApi(ptrCTPTraderApi);
            ptrShortTFMM->setTimeController(ptrTimeController);
            ptrShortTFMM->setBrokerID(strBrokerID.c_str());
            ptrShortTFMM->setInvestorID(strUserID.c_str());
            lstTFMMPtr.push_back(ptrShortTFMM);
        }
    }

    // ��ʼ�����黺����
    std::cout << "Initialize Market data buffer..." << std::endl;
    QStringList lstStrSecuCode;
    lstStrSecuCode << strSecuCode;
    CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->init(lstStrSecuCode);
    // ��ʼ��COrderRef
    COrderRef::getCOrderRefPtr()->initOrderRef(0);
    // ��ʼ��CRequestID
    CRequestID::getCRequestIDPtr()->initRequestID(0);
    // �����̣߳�����Ӧ����������߳���
    std::cout << "Create worker threads, move workers to threads..." << std::endl;

    ptrLogSysTrd = new QThread();
    CLogSys::getLogSysInstance()->moveToThread(ptrLogSysTrd);

    for(int i = 0;i < lstTFMMPtr.size();++i)
    {
        QThread* ptrWkTrd = new QThread();
        lstTFMMPtr[i]->moveToThread(ptrWkTrd);
        connect(ptrWkTrd,&QThread::finished,lstTFMMPtr[i],&QObject::deleteLater);
        lstTFMMWorkerThreads.push_back(ptrWkTrd);
    }

    ptrDispatcherTrd = new QThread();
    ptrDispatcher->moveToThread(ptrDispatcherTrd);
    connect(ptrDispatcherTrd,&QThread::finished,ptrDispatcher,&QObject::deleteLater);

    ptrMktBufferTrd = new QThread();
    CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->moveToThread(ptrMktBufferTrd);
    connect(ptrMktBufferTrd,&QThread::finished,CDerivativeMktDataBuffer::getMktDataBufferInstPtr(),&QObject::deleteLater);
    // connect signals and slots
    /* connect signals to CTFMarketMaker's slots */
    qRegisterMetaType<TThostFtdcFrontIDType>("TThostFtdcFrontIDType");
    qRegisterMetaType<TThostFtdcSessionIDType>("TThostFtdcSessionIDType");
    for(int i = 0;i < lstTFMMPtr.size();++i)
    {
        connect(CDerivativeMktDataBuffer::getMktDataBufferInstPtr(),&CDerivativeMktDataBuffer::mktDataUpdated,lstTFMMPtr[i],&CTFMarketMaker::handleUpdateMktData);
        connect(ptrCTPTraderSpi,&CCTPTraderSpi::sigOnRtnOrder,lstTFMMPtr[i],&CTFMarketMaker::handleRtnOrder);
        connect(ptrCTPTraderSpi,&CCTPTraderSpi::sigOnRtnTrade,lstTFMMPtr[i],&CTFMarketMaker::handleRtnTrade);
        connect(ptrCTPTraderSpi,&CCTPTraderSpi::sigOnErrRtnOrderAction,lstTFMMPtr[i],&CTFMarketMaker::handleErrRtnOrderAction);
        connect(ptrCTPTraderSpi,&CCTPTraderSpi::sigOnUserLogin,lstTFMMPtr[i],&CTFMarketMaker::setFrontSessionID);
        connect(lstTFMMPtr[i],&CTFMarketMaker::sigToConsole,CLogSys::getLogSysInstance(),&CLogSys::toConsole);

        if(LONGTRADING == lstTFMMPtr[i]->TradingType())
        {
            connect(lstTFMMPtr[i],&CTFMarketMaker::sigPositioned,ptrDispatcher,&CMMDispatcher::deleteBuyOrderPrice);
            connect(lstTFMMPtr[i],&CTFMarketMaker::sigOpenOrderDenied,ptrDispatcher,&CMMDispatcher::deleteBuyOrderPrice);
            connect(lstTFMMPtr[i],&CTFMarketMaker::sigLiquidationed,ptrDispatcher,&CMMDispatcher::deleteSellCloseOrderPrice);
            connect(lstTFMMPtr[i],&CTFMarketMaker::sigCloseOrderDenied,ptrDispatcher,&CMMDispatcher::deleteSellCloseOrderPrice);
            connect(lstTFMMPtr[i],&CTFMarketMaker::sigOpenOrderCanceled,ptrDispatcher,&CMMDispatcher::deleteBuyOrderPrice);
            connect(lstTFMMPtr[i],&CTFMarketMaker::sigCloseOrderCanceled,ptrDispatcher,&CMMDispatcher::deleteSellCloseOrderPrice);
        }
        else if(SHORTTRADING == lstTFMMPtr[i]->TradingType())
        {
            connect(lstTFMMPtr[i],&CTFMarketMaker::sigPositioned,ptrDispatcher,&CMMDispatcher::deleteSellOrderPrice);
            connect(lstTFMMPtr[i],&CTFMarketMaker::sigOpenOrderDenied,ptrDispatcher,&CMMDispatcher::deleteSellOrderPrice);
            connect(lstTFMMPtr[i],&CTFMarketMaker::sigLiquidationed,ptrDispatcher,&CMMDispatcher::deleteBuyCloseOrderPrice);
            connect(lstTFMMPtr[i],&CTFMarketMaker::sigCloseOrderDenied,ptrDispatcher,&CMMDispatcher::deleteBuyCloseOrderPrice);
            connect(lstTFMMPtr[i],&CTFMarketMaker::sigOpenOrderCanceled,ptrDispatcher,&CMMDispatcher::deleteSellOrderPrice);
            connect(lstTFMMPtr[i],&CTFMarketMaker::sigCloseOrderCanceled,ptrDispatcher,&CMMDispatcher::deleteBuyCloseOrderPrice);
        }
    }
//    connect(ptrCTPMDSpi,&CCTPMdSpi::sigOnRtnDepthMarketData,CDerivativeMktDataBuffer::getMktDataBufferInstPtr(),&CDerivativeMktDataBuffer::setDepthMktData);
    qRegisterMetaType<CThostFtdcDepthMarketDataField>("CThostFtdcDepthMarketDataField");
    connect(ptrCTPMDSpi,SIGNAL(sigOnRtnDepthMarketData(CThostFtdcDepthMarketDataField)),CDerivativeMktDataBuffer::getMktDataBufferInstPtr(),SLOT(setDepthMktData(CThostFtdcDepthMarketDataField)));
    // start work threads
    for(int i = 0;i < lstTFMMWorkerThreads.size();++i)
    {
        lstTFMMWorkerThreads.at(i)->start();
    }
    ptrDispatcherTrd->start();
    ptrMktBufferTrd->start();
    ptrLogSysTrd->start();
    // ��¼CTP����
    std::cout << "Login CTP TraderAPI..." << std::endl;
    /* ����˽������THOST_TERT_RESTART=�ӱ������տ�ʼ�ش�;THOST_TERT_RESUME=���ϴ��յ�������;THOST_TERT_QUICK=ֻ���͵�¼��˽���������� */
    ptrCTPTraderApi->SubscribePrivateTopic(THOST_TERT_QUICK);
    /* ���Ĺ����� */
    ptrCTPTraderApi->SubscribePublicTopic(THOST_TERT_QUICK);
    /* ���ý����й�ϵͳ����ĵ�ַ������ע������ַ���� */
    if(mpConfig.find("TradeFrontAdd") == mpConfig.end())
    {
        std::cout << "Could not find config value of 'TradeFrontAdd'." << std::endl;
        QCoreApplication::exit(-1);
    }
    else
    {
        char szTradeFrontAdd[100] = {0};
        strncpy(szTradeFrontAdd,mpConfig["TradeFrontAdd"].c_str(),sizeof(szTradeFrontAdd)-1);
        ptrCTPTraderApi->RegisterFront(szTradeFrontAdd);
    }
    /* ���̨�������������� */
    ptrCTPTraderApi->Init();
    // ��¼CTP���飬����������
    std::cout << "Login CTP MDUserApi, and subscribe market data..." << std::endl;
    /* ���������й�ϵͳ����ĵ�ַ������ע������ַ���� */
    if(mpConfig.find("MDFrontAdd") == mpConfig.end())
    {
        std::cout << "Could not find config value of 'MDFrontAdd'." << std::endl;
        QCoreApplication::exit(-1);
    }
    else
    {
        char szMDFrontAdd[100] = {0};
        strncpy(szMDFrontAdd,mpConfig["MDFrontAdd"].c_str(),sizeof(szMDFrontAdd)-1);
        ptrCTPMDApi->RegisterFront(szMDFrontAdd);
    }
    /* ���̨�������������� */
    ptrCTPMDApi->Init();
}

CTFMMController::~CTFMMController()
{
    ptrCTPMDApi->Release();
    delete ptrCTPMDSpi;
    ptrCTPTraderApi->Release();
    delete ptrCTPTraderSpi;

    delete ptrTimeController;
    while(!lstTFMMWorkerThreads.empty())
    {
        delete lstTFMMWorkerThreads.takeFirst();
    }
    delete ptrDispatcherTrd;
    delete ptrMktBufferTrd;
    delete ptrLogSysTrd;
}

} // namespace HESS
