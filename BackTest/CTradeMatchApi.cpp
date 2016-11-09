#include "CTradeMatchApi.h"

namespace HESS
{

void CTradeMatchApi::RegisterFront(char *pszFrontAddress)
{
    emit sigRegisterFront();
}

int CTradeMatchApi::ReqUserLogin(CThostFtdcReqUserLoginField *pReqUserLoginField, int nRequestID)
{
    emit sigUserLogin(nRequestID);
    return 0;
}

int CTradeMatchApi::ReqUserLogout(CThostFtdcUserLogoutField *pUserLogout, int nRequestID)
{
    emit sigUserLogout(nRequestID);
    return 0;
}

int CTradeMatchApi::ReqOrderInsert(CThostFtdcInputOrderField *pInputOrder, int nRequestID)
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
    strcpy(ptrNewOrder->BusinessUnit,pInputOrder->BusinessUnit);
    ptrNewOrder->RequestID = pInputOrder->RequestID;
    ptrNewOrder->UserForceClose = pInputOrder->UserForceClose;
    ptrNewOrder->IsSwapOrder = pInputOrder->IsSwapOrder;
    strcpy(ptrNewOrder->ExchangeID,pInputOrder->ExchangeID);
    strcpy(ptrNewOrder->InvestUnitID,pInputOrder->InvestUnitID);
    strcpy(ptrNewOrder->AccountID,pInputOrder->AccountID);
    strcpy(ptrNewOrder->CurrencyID,pInputOrder->CurrencyID);
    strcpy(ptrNewOrder->ClientID,pInputOrder->ClientID);
    strcpy(ptrNewOrder->IPAddress,pInputOrder->IPAddress);
    strcpy(ptrNewOrder->MacAddress,pInputOrder->MacAddress);

    emit sigOrderInsert(ptrNewOrder,nRequestID);
    return 0;
}

int CTradeMatchApi::ReqOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, int nRequestID)
{
    CThostFtdcInputOrderActionField *ptrNewAction = new CThostFtdcInputOrderActionField;
    memset(ptrNewAction,0,sizeof(*ptrNewAction));
    strcpy(ptrNewAction->BrokerID,pInputOrderAction->BrokerID);
    strcpy(ptrNewAction->InvestorID,pInputOrderAction->InvestorID);
    ptrNewAction->OrderActionRef = pInputOrderAction->OrderActionRef;
    strcpy(ptrNewAction->OrderRef,pInputOrderAction->OrderRef);
    ptrNewAction->RequestID = pInputOrderAction->RequestID;
    ptrNewAction->FrontID = pInputOrderAction->FrontID;
    ptrNewAction->SessionID = pInputOrderAction->SessionID;
    strcpy(ptrNewAction->ExchangeID,pInputOrderAction->ExchangeID);
    strcpy(ptrNewAction->OrderSysID,pInputOrderAction->OrderSysID);
    ptrNewAction->ActionFlag = pInputOrderAction->ActionFlag;
    ptrNewAction->LimitPrice = pInputOrderAction->LimitPrice;
    ptrNewAction->VolumeChange = pInputOrderAction->VolumeChange;
    strcpy(ptrNewAction->UserID,pInputOrderAction->UserID);
    strcpy(ptrNewAction->InstrumentID,pInputOrderAction->InstrumentID);
    strcpy(ptrNewAction->InvestUnitID,pInputOrderAction->InvestUnitID);
    strcpy(ptrNewAction->IPAddress,pInputOrderAction->IPAddress);
    strcpy(ptrNewAction->MacAddress,pInputOrderAction->MacAddress);

    emit sigOrderAction(ptrNewAction,nRequestID);
    return 0;
}

int CTradeMatchApi::ReqSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, int nRequestID)
{
    CThostFtdcSettlementInfoConfirmField *ptrSttlInfoConfirm = new CThostFtdcSettlementInfoConfirmField;
    memset(ptrSttlInfoConfirm,0,sizeof(*ptrSttlInfoConfirm));
    strcpy(ptrSttlInfoConfirm->BrokerID,pSettlementInfoConfirm->BrokerID);
    strcpy(ptrSttlInfoConfirm->InvestorID,pSettlementInfoConfirm->InvestorID);
    strcpy(ptrSttlInfoConfirm->ConfirmDate,pSettlementInfoConfirm->ConfirmDate);
    strcpy(ptrSttlInfoConfirm->ConfirmTime,pSettlementInfoConfirm->ConfirmTime);

//    emit sigSettlementInfoConfirm(ptrSttlInfoConfirm,nRequestID);
    return 0;
}

}
