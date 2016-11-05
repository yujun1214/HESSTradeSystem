#include "CTFMarketMaker.h"
#include "MarketData/CMktDataBuffer.h"
#include "Utility/CRequestID.h"
#include "Utility/COrderRef.h"
#include <cstring>
#include <stdio.h>
#include "Utility/Utilities.h"
#include <iostream>

namespace HESS
{

void CTFMarketMaker::handleUpdateMktData(QString _strSecuCode)
{
    if(_strSecuCode != m_strSecuCode)
        return;
//    std::cout << "In handleUpdateMktData..." << std::endl;
    // 根据trading status决定不同处理方式
    switch(m_eTradingStatus)
    {
    case EMPTYHOLDING:          // 空仓
        // 当前交易时间状态为“正常交易”时，才进行委托
        if(CTimeController::NORMALTRADING == m_ptrTimeController->CurrTimeStatus())
            submitOpenPositionOrder();
        break;
    case SENDEDPOSITIONORDER:   // 已发出建仓委托
        // 如果当前交易时间状态为“收盘缓冲期”或“收盘平仓期”时，进行撤单
        if(CTimeController::PRECLOSINGCUSHION == m_ptrTimeController->CurrTimeStatus() ||
           CTimeController::PRECLOSINGLIQUIDATION == m_ptrTimeController->CurrTimeStatus())
        {
            submitCancelOrder(m_nTradeOrderRef);
            m_eTradingStatus = SENDEDCANCELPOSITIONORDER;
        }
        // 如果当前的报价超出了intervals的范围，进行撤单
        if(LONGTRADING == m_eTradingType)
        {
            if(m_ptrMMDispatcher->exceedIntervalRange(0,m_nOrderIndex))
            {
                submitCancelOrder(m_nTradeOrderRef);
                m_eTradingStatus = SENDEDCANCELPOSITIONORDER;
            }
        }
        else if(SHORTTRADING == m_eTradingType)
        {
            if(m_ptrMMDispatcher->exceedIntervalRange(1,m_nOrderIndex))
            {
                submitCancelOrder(m_nTradeOrderRef);
                m_eTradingStatus = SENDEDCANCELPOSITIONORDER;
            }
        }
        break;
    case SENDEDCANCELPOSITIONORDER: // 已发出建仓委托撤单请求
        break;
    case POSITIONED:            // 建仓完成
        submitClosePositionOrder();
        break;
    case SENDEDLIQUIDATIONORDER:    // 已发出平仓委托
        if(CTimeController::NORMALTRADING == m_ptrTimeController->CurrTimeStatus() ||
           CTimeController::PRECLOSINGCUSHION == m_ptrTimeController->CurrTimeStatus())
        {// 如果当前交易时间状态是“正常交易”或“收盘缓冲期”，那么处理止损情况
            handleStoploss();
        }
        else if(CTimeController::PRECLOSINGLIQUIDATION == m_ptrTimeController->CurrTimeStatus())
        {// 如果当前交易时间状态是“收盘平仓期”，那么撤单
            submitCancelOrder(m_nTradeOrderRef);
            m_eTradingStatus = SENDEDCANCELLIQUIDATIONORDER;
        }
        break;
    case SENDEDCANCELLIQUIDATIONORDER:  // 已发出平仓委托撤单请求
        break;
    default:
        break;
    }

    // 发出行情处理完毕signal(for backtest)
    emit sigHandleMktDataDone();
} // CTFMarketMaker::handleUpdateMktData

void CTFMarketMaker::handleRtnOrder(CThostFtdcOrderField *_ptrOrder)
{
//    if(m_nTradeOrderRef != _nOrderRef)
//        return;
    if(m_nTradeOrderRef != atoi(_ptrOrder->OrderRef))
        return;
    if(QString(QLatin1String(_ptrOrder->InstrumentID)) != m_strSecuCode)
        return;

    // 根据报单提交状态分别处理
    switch(_ptrOrder->OrderSubmitStatus)
    {
    case THOST_FTDC_OSS_InsertSubmitted:    // 已经提交
        break;
    case THOST_FTDC_OSS_CancelSubmitted:    // 撤单已经提交
        break;
    case THOST_FTDC_OSS_ModifySubmitted:    // 修改已经提交
        break;
    case THOST_FTDC_OSS_Accepted:           // 已经接受
        break;
    case THOST_FTDC_OSS_InsertRejected:     // 报单已经被拒绝
        if(SENDEDPOSITIONORDER == m_eTradingStatus)
        {// 如果当前交易状态 = “已发出建仓委托”，将交易状态改为“空仓”，发出“开仓委托被拒绝”signal
            m_eTradingStatus = EMPTYHOLDING;
//            emit sigOpenOrderDenied(_ptrOrder->LimitPrice);
            emit sigOpenOrderDenied(m_nOrderIndex);
        }
        else if(SENDEDLIQUIDATIONORDER == m_eTradingStatus)
        {// 如果当前交易状态 = “已发出平仓委托”，将交易状态改为“建仓完成”，发出“平仓委托被拒绝”signal
            m_eTradingStatus = POSITIONED;
//            emit sigCloseOrderDenied(_ptrOrder->LimitPrice);
            emit sigCloseOrderDenied(m_nOrderIndex);
        }
        break;
    case THOST_FTDC_OSS_CancelRejected:     // 撤单已经被拒绝
        // 撤单被拒绝后的后续处理在OnRspOrderAction中处理
        break;
    case THOST_FTDC_OSS_ModifyRejected:     // 改单已经被拒绝
        break;
    default:
        break;
    }

    // 根据报单状态分别处理
    switch(_ptrOrder->OrderStatus)
    {
    case THOST_FTDC_OST_AllTraded:      // 全部成交
        //  不做处理，在RtnTrade中进行处理
        break;
    case THOST_FTDC_OST_PartTradedQueueing: // 部分成交还在队列中
        // 不做处理
        break;
    case THOST_FTDC_OST_PartTradedNotQueueing:  // 部分成交不在队列中
        // 不做处理
        break;
    case THOST_FTDC_OST_NoTradeQueueing:        // 未成交还在队列中
        // 不做处理
        break;
    case THOST_FTDC_OST_NoTradeNotQueueing:     // 未成交不在队列中
        // 不做处理
        break;
    case THOST_FTDC_OST_Canceled:       // 撤单
        if(SENDEDCANCELPOSITIONORDER == m_eTradingStatus)
        {// 当前交易状态如果是“已发出建仓委托撤单请求”，那么将交易状态更新为“空仓”，发出“开仓委托撤单完成”signal
            m_eTradingStatus = EMPTYHOLDING;
//            emit sigOpenOrderCanceled(_ptrOrder->LimitPrice);
            emit sigOpenOrderCanceled(m_nOrderIndex);
        }
        else if(SENDEDCANCELLIQUIDATIONORDER == m_eTradingStatus)
        {// 当前交易状态如果是“已发出平仓委托撤单请求”，那么将交易状态更新为“已发出平仓委托”，并发出止损委托，同时发出“平仓委托撤单完成”signal
            m_eTradingStatus = SENDEDLIQUIDATIONORDER;
            if(LONGTRADING == m_eTradingType)
            {
                sendMktPriceOrder(THOST_FTDC_D_Sell,THOST_FTDC_OF_Close,1);
            }
            else if(SHORTTRADING == m_eTradingType)
            {
                sendMktPriceOrder(THOST_FTDC_D_Buy,THOST_FTDC_OF_Close,1);
            }
//            emit sigCloseOrderCanceled(_ptrOrder->LimitPrice);
            emit sigCloseOrderCanceled(m_nOrderIndex);
        }
    case THOST_FTDC_OST_Unknown:    // 未知
        // 不做处理
        break;
    case THOST_FTDC_OST_NotTouched: // 尚未触发
        // 不做处理
        break;
    case THOST_FTDC_OST_Touched:    // 已触发
        // 不做处理
        break;
    default:
        break;
    }

    // 删除报单指针
    delete _ptrOrder;

} // CTFMarketMaker::handleRtnOrder

void CTFMarketMaker::handleRtnTrade(CThostFtdcTradeField *_ptrTrade)
{
//    if(m_nTradeOrderRef != _nOrderRef)
//        return;
    if(m_nTradeOrderRef != atoi(_ptrTrade->OrderRef))
        return;
    if(QString(QLatin1String(_ptrTrade->InstrumentID)) != m_strSecuCode)
        return;

    if(_ptrTrade->OffsetFlag == THOST_FTDC_OF_Open)
    {// 如果是开仓交易
        if(SENDEDPOSITIONORDER == m_eTradingStatus)
        {// 当前交易状态=“已发出建仓委托”，交易状态更新为“建仓完成”，记录holding cost,发出“开仓完成”signal
            m_eTradingStatus = POSITIONED;
            m_fHoldingCost = _ptrTrade->Price;
//            emit sigPositioned(_ptrTrade->Price);
            emit sigPositioned(m_nOrderIndex);
        }
        else if(SENDEDCANCELPOSITIONORDER == m_eTradingStatus)
        {// 当前交易状态=“已发出建仓委托撤单请求”，即建仓委托撤单order到达市场时该建仓order已经成交，更新交易状态为“建仓完成”，记录holding cost
            m_eTradingStatus = POSITIONED;
            m_fHoldingCost = _ptrTrade->Price;
            // 发出“开仓完成”signal
//            emit sigPositioned(_ptrTrade->Price);
            emit sigPositioned(m_nOrderIndex);
        }
    }
    else if(_ptrTrade->OffsetFlag == THOST_FTDC_OF_Close)
    {// 如果是平仓交易
        if(SENDEDLIQUIDATIONORDER == m_eTradingStatus)
        {// 当前交易状态=“已发出平仓委托”，交易状态更新为“空仓”
            m_eTradingStatus = EMPTYHOLDING;
            // 发出“平仓完成”signal
//            emit sigLiquidationed(_ptrTrade->Price);
            emit sigLiquidationed(m_nOrderIndex);
        }
        else if(SENDEDCANCELLIQUIDATIONORDER == m_eTradingStatus)
        {// 当前交易状态=“已发出平仓委托撤单请求”，即平仓委托撤单order到达市场时该平仓order已经成交，更新交易状态为“空仓”
            m_eTradingStatus = EMPTYHOLDING;
            // 发出“平仓完成”signal
//            emit sigLiquidationed(_ptrTrade->Price);
            emit sigLiquidationed(m_nOrderIndex);
        }
    }

    delete _ptrTrade;

} // CTFMarketMaker::handleRtnTrade

void CTFMarketMaker::handleErrRtnOrderAction(CThostFtdcOrderActionField *_ptrOrderAction)
{
//    if(m_nCancelOrderRef != _nOrderRef)
//        return;
    if(m_nCancelOrderRef != _ptrOrderAction->OrderActionRef)
        return;

    if(THOST_FTDC_OAS_Rejected == _ptrOrderAction->OrderActionStatus)
    {// 撤单委托被拒绝
        if(SENDEDCANCELPOSITIONORDER == m_eTradingStatus)
        {// 当前交易状态=”已发出建仓委托撤单请求“，交易状态更新为”已发出建仓委托“
            m_eTradingStatus = SENDEDPOSITIONORDER;
        }
        else if(SENDEDCANCELLIQUIDATIONORDER == m_eTradingStatus)
        {// 当前交易状态=“已发出平仓委托撤单请求”，交易状态更新为“已发出平仓委托”
            m_eTradingStatus = SENDEDLIQUIDATIONORDER;
        }
    }

    delete _ptrOrderAction;

} // CTFMarketMaker::handleRspOrderAction

// 设置frontid和sessionid
void CTFMarketMaker::setFrontSessionID(TThostFtdcFrontIDType _nFrontID, TThostFtdcSessionIDType _nSessionID)
{
    m_nFrontID = _nFrontID;
    m_nSessionID = _nSessionID;
}

// 开仓委托
void CTFMarketMaker::submitOpenPositionOrder()
{
    std::cout << "Submit Open Position Order..." << std::endl;
    if(LONGTRADING == m_eTradingType)
    {// long trading type
//        double fBuyPrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getDepthMktData(m_strSecuCode).BidPrice[0];
        double fBuyPrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(m_strSecuCode).BidPrice;
        m_ptrMMDispatcher->acquireBuyOrderPrice(m_nOrderIndex,fBuyPrice);
        sendLimitOrder(THOST_FTDC_D_Buy,THOST_FTDC_OF_Open,1,fBuyPrice);
        // 设置交易状态为“已发出建仓委托”
        m_eTradingStatus = SENDEDPOSITIONORDER;
    }
    else if(SHORTTRADING == m_eTradingType)
    {// short trading type
//        double fSellPrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getDepthMktData(m_strSecuCode).AskPrice[0];
        double fSellPrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(m_strSecuCode).AskPrice;
        m_ptrMMDispatcher->acquireSellOrderPrice(m_nOrderIndex,fSellPrice);
        sendLimitOrder(THOST_FTDC_D_Sell,THOST_FTDC_OF_Open,1,fSellPrice);
        // 设置交易状态为“已发出建仓委托”
        m_eTradingStatus = SENDEDPOSITIONORDER;
    }
}

// 平仓委托
void CTFMarketMaker::submitClosePositionOrder()
{
    if(LONGTRADING == m_eTradingType)
    {// long trading type
//        double fSellClosePrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getDepthMktData(m_strSecuCode).AskPrice[0];
        double fSellClosePrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(m_strSecuCode).AskPrice;
        m_ptrMMDispatcher->acquireSellCloseOrderPrice(m_nOrderIndex,fSellClosePrice);
        // 平仓价格至少保证盈利1个tick
        double fMinClosePrice = ((int)(m_fHoldingCost * m_nPricePrecision + 0.5) + m_nPriceTick) / (double)m_nPricePrecision;
        sendLimitOrder(THOST_FTDC_D_Sell,THOST_FTDC_OF_Close,1,fSellClosePrice < fMinClosePrice ? fMinClosePrice:fSellClosePrice);
        // 设置交易状态为“已发出平仓委托”
        m_eTradingStatus = SENDEDLIQUIDATIONORDER;
    }
    else if(SHORTTRADING == m_eTradingType)
    {// short trading tpe
//        double fBuyClosePrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getDepthMktData(m_strSecuCode).BidPrice[0];
        double fBuyClosePrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(m_strSecuCode).BidPrice;
        m_ptrMMDispatcher->acquireBuyCloseOrderPrice(m_nOrderIndex,fBuyClosePrice);
        // 平仓价格至少保证盈利1个tick
        double fMaxClosePrice = ((int)(m_fHoldingCost * m_nPricePrecision + 0.5) - m_nPriceTick) / (double)m_nPricePrecision;
        sendLimitOrder(THOST_FTDC_D_Buy,THOST_FTDC_OF_Close,1,fBuyClosePrice > fMaxClosePrice ? fMaxClosePrice:fBuyClosePrice);
        // 设置交易状态为“已发出平仓委托”
        m_eTradingStatus = SENDEDLIQUIDATIONORDER;
    }
}

// 撤单委托
void CTFMarketMaker::submitCancelOrder(int _nOrderRef)
{
    int nRequestID = CRequestID::getCRequestIDPtr()->getValidRequestID();
    int nOrderActionRef = COrderRef::getCOrderRefPtr()->getValidOrderRef();

    CThostFtdcInputOrderActionField orderActionField;
    memset(&orderActionField,0,sizeof(orderActionField));
    strcpy(orderActionField.BrokerID,m_szBrokerID);
    strcpy(orderActionField.InvestorID,m_szInvestorID);
    orderActionField.OrderActionRef = nOrderActionRef;
    snprintf(orderActionField.OrderRef,sizeof(orderActionField.OrderRef),"%d",_nOrderRef);
    orderActionField.FrontID = m_nFrontID;
    orderActionField.SessionID = m_nSessionID;
    orderActionField.ActionFlag = THOST_FTDC_AF_Delete;
    strcpy(orderActionField.InstrumentID,m_strSecuCode.toStdString().c_str());

    int ret = m_ptrUserTradeApi->ReqOrderAction(&orderActionField,nRequestID);
    m_nCancelOrderRef = nOrderActionRef;
} // CTFMarketMaker::submitCancelOrder

// 处理止损
// 当盘口价格与委托价格距离超过m_nStoplossTick个tick时，触发止损
void CTFMarketMaker::handleStoploss()
{
    if(LONGTRADING == m_eTradingType)
    {
//        double fBidPrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getDepthMktData(m_strSecuCode).BidPrice[0];
        double fBidPrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(m_strSecuCode).BidPrice;
        int nLossTickNum = (int)(((m_fHoldingCost - fBidPrice)*m_nPricePrecision + 0.5)/m_nPriceTick);
        if(nLossTickNum > m_nStoplossTicks)
        {// 如果触发止损，撤单，并把交易状态更新为“已发出平仓委托撤单请求”
            submitCancelOrder(m_nTradeOrderRef);
            m_eTradingStatus = SENDEDCANCELLIQUIDATIONORDER;
        }
    }
    else if(SHORTTRADING == m_eTradingType)
    {
//        double fAskPrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getDepthMktData(m_strSecuCode).AskPrice[0];
        double fAskPrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(m_strSecuCode).AskPrice;
        int nLossTickNum = (int)(((fAskPrice - m_fHoldingCost)*m_nPricePrecision + 0.5)/m_nPriceTick);
        if(nLossTickNum > m_nStoplossTicks)
        {// 如果触发止损，撤单，并把交易状态更新为“已发出平仓委托撤单请求”
            submitCancelOrder(m_nTradeOrderRef);
            m_eTradingStatus = SENDEDCANCELLIQUIDATIONORDER;
        }
    }
} // CTFMarketMaker::handleStoploss

void CTFMarketMaker::sendLimitOrder(TThostFtdcDirectionType _chDirection, TThostFtdcOffsetFlagType _chOpenClose, int _nVol, double _fPrice) const
{
    int nRequestID = CRequestID::getCRequestIDPtr()->getValidRequestID();
    int nOrderRef = COrderRef::getCOrderRefPtr()->getValidOrderRef();

    CThostFtdcInputOrderField orderInsert;
    memset(&orderInsert,0,sizeof(orderInsert));
    strcpy(orderInsert.BrokerID,m_szBrokerID);
    strcpy(orderInsert.InvestorID,m_szInvestorID);
    strcpy(orderInsert.UserID,m_szInvestorID);
    strcpy(orderInsert.InstrumentID,m_strSecuCode.toStdString().c_str());

    snprintf(orderInsert.OrderRef,sizeof(orderInsert.OrderRef),"%d",nOrderRef);

    orderInsert.Direction = _chDirection;
    orderInsert.CombOffsetFlag[0] = _chOpenClose;
    orderInsert.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
    orderInsert.VolumeTotalOriginal = _nVol;
    orderInsert.VolumeCondition = THOST_FTDC_VC_AV;
    orderInsert.MinVolume = 1;

    orderInsert.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
    orderInsert.TimeCondition = THOST_FTDC_TC_GFD;
    orderInsert.LimitPrice = _fPrice;

    orderInsert.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
    orderInsert.IsAutoSuspend = 0;
    orderInsert.UserForceClose = 0;
    orderInsert.ContingentCondition = THOST_FTDC_CC_Immediately;

    int ret = m_ptrUserTradeApi->ReqOrderInsert(&orderInsert,nRequestID);
    m_nTradeOrderRef = nOrderRef;
} // sendLimitOrder

void CTFMarketMaker::sendMktPriceOrder(TThostFtdcDirectionType _chDirection, TThostFtdcOffsetFlagType _chOpenClose, int _nvol) const
{
    int nRequestID = CRequestID::getCRequestIDPtr()->getValidRequestID();
    int nOrderRef = COrderRef::getCOrderRefPtr()->getValidOrderRef();

    CThostFtdcInputOrderField orderInsert;
    memset(&orderInsert,0,sizeof(orderInsert));
    strcpy(orderInsert.BrokerID,m_szBrokerID);
    strcpy(orderInsert.InvestorID,m_szInvestorID);
    strcpy(orderInsert.UserID,m_szInvestorID);
    strcpy(orderInsert.InstrumentID,m_strSecuCode.toStdString().c_str());

    snprintf(orderInsert.OrderRef,sizeof(orderInsert.OrderRef),"%d",nOrderRef);

    orderInsert.Direction = _chDirection;
    orderInsert.CombOffsetFlag[0] = _chOpenClose;
    orderInsert.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
    orderInsert.VolumeTotalOriginal = _nvol;
    orderInsert.VolumeCondition = THOST_FTDC_VC_AV;
    orderInsert.MinVolume = 1;

//    orderInsert.OrderPriceType = THOST_FTDC_OPT_AnyPrice;
//    orderInsert.TimeCondition = THOST_FTDC_TC_IOC;
//    orderInsert.LimitPrice = 0;

    double fOrderPrice = 0.0;
    if(LONGTRADING == m_eTradingType)
    {
        fOrderPrice = ((int)(CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(m_strSecuCode).BidPrice * m_nPricePrecision + 0.5) - 10 * m_nPriceTick) / (double)m_nPricePrecision;
    }
    else if(SHORTTRADING == m_eTradingType)
    {
        fOrderPrice = ((int)(CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(m_strSecuCode).AskPrice * m_nPricePrecision + 0.5) + 10 * m_nPriceTick) / (double)m_nPricePrecision;
    }
    orderInsert.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
    orderInsert.TimeCondition = THOST_FTDC_TC_GFD;
    orderInsert.LimitPrice = fOrderPrice;

    orderInsert.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
    orderInsert.IsAutoSuspend = 0;
    orderInsert.UserForceClose = 0;
    orderInsert.ContingentCondition = THOST_FTDC_CC_Immediately;

    int ret = m_ptrUserTradeApi->ReqOrderInsert(&orderInsert,nRequestID);
    m_nTradeOrderRef = nOrderRef;
} // sendMktPriceOrder

}
