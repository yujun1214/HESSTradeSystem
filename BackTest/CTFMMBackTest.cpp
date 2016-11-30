#include <iostream>
#include <map>
#include <string>
#include <string.h>
#include <math.h>
#include <QCoreApplication>
#include <QStringList>
#include <QTextStream>
#include <QString>
#include "CTFMMBackTest.h"
#include "Utility/CLogSys.h"
#include "Utility/CConfig.h"
#include "MarketData/CMktDataBuffer.h"
#include "Utility/COrderRef.h"
#include "Utility/CRequestID.h"

namespace HESS
{

CTFMMBackTest::~CTFMMBackTest()
{
    if(m_ptrTimeController != nullptr)
        delete m_ptrTimeController;
    if(m_ptrDispatcher != nullptr)
        delete m_ptrDispatcher;
    for(QList<CTFMarketMaker*>::iterator it = m_lstTFMMPtr.begin();it != m_lstTFMMPtr.end();++it)
    {
        delete *it;
    }
    m_lstTFMMPtr.clear();
    for(QMap<QString,QVector<CThostFtdcDepthMarketDataField>*>::iterator it = m_mpDepthMktData.begin();it != m_mpDepthMktData.end();++it)
    {
        delete it.value();
    }
    m_mpDepthMktData.clear();
    for(QMap<int,CThostFtdcInputOrderField*>::iterator it = m_ptrOrderPool->begin(); it != m_ptrOrderPool->end();++it)
    {
        delete it.value();
    }
    m_ptrOrderPool->clear();
    delete m_ptrOrderPool;

    m_ptrPnLFile->close();
    delete m_ptrPnLFile;
}

void CTFMMBackTest::doBackTest()
{
    // 遍历回测区间的交易日列表，播放行情，进行回测
    for(QVector<QDate>::const_iterator itday = m_vTradingDay.begin();itday != m_vTradingDay.end();++itday)
    {
        // 初始化深度行情数据
        initDepthMktData(*itday);
        // 初始化订单池
        initOrderPool();
        // 初始化COrderRef
        COrderRef::getCOrderRefPtr()->initOrderRef(0);
        // 初始化CRequestID
        CRequestID::getCRequestIDPtr()->initRequestID(0);
        // 播放当天深度行情数据
        while(onDistributeMktData())
        {

        }
        // 保存P&L
        QString strPnLData = QString("%1,%2").arg(itday->toString("yyyy-MM-dd")).arg(m_fPnL);
        CLogSys::getLogSysInstance()->toFile(m_ptrPnLFile,strPnLData);
    }
}

void CTFMMBackTest::initParam()
{
    // 初始化日志类
    std::cout << "Initializing LogSys..." << std::endl;
    CLogSys::getLogSysInstance()->init();
    // 读取配置信息
    std::map<std::string,std::string> mpConfig;
    CConfig TFMMConfig("config.ini");
    std::cout << "Reading config info..." << std::endl;
    if(!TFMMConfig.getConfig(&mpConfig))
    {
        std::cout << "[Error] Failed to read config file." << std::endl;
        QCoreApplication::exit(-1);
    }

    QString strSecuCode;
    if(mpConfig.find("SecuCode") != mpConfig.end())
        strSecuCode = QString::fromStdString(mpConfig["SecuCode"]);
    else
    {
        std::cout << "[Error] Could not find config value of 'SecuCode'." << std::endl;
        QCoreApplication::exit(-1);
    }
    // 创建做市商委托价格调度类，并配置相关参数
    std::cout << "Create market maker dispatcher, and config..." << std::endl;
    m_ptrDispatcher = new CMMDispatcher(strSecuCode);
    if(mpConfig.find("PriceTick") != mpConfig.end())
        m_ptrDispatcher->setPriceTick(atoi(mpConfig["PriceTick"].c_str()));
    else
    {
        std::cout << "[Error] Could not find config value of 'PriceTick'." << std::endl;
        QCoreApplication::exit(-1);
    }
    if(mpConfig.find("PricePrecision") != mpConfig.end())
        m_ptrDispatcher->setPricePrecision(atoi(mpConfig["PricePrecision"].c_str()));
    else
    {
        std::cout << "[Error] Could not find config value of 'PricePrecision'." << std::endl;
        QCoreApplication::exit(-1);
    }
    if(mpConfig.find("Intervals") != mpConfig.end())
    {
        QStringList lstIntervals = QString::fromStdString(mpConfig["Intervals"]).split(",");
        foreach(const QString &str,lstIntervals)
            m_ptrDispatcher->addOrderInterval(str.toInt());
    }
    else
    {
        std::cout << "[Error] Could not find config value of 'Intervals'." << std::endl;
        QCoreApplication::exit(-1);
    }
    // 创建time controller
    m_ptrTimeController = new CBackTestTimeController();
    // 创建国债期货做市商类，并配置相关参数
    std::cout << "Create TF market maker, and config..." << std::endl;
    if(mpConfig.find("Intervals") != mpConfig.end())
    {
        QStringList lstIntervals = QString::fromStdString(mpConfig["Intervals"]).split(",");
        for(int i = 0;i < lstIntervals.size();++i)
        {
            // create TFMarketMaker of long trading type
            CTFMarketMaker *ptrLongTFMM = new CTFMarketMaker(strSecuCode,LONGTRADING,m_ptrDispatcher);
            ptrLongTFMM->setPriceTick(atoi(mpConfig["PriceTick"].c_str()));
            ptrLongTFMM->setPricePrecision(atoi(mpConfig["PricePrecision"].c_str()));
            if(mpConfig.find("StopLossTicks") != mpConfig.end())
                ptrLongTFMM->setStoplossTicks(atoi(mpConfig["StopLossTicks"].c_str()));
            else
            {
                std::cout << "[Error] Could not find config value of 'StopLossTicks'." << std::endl;
                QCoreApplication::exit(-1);
            }
            ptrLongTFMM->setUserTradeApi(this);
            ptrLongTFMM->setTimeController(m_ptrTimeController);
            ptrLongTFMM->setBrokerID("000");
            ptrLongTFMM->setInvestorID("000");
            m_lstTFMMPtr.push_back(ptrLongTFMM);

            // create TFMarketMaker of short trading type
            CTFMarketMaker *ptrShortTFMM = new CTFMarketMaker(strSecuCode,SHORTTRADING,m_ptrDispatcher);
            ptrShortTFMM->setPriceTick(atoi(mpConfig["PriceTick"].c_str()));
            ptrShortTFMM->setPricePrecision(atoi(mpConfig["PricePrecision"].c_str()));
            ptrShortTFMM->setStoplossTicks(atoi(mpConfig["StopLossTicks"].c_str()));
            ptrShortTFMM->setUserTradeApi(this);
            ptrShortTFMM->setTimeController(m_ptrTimeController);
            ptrShortTFMM->setBrokerID("000");
            ptrShortTFMM->setInvestorID("000");
            m_lstTFMMPtr.push_back(ptrShortTFMM);
        }
    }
    // 初始化需要推送的深度行情数据
    if(!m_mpDepthMktData.empty())
    {
        QMap<QString,QVector<CThostFtdcDepthMarketDataField>*>::iterator it = m_mpDepthMktData.begin();
        for(;it != m_mpDepthMktData.end();++it)
        {
            delete it.value();
        }
        m_mpDepthMktData.clear();
    }
    // tick数据所在文件夹路径
    if(mpConfig.find("TickDataDir") != mpConfig.end())
        m_strTickDataDir = QString::fromStdString(mpConfig["TickDataDir"]);
    else
    {
        std::cout << "[Error] Could not find config value of 'TickDataDir'." << std::endl;
        QCoreApplication::exit(-1);
    }
    // 初始化订单池
    initOrderPool();
    // 初始化回测开始结束日期，并导入回测区间的交易日列表
    if(mpConfig.find("BegDate") != mpConfig.end())
        m_tmBegDate = QDate::fromString(mpConfig["BegDate"].c_str(),"yyyyMMdd");
    else
    {
        std::cout << "[Error] Could not find config value of 'BegDate'." << std::endl;
        QCoreApplication::exit(-1);
    }
    if(mpConfig.find("EndDate") != mpConfig.end())
        m_tmEndDate = QDate::fromString(mpConfig["EndDate"].c_str(),"yyyyMMdd");
    else
    {
        std::cout << "[Error] Could not find config value of 'EndDate'." << std::endl;
        QCoreApplication::exit(-1);
    }
    loadTradingDays();
    // 初始化accumulated PnL及PnL file pointer
    m_fPnL = 0.0;
    if(m_ptrPnLFile == nullptr)
        m_ptrPnLFile = new QFile("PnL.txt");
    // 初始化行情缓冲区
    std::cout << "Initialize market data buffer..." << std::endl;
    QStringList lstStrSecuCode;
    lstStrSecuCode << strSecuCode;
    m_lstSecuCode.push_back(strSecuCode);
    CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->init(lstStrSecuCode);
    // 初始化COrderRef
    COrderRef::getCOrderRefPtr()->initOrderRef(0);
    // 初始化CRequestID
    CRequestID::getCRequestIDPtr()->initRequestID(0);

}

int CTFMMBackTest::ReqOrderInsert(CThostFtdcInputOrderField *pInputOrder, int nRequestID)
{
    int nOrderRef = atoi(pInputOrder->OrderRef);
    if(!m_ptrOrderPool->contains(nOrderRef))    // 如果订单的OrderRef已存在，则不对该订单进行相应和撮合
    {
        // 先响应订单：生成CThostFtdcOrderField，调用CFTMarketMaker::handleRtnOrder
        CThostFtdcOrderField *ptrOrder = new CThostFtdcOrderField;
        memset(ptrOrder,0,sizeof(*ptrOrder));
        strcpy(ptrOrder->BrokerID,pInputOrder->BrokerID);
        strcpy(ptrOrder->InvestorID,pInputOrder->InvestorID);
        strcpy(ptrOrder->InstrumentID,pInputOrder->InstrumentID);
        strcpy(ptrOrder->OrderRef,pInputOrder->OrderRef);
        strcpy(ptrOrder->UserID,pInputOrder->UserID);
        ptrOrder->OrderPriceType = pInputOrder->OrderPriceType;
        ptrOrder->Direction = pInputOrder->Direction;
        strcpy(ptrOrder->CombOffsetFlag,pInputOrder->CombOffsetFlag);
        strcpy(ptrOrder->CombHedgeFlag,pInputOrder->CombHedgeFlag);
        ptrOrder->LimitPrice = pInputOrder->LimitPrice;
        ptrOrder->VolumeTotalOriginal = pInputOrder->VolumeTotalOriginal;
        ptrOrder->TimeCondition = pInputOrder->TimeCondition;
        strcpy(ptrOrder->GTDDate,pInputOrder->GTDDate);
        ptrOrder->VolumeCondition = pInputOrder->VolumeCondition;
        ptrOrder->MinVolume = pInputOrder->MinVolume;
        ptrOrder->ContingentCondition = pInputOrder->ContingentCondition;
        ptrOrder->StopPrice = pInputOrder->StopPrice;
        ptrOrder->RequestID = nRequestID;
        ptrOrder->OrderSubmitStatus = THOST_FTDC_OSS_Accepted;  // 报单已经接受
        ptrOrder->OrderStatus = THOST_FTDC_OST_NoTradeQueueing; // 未成交还在队列中
        ptrOrder->VolumeTraded = 0;
        ptrOrder->VolumeTotal = pInputOrder->VolumeTotalOriginal;
        strcpy(ptrOrder->StatusMsg,"Order has been accepted.");

        for(QList<CTFMarketMaker*>::iterator it = m_lstTFMMPtr.begin();it != m_lstTFMMPtr.end();++it)
        {
            (*it)->handleRtnOrder(ptrOrder);
        }

        // 再对订单进行即时撮合，如果订单成交，则不用插入订单池;如果订单不成交，则插入订单池
        if(!MatchOrder_OnInsertOrder(pInputOrder))
        {
            CThostFtdcInputOrderField *ptrNewOrder = new CThostFtdcInputOrderField;
            memset(ptrNewOrder,0,sizeof(*ptrNewOrder));
            strcpy(ptrNewOrder->BrokerID,pInputOrder->BrokerID);
            strcpy(ptrNewOrder->InvestorID,pInputOrder->InvestorID);
            strcpy(ptrNewOrder->InstrumentID,pInputOrder->InstrumentID);
            strcpy(ptrNewOrder->OrderRef,pInputOrder->OrderRef);
            strcpy(ptrNewOrder->UserID,pInputOrder->UserID);
            ptrNewOrder->OrderPriceType = pInputOrder->OrderPriceType;
            ptrNewOrder->Direction = pInputOrder->Direction;
            strcpy(ptrNewOrder->CombOffsetFlag,pInputOrder->CombOffsetFlag);
            strcpy(ptrNewOrder->CombHedgeFlag,pInputOrder->CombHedgeFlag);
            ptrNewOrder->LimitPrice = pInputOrder->LimitPrice;
            ptrNewOrder->VolumeTotalOriginal = pInputOrder->VolumeTotalOriginal;
            ptrNewOrder->TimeCondition = pInputOrder->TimeCondition;
            strcpy(ptrNewOrder->GTDDate,pInputOrder->GTDDate);
            ptrNewOrder->VolumeCondition = pInputOrder->VolumeCondition;
            ptrNewOrder->MinVolume = pInputOrder->MinVolume;
            ptrNewOrder->ContingentCondition = pInputOrder->ContingentCondition;
            ptrNewOrder->StopPrice = pInputOrder->StopPrice;
            ptrNewOrder->ForceCloseReason = pInputOrder->ForceCloseReason;
            ptrNewOrder->IsAutoSuspend = pInputOrder->IsAutoSuspend;
            ptrNewOrder->RequestID = nRequestID;

            m_ptrOrderPool->insert(nOrderRef,ptrNewOrder);
        }
    }
} // ReqOrderInsert

int CTFMMBackTest::ReqOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, int nRequestID)
{
    // 只处理撤单情况
    int nOrderRef = atoi(pInputOrderAction->OrderRef);
    if(THOST_FTDC_AF_Delete == pInputOrderAction->ActionFlag)
    {
        // 如果订单池中存在该订单，那么进行撤单，并调用CFTMarketMaker::handleRtnOrder
        if(m_ptrOrderPool->contains(nOrderRef))
        {
            CThostFtdcInputOrderField *ptrOrder = m_ptrOrderPool->value(nOrderRef);
            CThostFtdcOrderField *ptrCancelOrder = new CThostFtdcOrderField;
            memset(ptrCancelOrder,0,sizeof(*ptrCancelOrder));
            strcpy(ptrCancelOrder->BrokerID,ptrOrder->BrokerID);
            strcpy(ptrCancelOrder->InvestorID,ptrOrder->InvestorID);
            strcpy(ptrCancelOrder->InstrumentID,ptrOrder->InstrumentID);
            strcpy(ptrCancelOrder->OrderRef,ptrOrder->OrderRef);
            strcpy(ptrCancelOrder->UserID,ptrOrder->UserID);
            ptrCancelOrder->OrderPriceType = ptrOrder->OrderPriceType;
            ptrCancelOrder->Direction = ptrOrder->Direction;
            strcpy(ptrCancelOrder->CombOffsetFlag,ptrOrder->CombOffsetFlag);
            strcpy(ptrCancelOrder->CombHedgeFlag,ptrOrder->CombHedgeFlag);
            ptrCancelOrder->LimitPrice = ptrOrder->LimitPrice;
            ptrCancelOrder->VolumeTotalOriginal = ptrOrder->VolumeTotalOriginal;
            ptrCancelOrder->TimeCondition = ptrOrder->TimeCondition;
            strcpy(ptrCancelOrder->GTDDate,ptrOrder->GTDDate);
            ptrCancelOrder->VolumeCondition = ptrOrder->VolumeCondition;
            ptrCancelOrder->MinVolume = ptrOrder->MinVolume;
            ptrCancelOrder->ContingentCondition = ptrOrder->ContingentCondition;
            ptrCancelOrder->StopPrice = ptrOrder->StopPrice;
            ptrCancelOrder->ForceCloseReason = ptrOrder->ForceCloseReason;
            ptrCancelOrder->IsAutoSuspend = ptrOrder->IsAutoSuspend;
            ptrCancelOrder->RequestID = ptrOrder->RequestID;
            strcpy(ptrCancelOrder->ClientID,ptrOrder->ClientID);
            ptrCancelOrder->OrderSubmitStatus = THOST_FTDC_OSS_CancelSubmitted;     // 撤单已提交
            strcpy(ptrCancelOrder->TradingDay,ptrOrder->GTDDate);
            ptrCancelOrder->OrderStatus = THOST_FTDC_OST_Canceled;      // 撤单
            ptrCancelOrder->VolumeTraded = 0;
            ptrCancelOrder->VolumeTotal = ptrOrder->VolumeTotalOriginal;
            strcpy(ptrCancelOrder->InsertDate,ptrOrder->GTDDate);
            strcpy(ptrCancelOrder->StatusMsg,"Order has been canceled.");

            for(QList<CTFMarketMaker*>::iterator it = m_lstTFMMPtr.begin();it != m_lstTFMMPtr.end();++it)
            {
                (*it)->handleRtnOrder(ptrCancelOrder);
            }
            // 将已撤销订单从订单池中删除
            delete m_ptrOrderPool->value(nOrderRef);
            m_ptrOrderPool->remove(nOrderRef);
        }
        else
        {
            // 如果订单池中没有该订单，说明该订单已经成交或撤销，调用CFTMarketMaker::handleErrRtnOrderAction
            CThostFtdcOrderActionField *ptrNewOrderAction = new CThostFtdcOrderActionField;
            memset(ptrNewOrderAction,0,sizeof(*ptrNewOrderAction));
            strcpy(ptrNewOrderAction->BrokerID,pInputOrderAction->BrokerID);
            strcpy(ptrNewOrderAction->InvestorID,pInputOrderAction->InvestorID);
            ptrNewOrderAction->OrderActionRef = pInputOrderAction->OrderActionRef;
            strcpy(ptrNewOrderAction->OrderRef,pInputOrderAction->OrderRef);
            ptrNewOrderAction->RequestID = pInputOrderAction->RequestID;
            ptrNewOrderAction->ActionFlag = pInputOrderAction->ActionFlag;
            ptrNewOrderAction->OrderActionStatus = THOST_FTDC_OAS_Rejected; // 撤单被拒绝
            strcpy(ptrNewOrderAction->StatusMsg,"Cancel order has been rejected.");
            strcpy(ptrNewOrderAction->InstrumentID,pInputOrderAction->InstrumentID);

            for(QList<CTFMarketMaker*>::iterator it = m_lstTFMMPtr.begin();it != m_lstTFMMPtr.end();++it)
            {
                (*it)->handleErrRtnOrderAction(ptrNewOrderAction);
            }
        }
    }
} // ReqOrderAction

