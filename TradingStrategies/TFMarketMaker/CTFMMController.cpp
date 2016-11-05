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
    // 初始化日志类
    std::cout << "Initializing LogSys..." << std::endl;
    CLogSys::getLogSysInstance()->init();
    // 读取配置信息
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
    // 创建做市商委托价格调度类，并配置相关参数
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
    // 创建CTPMDApi和CTPMDSpi
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
    // 创建CTPTradeApi和CTPTradeSpi
    ptrCTPTraderApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
    ptrCTPTraderSpi = new CCTPTraderSpi(ptrCTPTraderApi,PRODUCTION);
    ptrCTPTraderSpi->setBrokerID(QString::fromStdString(strBrokerID));
    ptrCTPTraderSpi->setInvestorID(QString::fromStdString(strUserID));
    ptrCTPTraderSpi->setPassword(QString::fromStdString(strPassword));
    ptrCTPTraderApi->RegisterSpi(ptrCTPTraderSpi);
    // 创建国债期货做市商类，并配置相关参数
    std::cout << "Create TF MarketMaker, and config..." << std::endl;
    QList<HESS::CTFMarketMaker*> lstTFMMPtr;

    ptrTimeController = new CCFFEXTimeController();    // time controller
//    ptrTimeController = new CNightTradingTimeController();
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

    // 初始化行情缓冲区
    std::cout << "Initialize Market data buffer..." << std::endl;
    QStringList lstStrSecuCode;
    lstStrSecuCode << strSecuCode;
    CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->init(lstStrSecuCode);
    // 初始化COrderRef
    COrderRef::getCOrderRefPtr()->initOrderRef(0);
    // 初始化CRequestID
    CRequestID::getCRequestIDPtr()->initRequestID(0);
    // 创建线程，将相应类移入各自线程中
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
    // 登录CTP交易
    std::cout << "Login CTP TraderAPI..." << std::endl;
    /* 订阅私有流，THOST_TERT_RESTART=从本交易日开始重传;THOST_TERT_RESUME=从上次收到的续传;THOST_TERT_QUICK=只传送登录后私有流的内容 */
    ptrCTPTraderApi->SubscribePrivateTopic(THOST_TERT_QUICK);
    /* 订阅公共流 */
    ptrCTPTraderApi->SubscribePublicTopic(THOST_TERT_QUICK);
    /* 设置交易托管系统服务的地址，可以注册多个地址备用 */
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
    /* 与后台服务器建立连接 */
    ptrCTPTraderApi->Init();
    // 登录CTP行情，并订阅行情
    std::cout << "Login CTP MDUserApi, and subscribe market data..." << std::endl;
    /* 设置行情托管系统服务的地址，可以注册多个地址备用 */
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
    /* 与后台服务器建立连接 */
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

