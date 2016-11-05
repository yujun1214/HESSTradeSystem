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
    // ����trading status������ͬ����ʽ
    switch(m_eTradingStatus)
    {
    case EMPTYHOLDING:          // �ղ�
        // ��ǰ����ʱ��״̬Ϊ���������ס�ʱ���Ž���ί��
        if(CTimeController::NORMALTRADING == m_ptrTimeController->CurrTimeStatus())
            submitOpenPositionOrder();
        break;
    case SENDEDPOSITIONORDER:   // �ѷ�������ί��
        // �����ǰ����ʱ��״̬Ϊ�����̻����ڡ�������ƽ���ڡ�ʱ�����г���
        if(CTimeController::PRECLOSINGCUSHION == m_ptrTimeController->CurrTimeStatus() ||
           CTimeController::PRECLOSINGLIQUIDATION == m_ptrTimeController->CurrTimeStatus())
        {
            submitCancelOrder(m_nTradeOrderRef);
            m_eTradingStatus = SENDEDCANCELPOSITIONORDER;
        }
        // �����ǰ�ı��۳�����intervals�ķ�Χ�����г���
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
    case SENDEDCANCELPOSITIONORDER: // �ѷ�������ί�г�������
        break;
    case POSITIONED:            // �������
        submitClosePositionOrder();
        break;
    case SENDEDLIQUIDATIONORDER:    // �ѷ���ƽ��ί��
        if(CTimeController::NORMALTRADING == m_ptrTimeController->CurrTimeStatus() ||
           CTimeController::PRECLOSINGCUSHION == m_ptrTimeController->CurrTimeStatus())
        {// �����ǰ����ʱ��״̬�ǡ��������ס������̻����ڡ�����ô����ֹ�����
            handleStoploss();
        }
        else if(CTimeController::PRECLOSINGLIQUIDATION == m_ptrTimeController->CurrTimeStatus())
        {// �����ǰ����ʱ��״̬�ǡ�����ƽ���ڡ�����ô����
            submitCancelOrder(m_nTradeOrderRef);
            m_eTradingStatus = SENDEDCANCELLIQUIDATIONORDER;
        }
        break;
    case SENDEDCANCELLIQUIDATIONORDER:  // �ѷ���ƽ��ί�г�������
        break;
    default:
        break;
    }

    // �������鴦�����signal(for backtest)
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

    // ���ݱ����ύ״̬�ֱ���
    switch(_ptrOrder->OrderSubmitStatus)
    {
    case THOST_FTDC_OSS_InsertSubmitted:    // �Ѿ��ύ
        break;
    case THOST_FTDC_OSS_CancelSubmitted:    // �����Ѿ��ύ
        break;
    case THOST_FTDC_OSS_ModifySubmitted:    // �޸��Ѿ��ύ
        break;
    case THOST_FTDC_OSS_Accepted:           // �Ѿ�����
        break;
    case THOST_FTDC_OSS_InsertRejected:     // �����Ѿ����ܾ�
        if(SENDEDPOSITIONORDER == m_eTradingStatus)
        {// �����ǰ����״̬ = ���ѷ�������ί�С���������״̬��Ϊ���ղ֡�������������ί�б��ܾ���signal
            m_eTradingStatus = EMPTYHOLDING;
//            emit sigOpenOrderDenied(_ptrOrder->LimitPrice);
            emit sigOpenOrderDenied(m_nOrderIndex);
        }
        else if(SENDEDLIQUIDATIONORDER == m_eTradingStatus)
        {// �����ǰ����״̬ = ���ѷ���ƽ��ί�С���������״̬��Ϊ��������ɡ���������ƽ��ί�б��ܾ���signal
            m_eTradingStatus = POSITIONED;
//            emit sigCloseOrderDenied(_ptrOrder->LimitPrice);
            emit sigCloseOrderDenied(m_nOrderIndex);
        }
        break;
    case THOST_FTDC_OSS_CancelRejected:     // �����Ѿ����ܾ�
        // �������ܾ���ĺ���������OnRspOrderAction�д���
        break;
    case THOST_FTDC_OSS_ModifyRejected:     // �ĵ��Ѿ����ܾ�
        break;
    default:
        break;
    }

    // ���ݱ���״̬�ֱ���
    switch(_ptrOrder->OrderStatus)
    {
    case THOST_FTDC_OST_AllTraded:      // ȫ���ɽ�
        //  ����������RtnTrade�н��д���
        break;
    case THOST_FTDC_OST_PartTradedQueueing: // ���ֳɽ����ڶ�����
        // ��������
        break;
    case THOST_FTDC_OST_PartTradedNotQueueing:  // ���ֳɽ����ڶ�����
        // ��������
        break;
    case THOST_FTDC_OST_NoTradeQueueing:        // δ�ɽ����ڶ�����
        // ��������
        break;
    case THOST_FTDC_OST_NoTradeNotQueueing:     // δ�ɽ����ڶ�����
        // ��������
        break;
    case THOST_FTDC_OST_Canceled:       // ����
        if(SENDEDCANCELPOSITIONORDER == m_eTradingStatus)
        {// ��ǰ����״̬����ǡ��ѷ�������ί�г������󡱣���ô������״̬����Ϊ���ղ֡�������������ί�г�����ɡ�signal
            m_eTradingStatus = EMPTYHOLDING;
//            emit sigOpenOrderCanceled(_ptrOrder->LimitPrice);
            emit sigOpenOrderCanceled(m_nOrderIndex);
        }
        else if(SENDEDCANCELLIQUIDATIONORDER == m_eTradingStatus)
        {// ��ǰ����״̬����ǡ��ѷ���ƽ��ί�г������󡱣���ô������״̬����Ϊ���ѷ���ƽ��ί�С���������ֹ��ί�У�ͬʱ������ƽ��ί�г�����ɡ�signal
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
    case THOST_FTDC_OST_Unknown:    // δ֪
        // ��������
        break;
    case THOST_FTDC_OST_NotTouched: // ��δ����
        // ��������
        break;
    case THOST_FTDC_OST_Touched:    // �Ѵ���
        // ��������
        break;
    default:
        break;
    }

    // ɾ������ָ��
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
    {// ����ǿ��ֽ���
        if(SENDEDPOSITIONORDER == m_eTradingStatus)
        {// ��ǰ����״̬=���ѷ�������ί�С�������״̬����Ϊ��������ɡ�����¼holding cost,������������ɡ�signal
            m_eTradingStatus = POSITIONED;
            m_fHoldingCost = _ptrTrade->Price;
//            emit sigPositioned(_ptrTrade->Price);
            emit sigPositioned(m_nOrderIndex);
        }
        else if(SENDEDCANCELPOSITIONORDER == m_eTradingStatus)
        {// ��ǰ����״̬=���ѷ�������ί�г������󡱣�������ί�г���order�����г�ʱ�ý���order�Ѿ��ɽ������½���״̬Ϊ��������ɡ�����¼holding cost
            m_eTradingStatus = POSITIONED;
            m_fHoldingCost = _ptrTrade->Price;
            // ������������ɡ�signal
//            emit sigPositioned(_ptrTrade->Price);
            emit sigPositioned(m_nOrderIndex);
        }
    }
    else if(_ptrTrade->OffsetFlag == THOST_FTDC_OF_Close)
    {// �����ƽ�ֽ���
        if(SENDEDLIQUIDATIONORDER == m_eTradingStatus)
        {// ��ǰ����״̬=���ѷ���ƽ��ί�С�������״̬����Ϊ���ղ֡�
            m_eTradingStatus = EMPTYHOLDING;
            // ������ƽ����ɡ�signal
//            emit sigLiquidationed(_ptrTrade->Price);
            emit sigLiquidationed(m_nOrderIndex);
        }
        else if(SENDEDCANCELLIQUIDATIONORDER == m_eTradingStatus)
        {// ��ǰ����״̬=���ѷ���ƽ��ί�г������󡱣���ƽ��ί�г���order�����г�ʱ��ƽ��order�Ѿ��ɽ������½���״̬Ϊ���ղ֡�
            m_eTradingStatus = EMPTYHOLDING;
            // ������ƽ����ɡ�signal
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
    {// ����ί�б��ܾ�
        if(SENDEDCANCELPOSITIONORDER == m_eTradingStatus)
        {// ��ǰ����״̬=���ѷ�������ί�г������󡰣�����״̬����Ϊ���ѷ�������ί�С�
            m_eTradingStatus = SENDEDPOSITIONORDER;
        }
        else if(SENDEDCANCELLIQUIDATIONORDER == m_eTradingStatus)
        {// ��ǰ����״̬=���ѷ���ƽ��ί�г������󡱣�����״̬����Ϊ���ѷ���ƽ��ί�С�
            m_eTradingStatus = SENDEDLIQUIDATIONORDER;
        }
    }

    delete _ptrOrderAction;

} // CTFMarketMaker::handleRspOrderAction

// ����frontid��sessionid
void CTFMarketMaker::setFrontSessionID(TThostFtdcFrontIDType _nFrontID, TThostFtdcSessionIDType _nSessionID)
{
    m_nFrontID = _nFrontID;
    m_nSessionID = _nSessionID;
}

// ����ί��
void CTFMarketMaker::submitOpenPositionOrder()
{
    std::cout << "Submit Open Position Order..." << std::endl;
    if(LONGTRADING == m_eTradingType)
    {// long trading type
//        double fBuyPrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getDepthMktData(m_strSecuCode).BidPrice[0];
        double fBuyPrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(m_strSecuCode).BidPrice;
        m_ptrMMDispatcher->acquireBuyOrderPrice(m_nOrderIndex,fBuyPrice);
        sendLimitOrder(THOST_FTDC_D_Buy,THOST_FTDC_OF_Open,1,fBuyPrice);
        // ���ý���״̬Ϊ���ѷ�������ί�С�
        m_eTradingStatus = SENDEDPOSITIONORDER;
    }
    else if(SHORTTRADING == m_eTradingType)
    {// short trading type
//        double fSellPrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getDepthMktData(m_strSecuCode).AskPrice[0];
        double fSellPrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(m_strSecuCode).AskPrice;
        m_ptrMMDispatcher->acquireSellOrderPrice(m_nOrderIndex,fSellPrice);
        sendLimitOrder(THOST_FTDC_D_Sell,THOST_FTDC_OF_Open,1,fSellPrice);
        // ���ý���״̬Ϊ���ѷ�������ί�С�
        m_eTradingStatus = SENDEDPOSITIONORDER;
    }
}

// ƽ��ί��
void CTFMarketMaker::submitClosePositionOrder()
{
    if(LONGTRADING == m_eTradingType)
    {// long trading type
//        double fSellClosePrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getDepthMktData(m_strSecuCode).AskPrice[0];
        double fSellClosePrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(m_strSecuCode).AskPrice;
        m_ptrMMDispatcher->acquireSellCloseOrderPrice(m_nOrderIndex,fSellClosePrice);
        // ƽ�ּ۸����ٱ�֤ӯ��1��tick
        double fMinClosePrice = ((int)(m_fHoldingCost * m_nPricePrecision + 0.5) + m_nPriceTick) / (double)m_nPricePrecision;
        sendLimitOrder(THOST_FTDC_D_Sell,THOST_FTDC_OF_Close,1,fSellClosePrice < fMinClosePrice ? fMinClosePrice:fSellClosePrice);
        // ���ý���״̬Ϊ���ѷ���ƽ��ί�С�
        m_eTradingStatus = SENDEDLIQUIDATIONORDER;
    }
    else if(SHORTTRADING == m_eTradingType)
    {// short trading tpe
//        double fBuyClosePrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getDepthMktData(m_strSecuCode).BidPrice[0];
        double fBuyClosePrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(m_strSecuCode).BidPrice;
        m_ptrMMDispatcher->acquireBuyCloseOrderPrice(m_nOrderIndex,fBuyClosePrice);
        // ƽ�ּ۸����ٱ�֤ӯ��1��tick
        double fMaxClosePrice = ((int)(m_fHoldingCost * m_nPricePrecision + 0.5) - m_nPriceTick) / (double)m_nPricePrecision;
        sendLimitOrder(THOST_FTDC_D_Buy,THOST_FTDC_OF_Close,1,fBuyClosePrice > fMaxClosePrice ? fMaxClosePrice:fBuyClosePrice);
        // ���ý���״̬Ϊ���ѷ���ƽ��ί�С�
        m_eTradingStatus = SENDEDLIQUIDATIONORDER;
    }
}

// ����ί��
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

// ����ֹ��
// ���̿ڼ۸���ί�м۸���볬��m_nStoplossTick��tickʱ������ֹ��
void CTFMarketMaker::handleStoploss()
{
    if(LONGTRADING == m_eTradingType)
    {
//        double fBidPrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getDepthMktData(m_strSecuCode).BidPrice[0];
        double fBidPrice = CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(m_strSecuCode).BidPrice;
        int nLossTickNum = (int)(((m_fHoldingCost - fBidPrice)*m_nPricePrecision + 0.5)/m_nPriceTick);
        if(nLossTickNum > m_nStoplossTicks)
        {// �������ֹ�𣬳��������ѽ���״̬����Ϊ���ѷ���ƽ��ί�г�������
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
        {// �������ֹ�𣬳��������ѽ���״̬����Ϊ���ѷ���ƽ��ί�г�������
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