void CTFMMBackTest::initDepthMktData(QDate tmTradingday)
{
    // 清空之前加载的深度行情数据
    if(!m_mpDepthMktData.empty())
    {
        QMap<QString,QVector<CThostFtdcDepthMarketDataField>*>::iterator it = m_mpDepthMktData.begin();
        for(;it != m_mpDepthMktData.end();++it)
        {
            delete it.value();
        }
        m_mpDepthMktData.clear();
    }
    // 加载已订阅行情的合约的深度行情
    loadMktData(tmTradingday);
    //行情播放的位置索引重置为0
    if(!m_mpReplayIndex.empty())
        m_mpReplayIndex.clear();
    for(QList<QString>::const_iterator it = m_lstSecuCode.begin();it != m_lstSecuCode.end();++it)
    {
        if(!m_mpReplayIndex.contains(*it))
            m_mpReplayIndex[*it] = 0;
    }
    // 行情播放时间设置为行情数据中的最小时间
    if(!m_mpDepthMktData.empty())
    {
        QMap<QString,QVector<CThostFtdcDepthMarketDataField>*>::const_iterator it = m_mpDepthMktData.begin();
        strcpy(m_szMktReplayTime,it.value()->front().UpdateTime);
        ++it;
        for(;it != m_mpDepthMktData.end();++it)
        {
            if(strcmp(it.value()->front().UpdateTime,m_szMktReplayTime) < 0)
                strcpy(m_szMktReplayTime,it.value()->front().UpdateTime);
        }
    }
} // initDepthMktData

