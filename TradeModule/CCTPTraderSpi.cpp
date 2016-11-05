#include <iostream>
#include <QCoreApplication>
#include <QDate>
#include <QTime>
#include <QMutexLocker>
#include <QTextStream>
#include <math.h>
#include <cstring>
#include <stdio.h>
#include "Utility/Utilities.h"
#include "CCTPTraderSpi.h"
#include "Utility/CRequestID.h"
#include "Utility/COrderRef.h"
#include "MarketData/CMktDataBuffer.h"

namespace HESS
{

void CCTPTraderSpi::OnFrontConnected()
{
    CThostFtdcReqUserLoginField tLoginField;
    strcpy(tLoginField.BrokerID,m_szBrokerID);
    strcpy(tLoginField.UserID,m_szInvestorID);
    strcpy(tLoginField.Password,m_szPassword);
    int ret = m_ptrTraderApi->ReqUserLogin(&tLoginField,CRequestID::getCRequestIDPtr()->getValidRequestID());
    std::cout << "The front address connected! " << std::endl;

} // OnFrontConnected

void CCTPTraderSpi::OnFrontDisconnected()
{
    std::cout << "The front address disconnected!" << std::endl;
}

void CCTPTraderSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if(pRspInfo && !pRspInfo->ErrorID && bIsLast)
    {
        std::cout << "Login succeeded." << std::endl;

        m_nFrontID = pRspUserLogin->FrontID;
        m_nSessionID = pRspUserLogin->SessionID;
        // 设置COrderRef单例的OrderRef初始值
        COrderRef::getCOrderRefPtr()->initOrderRef(atoi(pRspUserLogin->MaxOrderRef));
        // 发送已登录信号
        emit sigOnUserLogin(m_nFrontID,m_nSessionID);
        // 结算单确认
        CThostFtdcSettlementInfoConfirmField tSetInfoConf;
        strcpy(tSetInfoConf.BrokerID,pRspUserLogin->BrokerID);
        strcpy(tSetInfoConf.InvestorID,pRspUserLogin->UserID);
        strcpy(tSetInfoConf.ConfirmDate,pRspUserLogin->TradingDay);
        int ret = m_ptrTraderApi->ReqSettlementInfoConfirm(&tSetInfoConf,CRequestID::getCRequestIDPtr()->getValidRequestID());
    }
    else if(pRspInfo->ErrorID)
    {
        printf("Failed to login, ErrorID = %d, ErrorMsg = %s.\n",pRspInfo->ErrorID,pRspInfo->ErrorMsg);
        QCoreApplication::exit(-1);
    }
}

void CCTPTraderSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if(pRspInfo && !pRspInfo->ErrorID && bIsLast)
    {
        std::cout << "Logout succeeded." << std::endl;
    }
    else if(pRspInfo->ErrorID)
    {
        printf("Failed to logout, ErrorID = %d, ErrorMsg = %s\n",pRspInfo->ErrorID,pRspInfo->ErrorMsg);
    }
}

void CCTPTraderSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    std::cout << "Settlement info has confirmed." << std::endl;
}

void CCTPTraderSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    printf("Something Error, ErrorCode = %d, ErrorMsg = %s,RequestID = %d",pRspInfo->ErrorID,pRspInfo->ErrorMsg,nRequestID);
}