CTFMMControllerForBackTest::CTFMMControllerForBackTest()
{
    // 初始化日志类
    std::cout << "Initializing LogSys..." << std::endl;
    CLogSys::getLogSysInstance()->init();
    // 读取配置信息
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
    // 创建做市商委托价格调度类，并配置相关参数
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
    std::string strBegDate;
    if(mpConfig.find("BegDate") != mpConfig.end())
        strBegDate = mpConfig["BegDate"];
    else
    {
        std::cout << "Could not find config value of 'BegDate'." << std::endl;
        QCoreApplication::exit(-1);
    }
    std::string strEndDate;
    if(mpConfig.find("EndDate") != mpConfig.end())
        strEndDate = mpConfig["EndDate"];
    else
    {
        std::cout << "Could not find config value of 'EndDate'." << std::endl;
        QCoreApplication::exit(-1);
    }
    if(strBegDate > strEndDate)
    {
        std::cout << "BackTest begin date should not greater than end date." << std::endl;
        QCoreApplication::exit(-1);
    }
    std::string strTickDataDir;
    if(mpConfig.find("TickDataDir") != mpConfig.end())
        strTickDataDir = mpConfig["TickDataDir"];
    else
    {
        std::cout << "Could not find config value of 'TickDataDir'." << std::endl;
        QCoreApplication::exit(-1);
    }

    // 创建CTPMDApi和CTPMDSpi
    std::cout << "Create CTPMDApi and CTPMDSpi..." << std::endl;
    ptrCTPMDApi = new CMktDistributorApi();
    ptrCTPMDSpi = new CCTPMdSpi(ptrCTPMDApi);
    ptrCTPMDSpi->setBrokerID(QString::fromStdString(strBrokerID));
    ptrCTPMDSpi->setUserID(QString::fromStdString(strUserID));
    ptrCTPMDSpi->setPassword(QString::fromStdString(strPassword));
    ptrCTPMDSpi->addSecuCode(strSecuCode);
    ptrCTPMDSpi->setTickDataDir(QString::fromStdString(strTickDataDir));
    // 创建CTPTradeApi和CTPTradeSpi
    ptrCTPTraderApi = new CTradeMatchApi();
    ptrCTPTraderSpi = new CCTPTraderSpi(ptrCTPTraderApi,BACKTEST);
    ptrCTPTraderSpi->setBrokerID(QString::fromStdString(strBrokerID));
    ptrCTPTraderSpi->setInvestorID(QString::fromStdString(strUserID));
    ptrCTPTraderSpi->setPassword(QString::fromStdString(strPassword));
    QDate tmBegDate = QDate::fromString(QString::fromStdString(strBegDate),"yyyy-MM-dd");
    QDate tmEndDate = QDate::fromString(QString::fromStdString(strEndDate),"yyyy-MM-dd");
    ptrCTPTraderSpi->setBackTestInterval(tmBegDate,tmEndDate);
    // 创建国债期货做市商类，并配置相关参数
    std::cout << "Create TF MarketMaker, and config..." << std::endl;
    QList<HESS::CTFMarketMaker*> lstTFMMPtr;

    ptrTimeController = new CBackTestTimeController();    // time controller
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

    // 初始化行情缓冲区
    std::cout << "Initialize Market data buffer..." << std::endl;
    QStringList lstStrSecuCode;
    lstStrSecuCode << strSecuCode;
    CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->init(lstStrSecuCode);
    // 初始化COrderRef
    COrderRef::getCOrderRefPtr()->initOrderRef(0);
    // 初始化CRequestID
    CRequestID::getCRequestIDPtr()->initRequestID(0);
    // 创建线程，将相应类移入各自线程中
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
        connect(lstTFMMPtr[i],&CTFMarketMaker::sigHandleMktDataDone,CDerivativeMktDataBuffer::getMktDataBufferInstPtr(),&CDerivativeMktDataBuffer::ObserverHandleMktDataDone);

        // 向行情缓冲区添加观察者
        CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->addObserver();

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
    qRegisterMetaType<CThostFtdcDepthMarketDataField>("CThostFtdcDepthMarketDataField");
    connect(ptrCTPMDSpi,SIGNAL(sigOnRtnDepthMarketData(CThostFtdcDepthMarketDataField)),CDerivativeMktDataBuffer::getMktDataBufferInstPtr(),SLOT(setDepthMktData(CThostFtdcDepthMarketDataField)));

    connect(CDerivativeMktDataBuffer::getMktDataBufferInstPtr(),&CDerivativeMktDataBuffer::sigReadyToReceiveMktData,ptrCTPMDSpi,&CCTPMdSpi::OnDistributeMktData);

    // connect signals of CMktDistributorApi to slots of CCTPMdSpi
    connect(ptrCTPMDApi,&CMktDistributorApi::sigRegisterFront,ptrCTPMDSpi,&CCTPMdSpi::OnRegisterFront);
    connect(ptrCTPMDApi,&CMktDistributorApi::sigSubscribeMktData,ptrCTPMDSpi,&CCTPMdSpi::OnSubscribeMktData);
    connect(ptrCTPMDApi,&CMktDistributorApi::sigUnSubscribeMktData,ptrCTPMDSpi,&CCTPMdSpi::OnUnSubscribeMktData);
    connect(ptrCTPMDApi,&CMktDistributorApi::sigUserLogin,ptrCTPMDSpi,&CCTPMdSpi::OnUserLogin);
    connect(ptrCTPMDApi,&CMktDistributorApi::sigUserLogout,ptrCTPMDSpi,&CCTPMdSpi::OnUserLogout);

    // connect signals of CTradeMatchApi to slots of CCTPTraderSpi
    connect(ptrCTPTraderApi,&CTradeMatchApi::sigRegisterFront,ptrCTPTraderSpi,&CCTPTraderSpi::OnRegisterFront);
    connect(ptrCTPTraderApi,&CTradeMatchApi::sigUserLogin,ptrCTPTraderSpi,&CCTPTraderSpi::OnUserLogin);
    connect(ptrCTPTraderApi,&CTradeMatchApi::sigUserLogout,ptrCTPTraderSpi,&CCTPTraderSpi::OnUserLogout);
    connect(ptrCTPTraderApi,&CTradeMatchApi::sigOrderInsert,ptrCTPTraderSpi,&CCTPTraderSpi::OnOrderInsert);
    connect(ptrCTPTraderApi,&CTradeMatchApi::sigOrderAction,ptrCTPTraderSpi,&CCTPTraderSpi::OnOrderAction);
    connect(ptrCTPTraderApi,&CTradeMatchApi::sigSettlementInfoConfirm,ptrCTPTraderSpi,&CCTPTraderSpi::OnSettlementInfoConfirm);

    connect(CDerivativeMktDataBuffer::getMktDataBufferInstPtr(),&CDerivativeMktDataBuffer::mktDataUpdated,ptrCTPTraderSpi,&CCTPTraderSpi::OnMatchOder);

    // connect signal CCTPMdSpi::sigMktDataReplayCompleted to slot CCTPTraderSpi::OnSavePnL
    connect(ptrCTPMDSpi,&CCTPMdSpi::sigMktDataReplayCompleted,ptrCTPTraderSpi,&CCTPTraderSpi::OnSavePnL);

    // connect signal CCTPTraderSpi::sigSwitchToNextTradingDay to slot CCTPMdSpi::InitMktDataReplay
    connect(ptrCTPTraderSpi,&CCTPTraderSpi::sigSwitchToNextTradingDay,ptrCTPMDSpi,&CCTPMdSpi::InitMktDataReplay);

    // connect signal of CCTPTraderSpi::sigOnSavePnL to slot CLogSys::toFile
    connect(ptrCTPTraderSpi,&CCTPTraderSpi::sigOnSavePnL,CLogSys::getLogSysInstance(),&CLogSys::toFile);

    // connect signal of CDerivativeMktDataBuffer::sigMktDataUpdateTime to slot CBackTestTimeController::onSetCurrTime
    connect(CDerivativeMktDataBuffer::getMktDataBufferInstPtr(),&CDerivativeMktDataBuffer::sigMktDataUpdateTime,ptrTimeController,&CTimeController::onSetCurrTime);

    // start work threads
    for(int i = 0;i < lstTFMMWorkerThreads.size();++i)
    {
        lstTFMMWorkerThreads.at(i)->start();
    }
    ptrDispatcherTrd->start();
    ptrMktBufferTrd->start();
    ptrLogSysTrd->start();
    // 登录CTP交易
    std::cout << "Login CTP TraderAPI..." << std::endl;
    /* 设置交易托管系统服务的地址，可以注册多个地址备用 */
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
    // 登录CTP行情，并订阅行情
    std::cout << "Login CTP MDUserApi, and subscribe market data..." << std::endl;
    /* 设置行情托管系统服务的地址，可以注册多个地址备用 */
    if(mpConfig.find("MDFrontAdd") == mpConfig.end())
    {
        std::cout << "Could not find config value of 'MDFrontAdd'." << std::endl;
        QCoreApplication::exit(-1);
    }
    else
    {
        char szMDFrontAdd[100] = {0};
        strncpy(szMDFrontAdd,mpConfig["MDFrontAdd"].c_str(),sizeof(szMDFrontAdd)-1);
    }
    // 初始化CTPTraderSpi
//    ptrCTPTraderSpi->InitTradeInfo(tmBegDate);
    // 初始化CTPMDSpi
    ptrCTPMDSpi->InitMktDataReplay(ptrCTPTraderSpi->TradingDay());
}

CTFMMControllerForBackTest::~CTFMMControllerForBackTest()
{
    delete ptrCTPMDApi;
    delete ptrCTPMDSpi;
    delete ptrCTPTraderApi;
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