void CTFMMBackTest::loadMktData(QDate tmTradingDay)
{
    // 遍历订阅的合约，导入当天的tick数据
    for(QList<QString>::const_iterator itcode = m_lstSecuCode.begin();itcode != m_lstSecuCode.end();++itcode)
    {
        // 如果已导入该合约当天的tick数据，则continue
        if(m_mpDepthMktData.contains(*itcode))
            continue;

//        std::count << QString("Loading tick data of %1 at %2.").arg(*itcode,tmTradingDay.toString("yyyy-MM-dd")).toStdString() << std::endl;
        std::cout << "Loading tick data of " << itcode->toStdString() << " at " << tmTradingDay.toString("yyyy-MM-dd").toStdString() << std::endl;

        QString strTickFilePath = QString("%1%2_%3.csv").arg(m_strTickDataDir,*itcode,tmTradingDay.toString("yyyyMMdd"));
        QFile TickFile(strTickFilePath);
        if(!TickFile.open(QIODevice::ReadOnly | QIODevice::Text))
            continue;
        QTextStream in(&TickFile);
        in.readLine();  // 忽略第一行的标题行
        QVector<CThostFtdcDepthMarketDataField>* ptrvDepthMktData = new QVector<CThostFtdcDepthMarketDataField>();
        while(!in.atEnd())
        {
            QString tickline = in.readLine();
            QStringList tickdata = tickline.split(',');
            CThostFtdcDepthMarketDataField tDepthMktData;
            strcpy(tDepthMktData.TradingDay,tickdata[0].toStdString().c_str());
            strcpy(tDepthMktData.InstrumentID,tickdata[1].toStdString().c_str());
            tDepthMktData.LastPrice = atof(tickdata[4].toStdString().c_str());
            tDepthMktData.PreSettlementPrice = atof(tickdata[5].toStdString().c_str());
            tDepthMktData.PreClosePrice = atof(tickdata[6].toStdString().c_str());
            tDepthMktData.PreOpenInterest = atof(tickdata[7].toStdString().c_str());
            tDepthMktData.OpenPrice = atof(tickdata[8].toStdString().c_str());
            tDepthMktData.HighestPrice = atof(tickdata[9].toStdString().c_str());
            tDepthMktData.LowestPrice = atof(tickdata[10].toStdString().c_str());
            tDepthMktData.Volume = atoi(tickdata[11].toStdString().c_str());
            tDepthMktData.Turnover = atof(tickdata[12].toStdString().c_str());
            tDepthMktData.OpenInterest = atof(tickdata[13].toStdString().c_str());
            tDepthMktData.ClosePrice = atof(tickdata[14].toStdString().c_str());
            tDepthMktData.SettlementPrice = atof(tickdata[15].toStdString().c_str());
            tDepthMktData.UpperLimitPrice = atof(tickdata[16].toStdString().c_str());
            tDepthMktData.LowerLimitPrice = atof(tickdata[17].toStdString().c_str());
            tDepthMktData.PreDelta = atof(tickdata[18].toStdString().c_str());
            tDepthMktData.CurrDelta = atof(tickdata[19].toStdString().c_str());
            strcpy(tDepthMktData.UpdateTime,tickdata[20].toStdString().c_str());
            tDepthMktData.UpdateMillisec = atoi(tickdata[21].toStdString().c_str());
            tDepthMktData.BidPrice1 = atof(tickdata[22].toStdString().c_str());
            tDepthMktData.BidVolume1 = atoi(tickdata[23].toStdString().c_str());
            tDepthMktData.AskPrice1 = atof(tickdata[24].toStdString().c_str());
            tDepthMktData.AskVolume1 = atoi(tickdata[25].toStdString().c_str());
            tDepthMktData.BidPrice2 = atof(tickdata[26].toStdString().c_str());
            tDepthMktData.BidVolume2 = atoi(tickdata[27].toStdString().c_str());
            tDepthMktData.AskPrice2 = atof(tickdata[28].toStdString().c_str());
            tDepthMktData.AskVolume2 = atoi(tickdata[29].toStdString().c_str());
            tDepthMktData.BidPrice3 = atof(tickdata[30].toStdString().c_str());
            tDepthMktData.BidVolume3 = atoi(tickdata[31].toStdString().c_str());
            tDepthMktData.AskPrice3 = atof(tickdata[32].toStdString().c_str());
            tDepthMktData.AskVolume3 = atoi(tickdata[33].toStdString().c_str());
            tDepthMktData.BidPrice4 = atof(tickdata[34].toStdString().c_str());
            tDepthMktData.BidVolume4 = atoi(tickdata[35].toStdString().c_str());
            tDepthMktData.AskPrice4 = atof(tickdata[36].toStdString().c_str());
            tDepthMktData.AskVolume4 = atoi(tickdata[37].toStdString().c_str());
            tDepthMktData.BidPrice5 = atof(tickdata[38].toStdString().c_str());
            tDepthMktData.BidVolume5 = atoi(tickdata[39].toStdString().c_str());
            tDepthMktData.AskPrice5 = atof(tickdata[40].toStdString().c_str());
            tDepthMktData.AskVolume5 = atoi(tickdata[41].toStdString().c_str());
            tDepthMktData.AveragePrice = atof(tickdata[42].toStdString().c_str());
            strcpy(tDepthMktData.ActionDay,tickdata[43].toStdString().c_str());
            ptrvDepthMktData->push_back(tDepthMktData);
        }
        TickFile.close();
        m_mpDepthMktData.insert(*itcode,ptrvDepthMktData);
    }
} // loadMktData