void CCTPTraderSpi::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
    CThostFtdcOrderField* ptrNewOrder = new CThostFtdcOrderField;
    memset(ptrNewOrder,0,sizeof(*ptrNewOrder));
    strcpy(ptrNewOrder->BrokerID,pOrder->BrokerID);
    strcpy(ptrNewOrder->InvestorID,pOrder->InvestorID);
    strcpy(ptrNewOrder->InstrumentID,pOrder->InstrumentID);
    strcpy(ptrNewOrder->OrderRef,pOrder->OrderRef);
    strcpy(ptrNewOrder->UserID,pOrder->UserID);
    ptrNewOrder->OrderPriceType = pOrder->OrderPriceType;
    ptrNewOrder->Direction = pOrder->Direction;
    strcpy(ptrNewOrder->CombOffsetFlag,pOrder->CombOffsetFlag);
    strcpy(ptrNewOrder->CombHedgeFlag,pOrder->CombHedgeFlag);
    ptrNewOrder->LimitPrice = pOrder->LimitPrice;
    ptrNewOrder->VolumeTotalOriginal = pOrder->VolumeTotalOriginal;
    ptrNewOrder->TimeCondition = pOrder->TimeCondition;
    strcpy(ptrNewOrder->GTDDate,pOrder->GTDDate);
    ptrNewOrder->VolumeCondition = pOrder->VolumeCondition;
    ptrNewOrder->MinVolume = pOrder->MinVolume;
    ptrNewOrder->ContingentCondition = pOrder->ContingentCondition;
    ptrNewOrder->StopPrice = pOrder->StopPrice;
    ptrNewOrder->ForceCloseReason = pOrder->ForceCloseReason;
    ptrNewOrder->IsAutoSuspend = pOrder->IsAutoSuspend;
    strcpy(ptrNewOrder->BusinessUnit,pOrder->BusinessUnit);
    ptrNewOrder->RequestID = pOrder->RequestID;
    strcpy(ptrNewOrder->OrderLocalID,pOrder->OrderLocalID);
    strcpy(ptrNewOrder->ExchangeID,pOrder->ExchangeID);
    strcpy(ptrNewOrder->ParticipantID,pOrder->ParticipantID);
    strcpy(ptrNewOrder->ClientID,pOrder->ClientID);
    strcpy(ptrNewOrder->ExchangeInstID,pOrder->ExchangeInstID);
    strcpy(ptrNewOrder->TraderID,pOrder->TraderID);
    ptrNewOrder->InstallID = pOrder->InstallID;
    ptrNewOrder->OrderSubmitStatus = pOrder->OrderSubmitStatus;
    ptrNewOrder->NotifySequence = pOrder->NotifySequence;
    strcpy(ptrNewOrder->TradingDay,pOrder->TradingDay);
    ptrNewOrder->SettlementID = pOrder->SettlementID;
    strcpy(ptrNewOrder->OrderSysID,pOrder->OrderSysID);
    ptrNewOrder->OrderSource = pOrder->OrderSource;
    ptrNewOrder->OrderStatus = pOrder->OrderStatus;
    ptrNewOrder->OrderType = pOrder->OrderType;
    ptrNewOrder->VolumeTraded = pOrder->VolumeTraded;
    ptrNewOrder->VolumeTotal = pOrder->VolumeTotal;
    strcpy(ptrNewOrder->InsertDate,pOrder->InsertDate);
    strcpy(ptrNewOrder->InsertTime,pOrder->InsertTime);
    strcpy(ptrNewOrder->ActiveTime,pOrder->ActiveTime);
    strcpy(ptrNewOrder->SuspendTime,pOrder->SuspendTime);
    strcpy(ptrNewOrder->UpdateTime,pOrder->UpdateTime);
    strcpy(ptrNewOrder->CancelTime,pOrder->CancelTime);
    strcpy(ptrNewOrder->ActiveTraderID,pOrder->ActiveTraderID);
    strcpy(ptrNewOrder->ClearingPartID,pOrder->ClearingPartID);
    ptrNewOrder->SequenceNo = pOrder->SequenceNo;
    ptrNewOrder->FrontID = pOrder->FrontID;
    ptrNewOrder->SessionID = pOrder->SessionID;
    strcpy(ptrNewOrder->UserProductInfo,pOrder->UserProductInfo);
    strcpy(ptrNewOrder->StatusMsg,pOrder->StatusMsg);
    ptrNewOrder->UserForceClose = pOrder->UserForceClose;
    strcpy(ptrNewOrder->ActiveUserID,pOrder->ActiveUserID);
    ptrNewOrder->BrokerOrderSeq = pOrder->BrokerOrderSeq;
    strcpy(ptrNewOrder->RelativeOrderSysID,pOrder->RelativeOrderSysID);
    ptrNewOrder->ZCETotalTradedVolume = pOrder->ZCETotalTradedVolume;
    ptrNewOrder->IsSwapOrder = pOrder->IsSwapOrder;

    emit sigOnRtnOrder(ptrNewOrder);
}

