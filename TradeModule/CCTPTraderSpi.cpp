#include <iostream>
#include <QCoreApplication>
#include <QDate>
#include <QTime>
#include <QMutexLocker>
#include <QTextStream>
#include <math.h>
#include <cstring>
#include <stdio.h>
#include "CCTPTraderSpi.h"
#include "Utility/CRequestID.h"
#include "Utility/COrderRef.h"
#include "MarketData/CMktDataBuffer.h"
#include "Utility/Utilities.h"

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

} // namespace HESS