bool CTFMMBackTest::onDistributeMktData()
{
    // 遍历已订阅合约代码，
    // (1)定位到播放行情的位置，
    // (2)如果定位到的行情时间小于等于（精确到秒）“当前播放行情对应的时间”，那么播放该行情
    // (3)更新“当前播放行情对应的时间
    TThostFtdcTimeType szNextMktReplayTime; // 记录下一次播放行情对应的时间（精确到秒）
    memset(szNextMktReplayTime,'0',sizeof(szNextMktReplayTime));
    bool isSetNextTime = false;     // 是否已设置了下次播放行情的对应时间
    bool isReplayCompleted = true;  // 当天行情是否全部播放完毕
    // 遍历合约代码，逐个播放深度行情
    for(QList<QString>::const_iterator itcode = m_lstSecuCode.begin();itcode != m_lstSecuCode.end();++itcode)
    {
        // 如果位置索引中不含当前合约，continue
        if(!m_mpReplayIndex.contains(*itcode))
            continue;
        int nReplayIndex = m_mpReplayIndex[*itcode];
        // 如果深度行情数据中不含当前合约，continue
        if(!m_mpDepthMktData.contains(*itcode))
            continue;
        // 如果当前合约当天深度行情已播放完毕，continue
        if(nReplayIndex >= m_mpDepthMktData.value(*itcode)->size())
            continue;
        isReplayCompleted = false;
        // 如果当前合约定位到的深度行情对应的时间小于等于本次行情播放时间(m_szMktReplayTime)，那么播放该深度行情
        if(strcmp(m_mpDepthMktData.value(*itcode)->at(nReplayIndex).UpdateTime,m_szMktReplayTime) <= 0)
        {
            // 更新行情缓冲区行情数据
            CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->setDepthMktData(m_mpDepthMktData[*itcode]->at(nReplayIndex));
            // 撮合订单
            onMatchOrder();
            // 调用CTFMarketMaker::handleUpdateMktData
            for(QList<CTFMarketMaker*>::iterator ittfmm = m_lstTFMMPtr.begin();ittfmm != m_lstTFMMPtr.end();++ittfmm)
            {
                (*ittfmm)->handleUpdateMktData(*itcode);
            }

            ++m_mpReplayIndex[*itcode]; // 位置索引递增
            if(++nReplayIndex < m_mpDepthMktData.value(*itcode)->size())
            {
                if(!isSetNextTime)
                {
                    strcpy(szNextMktReplayTime,m_mpDepthMktData.value(*itcode)->at(nReplayIndex).UpdateTime);
                    isSetNextTime = true;
                }
                else
                {
                    if(strcmp(m_mpDepthMktData.value(*itcode)->at(nReplayIndex).UpdateTime,szNextMktReplayTime) < 0)
                        strcpy(szNextMktReplayTime,m_mpDepthMktData.value(*itcode)->at(nReplayIndex).UpdateTime);
                }
            }
        }
        else
        {
            if(!isSetNextTime)
            {
                strcpy(szNextMktReplayTime,m_mpDepthMktData.value(*itcode)->at(nReplayIndex).UpdateTime);
                isSetNextTime = true;
            }
            else
            {
                if(strcmp(m_mpDepthMktData.value(*itcode)->at(nReplayIndex).UpdateTime,szNextMktReplayTime) < 0)
                    strcpy(szNextMktReplayTime,m_mpDepthMktData.value(*itcode)->at(nReplayIndex).UpdateTime);
            }
        }
    }
    // 更新当前播放行情对应的时间
    if(isSetNextTime)
        strcpy(m_szMktReplayTime,szNextMktReplayTime);
    // 如果当天深度行情播放完毕，返回false；否则返回true
    return !isReplayCompleted;

}// onDistributeMktData