void CCTPTraderSpi::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    CThostFtdcTradeField* ptrNewTrade = new CThostFtdcTradeField;
    memset(ptrNewTrade,0,sizeof(*ptrNewTrade));
    strcpy(ptrNewTrade->BrokerID,pTrade->BrokerID);
    strcpy(ptrNewTrade->InvestorID,pTrade->InvestorID);
    strcpy(ptrNewTrade->InstrumentID,pTrade->InstrumentID);
    strcpy(ptrNewTrade->OrderRef,pTrade->OrderRef);
    strcpy(ptrNewTrade->UserID,pTrade->UserID);
    strcpy(ptrNewTrade->ExchangeID,pTrade->ExchangeID);
    strcpy(ptrNewTrade->TradeID,pTrade->TradeID);
    ptrNewTrade->Direction = pTrade->Direction;
    strcpy(ptrNewTrade->OrderSysID,pTrade->OrderSysID);
    strcpy(ptrNewTrade->ParticipantID,pTrade->ParticipantID);
    strcpy(ptrNewTrade->ClientID,pTrade->ClientID);
    ptrNewTrade->TradingRole = pTrade->TradingRole;
    strcpy(ptrNewTrade->ExchangeInstID,pTrade->ExchangeInstID);
    ptrNewTrade->OffsetFlag = pTrade->OffsetFlag;
    ptrNewTrade->HedgeFlag = pTrade->HedgeFlag;
    ptrNewTrade->Price = pTrade->Price;
    ptrNewTrade->Volume = pTrade->Volume;
    strcpy(ptrNewTrade->TradeDate,pTrade->TradeDate);
    strcpy(ptrNewTrade->TradeTime,pTrade->TradeTime);
    ptrNewTrade->TradeType = pTrade->TradeType;
    ptrNewTrade->PriceSource = pTrade->PriceSource;
    strcpy(ptrNewTrade->TraderID,pTrade->TraderID);
    strcpy(ptrNewTrade->OrderLocalID,pTrade->OrderLocalID);
    strcpy(ptrNewTrade->ClearingPartID,pTrade->ClearingPartID);
    strcpy(ptrNewTrade->BusinessUnit,pTrade->BusinessUnit);
    ptrNewTrade->SequenceNo = pTrade->SequenceNo;
    strcpy(ptrNewTrade->TradingDay,pTrade->TradingDay);
    ptrNewTrade->SettlementID = pTrade->SettlementID;
    ptrNewTrade->BrokerOrderSeq = pTrade->BrokerOrderSeq;
    ptrNewTrade->TradeSource = pTrade->TradeSource;

    emit sigOnRtnTrade(ptrNewTrade);
}

void CCTPTraderSpi::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if(pRspInfo && !pRspInfo->ErrorID && bIsLast)
    {
        CThostFtdcInputOrderActionField* ptrNewInpOrderAction = new CThostFtdcInputOrderActionField;
        memset(ptrNewInpOrderAction,0,sizeof(ptrNewInpOrderAction));
        strcpy(ptrNewInpOrderAction->BrokerID,pInputOrderAction->BrokerID);
        strcpy(ptrNewInpOrderAction->InvestorID,pInputOrderAction->InvestorID);
        ptrNewInpOrderAction->OrderActionRef = pInputOrderAction->OrderActionRef;
        strcpy(ptrNewInpOrderAction->OrderRef,pInputOrderAction->OrderRef);
        ptrNewInpOrderAction->RequestID = pInputOrderAction->RequestID;
        ptrNewInpOrderAction->FrontID = pInputOrderAction->FrontID;
        ptrNewInpOrderAction->SessionID = pInputOrderAction->SessionID;
        strcpy(ptrNewInpOrderAction->ExchangeID,pInputOrderAction->ExchangeID);
        strcpy(ptrNewInpOrderAction->OrderSysID,pInputOrderAction->OrderSysID);
        ptrNewInpOrderAction->ActionFlag = pInputOrderAction->ActionFlag;
        ptrNewInpOrderAction->LimitPrice = pInputOrderAction->LimitPrice;
        ptrNewInpOrderAction->VolumeChange = pInputOrderAction->VolumeChange;
        strcpy(ptrNewInpOrderAction->UserID,pInputOrderAction->UserID);
        strcpy(ptrNewInpOrderAction->InstrumentID,pInputOrderAction->InstrumentID);

        emit sigOnRspOrderAction(ptrNewInpOrderAction);
    }
    else if(pRspInfo->ErrorID)
    {
        printf("Something error in OnRspOrderAction, ErrorID = %d, ErrorMsg = %s",pRspInfo->ErrorID,pRspInfo->ErrorMsg);
    }
}

void CCTPTraderSpi::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo)
{
    CThostFtdcOrderActionField* ptrNewOrderAction = new CThostFtdcOrderActionField();
    memset(ptrNewOrderAction,0,sizeof(*ptrNewOrderAction));
    strcpy(ptrNewOrderAction->BrokerID,pOrderAction->BrokerID);
    strcpy(ptrNewOrderAction->InvestorID,pOrderAction->InvestorID);
    ptrNewOrderAction->OrderActionRef = pOrderAction->OrderActionRef;
    strcpy(ptrNewOrderAction->OrderRef,pOrderAction->OrderRef);
    ptrNewOrderAction->RequestID = pOrderAction->RequestID;
    ptrNewOrderAction->FrontID = pOrderAction->FrontID;
    ptrNewOrderAction->SessionID = pOrderAction->SessionID;
    strcpy(ptrNewOrderAction->ExchangeID,pOrderAction->ExchangeID);
    strcpy(ptrNewOrderAction->OrderSysID,pOrderAction->OrderSysID);
    ptrNewOrderAction->ActionFlag = pOrderAction->ActionFlag;
    ptrNewOrderAction->LimitPrice = pOrderAction->LimitPrice;
    ptrNewOrderAction->VolumeChange = pOrderAction->VolumeChange;
    strcpy(ptrNewOrderAction->ActionDate,pOrderAction->ActionDate);
    strcpy(ptrNewOrderAction->ActionTime,pOrderAction->ActionTime);
    strcpy(ptrNewOrderAction->TraderID,pOrderAction->TraderID);
    ptrNewOrderAction->InstallID = pOrderAction->InstallID;
    strcpy(ptrNewOrderAction->OrderLocalID,pOrderAction->OrderLocalID);
    strcpy(ptrNewOrderAction->ActionLocalID,pOrderAction->ActionLocalID);
    strcpy(ptrNewOrderAction->ParticipantID,pOrderAction->ParticipantID);
    strcpy(ptrNewOrderAction->ClientID,pOrderAction->ClientID);
    strcpy(ptrNewOrderAction->BusinessUnit,pOrderAction->BusinessUnit);
    ptrNewOrderAction->OrderActionStatus = pOrderAction->OrderActionStatus;
    strcpy(ptrNewOrderAction->UserID,pOrderAction->UserID);
    strcpy(ptrNewOrderAction->StatusMsg,pOrderAction->StatusMsg);
    strcpy(ptrNewOrderAction->InstrumentID,pOrderAction->InstrumentID);

    emit sigOnErrRtnOrderAction(ptrNewOrderAction);
}

void CCTPTraderSpi::OnRegisterFront()
{
    OnFrontConnected();
}

void CCTPTraderSpi::OnUserLogin(int nRequestID)
{
    CThostFtdcRspUserLoginField LoginField;
    memset(&LoginField,0,sizeof(LoginField));
    strcpy(LoginField.TradingDay,QDate::currentDate().toString("yyyyMMdd").toStdString().c_str());
    strcpy(LoginField.LoginTime,QTime::currentTime().toString("HH:mm:ss").toStdString().c_str());
    strcpy(LoginField.BrokerID,m_szBrokerID);
    strcpy(LoginField.UserID,m_szInvestorID);
    LoginField.FrontID = 0;
    LoginField.SessionID = 0;
    snprintf(LoginField.MaxOrderRef,sizeof(LoginField.MaxOrderRef),"%d",0);

    CThostFtdcRspInfoField RspInfoField;
    RspInfoField.ErrorID = 0;
    strcpy(RspInfoField.ErrorMsg,"TradeAPI user login succeeded.");

    OnRspUserLogin(&LoginField,&RspInfoField,nRequestID,true);
}