void CTFMMBackTest::initOrderPool()
{
    if(m_ptrOrderPool == nullptr)
    {
        m_ptrOrderPool = new QMap<int,CThostFtdcInputOrderField*>();
    }
    else if(!m_ptrOrderPool->empty())
    {
        QMap<int,CThostFtdcInputOrderField*>::iterator it = m_ptrOrderPool->begin();
        for(;it != m_ptrOrderPool->end();++it)
        {
            delete it.value();
        }
        m_ptrOrderPool->clear();
    }
} // initOrderPool

void CTFMMBackTest::loadTradingDays()
{
    QFile fileTradingDays("TFTradingDay.csv");
    if(!fileTradingDays.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&fileTradingDays);

    m_vTradingDay.clear();
    while(!in.atEnd())
    {
        QString strTradingDay = in.readLine();
        QDate tmTradingDay = QDate::fromString(strTradingDay,"yyyy-MM-dd");
        if(tmTradingDay >= m_tmBegDate && tmTradingDay <= m_tmEndDate)
            m_vTradingDay.push_back(tmTradingDay);
    }

    fileTradingDays.close();
} // loadTradingDays

void CTFMMBackTest::onMatchOrder()
{
    // 遍历订单池，对订单进行撮合，如果成交，则释放该订单内存
    QMap<int,CThostFtdcInputOrderField*>::iterator it = m_ptrOrderPool->begin();
    while(it != m_ptrOrderPool->end())
    {
        if(MatchOrder_OnTraverseOrder(it.value()))
        {
            delete it.value();
            it = m_ptrOrderPool->erase(it);
        }
        else
            ++it;
    }
} // onMatchOrder