void CCTPTraderSpi::OnUserLogout(int nRequestID)
{
    CThostFtdcUserLogoutField LogoutField;
    memset(&LogoutField,0,sizeof(LogoutField));
    strcpy(LogoutField.BrokerID,m_szBrokerID);
    strcpy(LogoutField.UserID,m_szInvestorID);

    CThostFtdcRspInfoField RspInfoField;
    RspInfoField.ErrorID = 0;
    strcpy(RspInfoField.ErrorMsg,"TradeAPI user logout succeeded.");

    OnRspUserLogout(&LogoutField,&RspInfoField,nRequestID,true);
}

void CCTPTraderSpi::OnOrderInsert(CThostFtdcInputOrderField *ptrInputOrder, int nRequestID)
{
    int nOrderRef = atoi(ptrInputOrder->OrderRef);
    QMutexLocker locker(&m_lock);
    if(!m_mpOrderPool->contains(nOrderRef))     // 如果订单的OrderRef已存在，则不对该订单进行响应和撮合
    {
        // 先响应订单：生成CThostFtdcOrderField，调用sigOnRtnOrder
        CThostFtdcOrderField *ptrOrder = new CThostFtdcOrderField;
        memset(ptrOrder,0,sizeof(*ptrOrder));
        strcpy(ptrOrder->BrokerID,m_szBrokerID);
        strcpy(ptrOrder->InvestorID,m_szInvestorID);
        strcpy(ptrOrder->InstrumentID,ptrInputOrder->InstrumentID);
        strcpy(ptrOrder->OrderRef,ptrInputOrder->OrderRef);
        strcpy(ptrOrder->UserID,m_szInvestorID);
        ptrOrder->OrderPriceType = ptrInputOrder->OrderPriceType;
        ptrOrder->Direction = ptrInputOrder->Direction;
        strcpy(ptrOrder->CombOffsetFlag,ptrInputOrder->CombOffsetFlag);
        strcpy(ptrOrder->CombHedgeFlag,ptrInputOrder->CombHedgeFlag);
        ptrOrder->LimitPrice = ptrInputOrder->LimitPrice;
        ptrOrder->VolumeTotalOriginal = ptrInputOrder->VolumeTotalOriginal;
        ptrOrder->TimeCondition = ptrInputOrder->TimeCondition;
        strcpy(ptrOrder->GTDDate,ptrInputOrder->GTDDate);
        ptrOrder->VolumeCondition = ptrInputOrder->VolumeCondition;
        ptrOrder->MinVolume = ptrInputOrder->MinVolume;
        ptrOrder->ContingentCondition = ptrInputOrder->ContingentCondition;
        ptrOrder->StopPrice = ptrInputOrder->StopPrice;
        ptrOrder->RequestID = ptrInputOrder->RequestID;
        ptrOrder->OrderSubmitStatus = THOST_FTDC_OSS_Accepted;  // 报单已经接受
        ptrOrder->OrderStatus = THOST_FTDC_OST_NoTradeQueueing; // 未成交还在队列中
        ptrOrder->VolumeTraded = 0;
        ptrOrder->VolumeTotal = ptrInputOrder->VolumeTotalOriginal;
        strcpy(ptrOrder->InsertDate,m_tmTradingDay.toString("yyyyMMdd").toStdString().c_str());
        ptrOrder->FrontID = m_nFrontID;
        ptrOrder->SessionID = m_nSessionID;
        strcpy(ptrOrder->StatusMsg,"Order has been accepted.");

        sigOnRtnOrder(ptrOrder);

        // 再对订单进行即时撮合，如果订单成交，则不用插入订单池
        if(MatchOrder_OnInsertOrder(ptrInputOrder))
        {
            delete ptrInputOrder;   // 如果订单撮合成交，那么释放订单内存
        }
        else
        {// 如果订单不成交，则插入订单池
            m_mpOrderPool->insert(nOrderRef,ptrInputOrder);
        }
    }
}