bool CTFMMBackTest::MatchOrder_OnInsertOrder(CThostFtdcInputOrderField *ptrOrder)
{
    bool bIsMatched = false;    // 是否撮合成交
    double fTradePrice = 0.0;   // 成交价
    QString strSecuCode = QString(ptrOrder->InstrumentID);
    if(THOST_FTDC_D_Buy == ptrOrder->CombOffsetFlag[0])
    {// 买入委托，委托价大于等于卖一价时，成交
        double fAskPrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(strSecuCode).AskPrice;
        double fLast = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(strSecuCode).Last;
        if(ptrOrder->LimitPrice > fAskPrice || fabs(ptrOrder->LimitPrice - fAskPrice) < EPSILON)
        {
            if(fAskPrice > fLast || fabs(fAskPrice - fLast) < EPSILON)
                fTradePrice = fAskPrice;
            else if(fLast > ptrOrder->LimitPrice || fabs(fLast - ptrOrder->LimitPrice) < EPSILON)
                fTradePrice = ptrOrder->LimitPrice;
            else
                fTradePrice = fLast;
            bIsMatched = true;
        }
    }
    else if(THOST_FTDC_D_Sell == ptrOrder->CombOffsetFlag[0])
    {// 卖出委托，委托价小于等于买一价时，成交
        double fBidPrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(strSecuCode).BidPrice;
        double fLast = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(strSecuCode).Last;
        if(fBidPrice > ptrOrder->LimitPrice || fabs(fBidPrice - ptrOrder->LimitPrice) < EPSILON)
        {
            if(ptrOrder->LimitPrice > fLast || fabs(ptrOrder->LimitPrice - fLast) < EPSILON)
                fTradePrice = ptrOrder->LimitPrice;
            else if(fLast > fBidPrice || fabs(fLast - fBidPrice) < EPSILON)
                fTradePrice = fBidPrice;
            else
                fTradePrice = fLast;

            bIsMatched = true;
        }
    }
    // 如果撮合成交，发送成交回报
    if(bIsMatched)
    {
        sendRtnTrade(ptrOrder,fTradePrice);
    }
    return bIsMatched;
} // MatchOrder_OnInsertOrder