void CCTPTraderSpi::OnOrderAction(CThostFtdcInputOrderActionField *ptrOrderAction, int nRequestID)
{
    QMutexLocker locker(&m_lock);
    // 只处理撤单情况
    int nOrderRef = atoi(ptrOrderAction->OrderRef);
    if(THOST_FTDC_AF_Delete == ptrOrderAction->ActionFlag)
    {
        // 如果订单池中存在该订单，那么进行撤单，并调用OnRtnOrder
        if(m_mpOrderPool->contains(nOrderRef))
        {
            CThostFtdcInputOrderField *ptrOrder = m_mpOrderPool->value(nOrderRef);
            CThostFtdcOrderField *ptrCancelOrder = new CThostFtdcOrderField;
            memset(ptrCancelOrder,0,sizeof(*ptrCancelOrder));
            strcpy(ptrCancelOrder->BrokerID,m_szBrokerID);
            strcpy(ptrCancelOrder->InvestorID,m_szInvestorID);
            strcpy(ptrCancelOrder->InstrumentID,ptrOrder->InstrumentID);
            strcpy(ptrCancelOrder->OrderRef,ptrOrder->OrderRef);
            strcpy(ptrCancelOrder->UserID,m_szInvestorID);
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
            strcpy(ptrCancelOrder->TradingDay,m_tmTradingDay.toString("yyyyMMdd").toStdString().c_str());
            ptrCancelOrder->OrderStatus = THOST_FTDC_OST_Canceled;      // 撤单
            ptrCancelOrder->VolumeTraded = 0;
            ptrCancelOrder->VolumeTotal = ptrOrder->VolumeTotalOriginal;
            strcpy(ptrCancelOrder->InsertDate,m_tmTradingDay.toString("yyyyMMdd").toStdString().c_str());
            ptrCancelOrder->FrontID = m_nFrontID;
            ptrCancelOrder->SessionID = m_nSessionID;
            strcpy(ptrCancelOrder->StatusMsg,"Order has been canceled.");

            OnRtnOrder(ptrCancelOrder);

            delete m_mpOrderPool->value(nOrderRef);
            m_mpOrderPool->remove(nOrderRef);
        }
        else
        {
            // 如果订单池中没有该订单，说明该订单已经成交或撤单，调用OnErrRtnOrderAction
            CThostFtdcOrderActionField *ptrNewOrderAction = new CThostFtdcOrderActionField;
            memset(ptrNewOrderAction,0,sizeof(*ptrNewOrderAction));
            strcpy(ptrNewOrderAction->BrokerID,ptrOrderAction->BrokerID);
            strcpy(ptrNewOrderAction->InvestorID,ptrOrderAction->InvestorID);
            ptrNewOrderAction->OrderActionRef = ptrOrderAction->OrderActionRef;
            strcpy(ptrNewOrderAction->OrderRef,ptrOrderAction->OrderRef);
            ptrNewOrderAction->RequestID = ptrOrderAction->RequestID;
            ptrNewOrderAction->FrontID = m_nFrontID;
            ptrNewOrderAction->SessionID = m_nSessionID;
            ptrNewOrderAction->ActionFlag = ptrOrderAction->ActionFlag;
            strcpy(ptrNewOrderAction->ActionDate,m_tmTradingDay.toString("yyyyMMdd").toStdString().c_str());
            ptrNewOrderAction->OrderActionStatus = THOST_FTDC_OAS_Rejected;     // 撤单被拒绝
            strcpy(ptrNewOrderAction->StatusMsg,"Cancel order has been rejected.");
            strcpy(ptrNewOrderAction->InstrumentID,ptrOrderAction->InstrumentID);

            CThostFtdcRspInfoField RspInfo;
            RspInfo.ErrorID = 1;
            strcpy(RspInfo.ErrorMsg,"Cancel order has been rejected.");

            OnErrRtnOrderAction(ptrNewOrderAction,&RspInfo);
        }

    }
}

void CCTPTraderSpi::OnSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *ptrSttlInfoConfirm, int nRequestID)
{
    CThostFtdcRspInfoField RspInfo;
    RspInfo.ErrorID = 1;
    strcpy(RspInfo.ErrorMsg,"Settlemet info has confirmed.");

    OnRspSettlementInfoConfirm(ptrSttlInfoConfirm,&RspInfo,nRequestID,true);
}

void CCTPTraderSpi::OnMatchOder()
{
    // 遍历订单池，对订单进行撮合，如果成交，则释放订单内存
    QMap<int,CThostFtdcInputOrderField*>::iterator it = m_mpOrderPool->begin();
    while(it != m_mpOrderPool->end())
    {
        if(MatchOrder_OnTraverseOrder(it.value()))
        {
            delete it.value();
            it = m_mpOrderPool->erase(it);
        }
        else
            ++it;
    }
}