bool CTFMMBackTest::MatchOrder_OnTraverseOrder(CThostFtdcInputOrderField *ptrOrder)
{
    bool bIsMatched = false;    // 是否撮合成交
    double fTradePrice = 0.0;   // 成交价
    QString strSecuCode = QString(ptrOrder->InstrumentID);
    if(THOST_FTDC_D_Buy == ptrOrder->CombOffsetFlag[0])
    {// 买入委托，委托价大于卖一价时，成交
        double fAskPrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(strSecuCode).AskPrice;
        double fLast = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(strSecuCode).Last;
        if(ptrOrder->LimitPrice > fAskPrice)
        {
            if(fAskPrice > fLast || fabs(fAskPrice - fLast) < EPSILON)
                fTradePrice = fAskPrice;
            else if(fLast > ptrOrder->LimitPrice || fabs(fLast - ptrOrder->LimitPrice) < EPSILON)
                fTradePrice = ptrOrder->LimitPrice;
            else
                fTradePrice = fLast;

            bIsMatched = true;
        }
    }
    else if(THOST_FTDC_D_Sell == ptrOrder->CombOffsetFlag[0])
    {// 卖出委托，委托价小于买一价时，成交
        double fBidPrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(strSecuCode).BidPrice;
        double fLast = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(strSecuCode).Last;
        if(fBidPrice > ptrOrder->LimitPrice)
        {
            if(ptrOrder->LimitPrice > fLast || fabs(ptrOrder->LimitPrice - fLast) < EPSILON)
                fTradePrice = ptrOrder->LimitPrice;
            else if(fLast > fBidPrice || fabs(fLast - fBidPrice) < EPSILON)
                fTradePrice = fBidPrice;
            else
                fTradePrice = fLast;

            bIsMatched = true;
        }
    }
    // 如果撮合成交，发送成交回报
    if(bIsMatched)
    {
        sendRtnTrade(ptrOrder,fTradePrice);
    }
    return bIsMatched;
} // MatchOrder_OnTraverseOrder