void CCTPTraderSpi::loadTradingDays()
{
    QFile fileTradingDays("TradingDay.csv");
    if(!fileTradingDays.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&fileTradingDays);

    m_vTradingDays.clear();
    while(!in.atEnd())
    {
        QString strTradingDay = in.readLine();
        QDate tmTradingDay = QDate::fromString(strTradingDay,"yyyy-MM-dd");
        if(tmTradingDay >= m_tmBegDate && tmTradingDay <= m_tmEndDate)
            m_vTradingDays.push_back(tmTradingDay);
    }
//    m_tmTradingDay = m_vTradingDays.front();
    if(!m_vTradingDays.empty())
    {
        InitTradeInfo(m_vTradingDays.front());      // 初始化交易信息
        m_itTradingDay = m_vTradingDays.begin();    // 交易日期迭代器指向日期列表第一个元素
    }
    fileTradingDays.close();
}

bool CCTPTraderSpi::MatchOrder_OnInsertOrder(CThostFtdcInputOrderField *ptrOrder)
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

    if(bIsMatched)
    {// 撮合成交，依次发出sigOnRtnOrder和sigOnRtnTrade信号
        CThostFtdcOrderField *ptrNewOrder = new CThostFtdcOrderField;
        memset(ptrNewOrder,0,sizeof(*ptrNewOrder));
        strcpy(ptrNewOrder->BrokerID,m_szBrokerID);
        strcpy(ptrNewOrder->InvestorID,m_szInvestorID);
        strcpy(ptrNewOrder->InstrumentID,ptrOrder->InstrumentID);
        strcpy(ptrNewOrder->OrderRef,ptrOrder->OrderRef);
        strcpy(ptrNewOrder->UserID,m_szInvestorID);
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
        strcpy(ptrNewOrder->InsertDate,m_tmTradingDay.toString("yyyyMMdd").toStdString().c_str());
        ptrNewOrder->FrontID = m_nFrontID;
        ptrNewOrder->SessionID = m_nSessionID;
        strcpy(ptrNewOrder->StatusMsg,"Order has been traded.");
        sigOnRtnOrder(ptrNewOrder);

        CThostFtdcTradeField *ptrTrade = new CThostFtdcTradeField;
        strcpy(ptrTrade->BrokerID,m_szBrokerID);
        strcpy(ptrTrade->InvestorID,m_szInvestorID);
        strcpy(ptrTrade->InstrumentID,ptrOrder->InstrumentID);
        strcpy(ptrTrade->OrderRef,ptrOrder->OrderRef);
        strcpy(ptrTrade->UserID,m_szInvestorID);
        ptrTrade->Direction = ptrOrder->Direction;
        ptrTrade->OffsetFlag = ptrOrder->CombOffsetFlag[0];
        ptrTrade->HedgeFlag = ptrOrder->CombHedgeFlag[0];
        ptrTrade->Price = fTradePrice;
        ptrTrade->Volume = ptrOrder->VolumeTotalOriginal;
        strcpy(ptrTrade->TradeDate,m_tmTradingDay.toString("yyyyMMdd").toStdString().c_str());

        CalcPnL(ptrTrade);  // 计算P&L
        sigOnRtnTrade(ptrTrade);
    }

    return bIsMatched;
}