void CTFMMBackTest::sendRtnTrade(CThostFtdcInputOrderField *ptrOrder, double fTradePrice)
{
    // 撮合成交，依次调用CTFMarketMaker::handlRtnOrder，和CTFMarketMaker::handleRtnTrade
    CThostFtdcOrderField *ptrNewOrder = new CThostFtdcOrderField;
    memset(ptrNewOrder,0,sizeof(*ptrNewOrder));
    strcpy(ptrNewOrder->BrokerID,ptrOrder->BrokerID);
    strcpy(ptrNewOrder->InvestorID,ptrOrder->InvestorID);
    strcpy(ptrNewOrder->InstrumentID,ptrOrder->InstrumentID);
    strcpy(ptrNewOrder->OrderRef,ptrOrder->OrderRef);
    strcpy(ptrNewOrder->UserID,ptrOrder->UserID);
    ptrNewOrder->OrderPriceType = ptrOrder->OrderPriceType;
    ptrNewOrder->Direction = ptrOrder->Direction;
    strcpy(ptrNewOrder->CombOffsetFlag,ptrOrder->CombOffsetFlag);
    strcpy(ptrNewOrder->CombHedgeFlag,ptrOrder->CombHedgeFlag);
    ptrNewOrder->LimitPrice = ptrOrder->LimitPrice;
    ptrNewOrder->VolumeTotalOriginal = ptrOrder->VolumeTotalOriginal;
    ptrNewOrder->TimeCondition = ptrOrder->TimeCondition;
    strcpy(ptrOrder->GTDDate,ptrOrder->GTDDate);
    ptrNewOrder->VolumeCondition = ptrOrder->VolumeCondition;
    ptrNewOrder->MinVolume = ptrOrder->MinVolume;
    ptrNewOrder->ContingentCondition = ptrOrder->ContingentCondition;
    ptrNewOrder->StopPrice = ptrOrder->StopPrice;
    ptrNewOrder->RequestID = ptrOrder->RequestID;
    ptrNewOrder->OrderSubmitStatus = THOST_FTDC_OSS_Accepted;   // 报单已经接受
    ptrNewOrder->OrderStatus = THOST_FTDC_OST_AllTraded;        // 全部成交
    ptrNewOrder->VolumeTraded = 0;
    ptrNewOrder->VolumeTotal = ptrOrder->VolumeTotalOriginal;
    strcpy(ptrNewOrder->InsertDate,ptrOrder->GTDDate);
    strcpy(ptrNewOrder->StatusMsg,"Order has been traded.");

    for(QList<CTFMarketMaker*>::iterator it = m_lstTFMMPtr.begin();it != m_lstTFMMPtr.end();++it)
    {
        (*it)->handleRtnOrder(ptrNewOrder);
    }

    CThostFtdcTradeField *ptrTrade = new CThostFtdcTradeField;
    memset(ptrTrade,0,sizeof(*ptrTrade));
    strcpy(ptrTrade->BrokerID,ptrOrder->BrokerID);
    strcpy(ptrTrade->InvestorID,ptrOrder->InvestorID);
    strcpy(ptrTrade->InstrumentID,ptrOrder->InstrumentID);
    strcpy(ptrTrade->OrderRef,ptrOrder->OrderRef);
    strcpy(ptrTrade->UserID,ptrOrder->UserID);
    ptrTrade->Direction = ptrOrder->Direction;
    ptrTrade->OffsetFlag = ptrOrder->CombOffsetFlag[0];
    ptrTrade->HedgeFlag = ptrOrder->CombHedgeFlag[0];
    ptrTrade->Price = fTradePrice;
    ptrTrade->Volume = ptrOrder->VolumeTotalOriginal;
    strcpy(ptrTrade->TradeDate,ptrOrder->GTDDate);

    calcPnL(ptrTrade);  // 计算P&L

    for(QList<CTFMarketMaker*>::iterator it = m_lstTFMMPtr.begin();it != m_lstTFMMPtr.end();++it)
    {
        (*it)->handleRtnTrade(ptrTrade);
    }
} // sendRtnTrade

void CTFMMBackTest::calcPnL(const CThostFtdcTradeField *ptrTrade)
{
    if(ptrTrade == nullptr)
        return;
    switch(ptrTrade->Direction)
    {
    case THOST_FTDC_D_Buy:
        m_fPnL -= ptrTrade->Volume * ptrTrade->Price * MULTIPLIER;
        break;
    case THOST_FTDC_D_Sell:
        m_fPnL += ptrTrade->Volume * ptrTrade->Price * MULTIPLIER;
        break;
    }
} // calcPnL

} // namespace HESS