bool CCTPTraderSpi::MatchOrder_OnTraverseOrder(CThostFtdcInputOrderField *ptrOrder)
{
    bool bIsMatched = false;    // 是否撮合成交
    double fTradePrice = 0.0;   // 成交价
    QString strSecuCode = QString(ptrOrder->InstrumentID);
    if(THOST_FTDC_D_Buy == ptrOrder->CombOffsetFlag[0])
    {// 买入委托，委托价大于卖一价时，成交
        double fAskPrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(strSecuCode).AskPrice;
        double fLast = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(strSecuCode).Last;
//        if(ptrOrder->LimitPrice > fAskPrice || fabs(ptrOrder->LimitPrice - fAskPrice) < EPSILON)
        if(ptrOrder->LimitPrice - fAskPrice > EPSILON)
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
//        if(fBidPrice > ptrOrder->LimitPrice || fabs(fBidPrice - ptrOrder->LimitPrice) < EPSILON)
        if(fBidPrice - ptrOrder->LimitPrice > EPSILON)
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

    if(bIsMatched)
    {// 撮合成交，依次发出sigOnRtnOrder和sigOnRtnTrade信号
        CThostFtdcOrderField *ptrNewOrder = new CThostFtdcOrderField;
        memset(ptrNewOrder,0,sizeof(*ptrNewOrder));
        strcpy(ptrNewOrder->BrokerID,m_szBrokerID);
        strcpy(ptrNewOrder->InvestorID,m_szInvestorID);
        strcpy(ptrNewOrder->InstrumentID,ptrOrder->InstrumentID);
        strcpy(ptrNewOrder->OrderRef,ptrOrder->OrderRef);
        strcpy(ptrNewOrder->UserID,m_szInvestorID);
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
        strcpy(ptrNewOrder->InsertDate,m_tmTradingDay.toString("yyyyMMdd").toStdString().c_str());
        ptrNewOrder->FrontID = m_nFrontID;
        ptrNewOrder->SessionID = m_nSessionID;
        strcpy(ptrNewOrder->StatusMsg,"Order has been traded.");
        sigOnRtnOrder(ptrNewOrder);

        CThostFtdcTradeField *ptrTrade = new CThostFtdcTradeField;
        strcpy(ptrTrade->BrokerID,m_szBrokerID);
        strcpy(ptrTrade->InvestorID,m_szInvestorID);
        strcpy(ptrTrade->InstrumentID,ptrOrder->InstrumentID);
        strcpy(ptrTrade->OrderRef,ptrOrder->OrderRef);
        strcpy(ptrTrade->UserID,m_szInvestorID);
        ptrTrade->Direction = ptrOrder->Direction;
        ptrTrade->OffsetFlag = ptrOrder->CombOffsetFlag[0];
        ptrTrade->HedgeFlag = ptrOrder->CombHedgeFlag[0];
        ptrTrade->Price = fTradePrice;
        ptrTrade->Volume = ptrOrder->VolumeTotalOriginal;
        strcpy(ptrTrade->TradeDate,m_tmTradingDay.toString("yyyyMMdd").toStdString().c_str());

        CalcPnL(ptrTrade);  // 计算P&L
        sigOnRtnTrade(ptrTrade);
    }

    return bIsMatched;
}

void CCTPTraderSpi::InitTradeInfo(QDate _tmTradingDay)
{
    QMutexLocker locker(&m_lock);

    m_tmTradingDay = _tmTradingDay;
    if(nullptr != m_mpOrderPool)
    {
        for(QMap<int,CThostFtdcInputOrderField*>::iterator it = m_mpOrderPool->begin();it != m_mpOrderPool->end();++it)
        {
            delete it.value();
        }
        m_mpOrderPool->clear();
    }
    else
        m_mpOrderPool = new QMap<int,CThostFtdcInputOrderField*>();
}

void CCTPTraderSpi::OnSavePnL()
{
    // 向日志类发信号，保存P&L值
    QString strPnLData = QString("%1,%2").arg(m_tmTradingDay.toString("yyyy-MM-dd")).arg(m_fPnL);
    emit sigOnSavePnL(m_ptrPnLFile,strPnLData);
    // 交易日期迭代器递增
    if(m_itTradingDay != m_vTradingDays.end())
    {
        m_tmTradingDay = *(++m_itTradingDay);
        // 初始化交易信息
        InitTradeInfo(m_tmTradingDay);
        // 发信号给CCTPMdSpi,每天初始化播放的行情数据
        emit sigSwitchToNextTradingDay(m_tmTradingDay);
    }
    else
    {// 如果交易日期向量遍历完毕，发出回测结束信号
        emit sigBackTestFinished();
    }
}

void CCTPTraderSpi::CalcPnL(const CThostFtdcTradeField *ptrTrade)
{
    if(ptrTrade == nullptr)
        return;

    QMutexLocker locker(&m_lock);
    switch(ptrTrade->Direction)
    {
    case THOST_FTDC_D_Buy:
        m_fPnL -= ptrTrade->Volume * ptrTrade->Price * MULTIPLIER;
        break;
    case THOST_FTDC_D_Sell:
        m_fPnL += ptrTrade->Volume * ptrTrade->Price * MULTIPLIER;
        break;
    }
}

} // namespace HESS
