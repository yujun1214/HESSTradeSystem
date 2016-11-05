#include "CCrossArbitrage.h"
#include "Utility/CConfig.h"
#include <stdio.h>
#include <QStringList>
#include <QMutexLocker>
#include <cstdio>

namespace HESS
{

// ��ʼ��requestid��orderref
int CCrossArbitrage::m_nRequestID = 0;
QString CCrossArbitrage::m_strOrderRef = "000000000000";

CCrossArbitrage::CCrossArbitrage(CThostFtdcTraderApi *pUserTraderApi, QString _LegOneTicker,
                                 QString _LegTwoTicker, int _nPairNum, int _nMaxHoldingNum):
                                 m_pUserTraderApi(pUserTraderApi),m_strLegOneTicker(_LegOneTicker),
                                 m_strLegTwoTicker(_LegTwoTicker)
{
    // _nPairNumΪ�������Կɿ��ֵ�����������
    m_HoldingDataList.clear();
    for(int i = 0;i < _nPairNum;++i)
    {
        m_HoldingDataList.push_back(new THoldingVolPair());
        m_HoldingDataList[i]->nMaxHoldingVol = _nMaxHoldingNum;
    }

    m_tLegOneMktData.ticker = _LegOneTicker;
    m_tLegTwoMktData.ticker = _LegTwoTicker;
} // CCrossArbitrage

CCrossArbitrage::~CCrossArbitrage()
{
    for(QList<THoldingVolPair*>::iterator it = m_HoldingDataList.begin();it != m_HoldingDataList.end();++it)
    {
        delete *it;
    }
} // ~CCrossArbitrage

void CCrossArbitrage::updateMktData(TDerivativeMktDataField _mktData)
{
//    QMutexLocker locker(&m_MktMutex);
    if(_mktData.ticker == m_strLegOneTicker)
    {
        m_tLegOneMktData.ticker = _mktData.ticker;
        m_tLegOneMktData.Last = _mktData.Last;
        // ��������һ��һ�ۺ���
        m_tLegOneMktData.AskPrice[0] = _mktData.AskPrice[0];
        m_tLegOneMktData.AskVol[0] = _mktData.AskVol[0];
        m_tLegOneMktData.BidPrice[0] = _mktData.BidPrice[0];
        m_tLegOneMktData.BidVol[0] = _mktData.BidVol[0];
        m_tLegOneMktData.tmUpdateTime = _mktData.tmUpdateTime;
//        locker.unlock();

        // ������������������
        doArbitrage();
    }
    else
    {
        if(_mktData.ticker == m_strLegTwoTicker)
        {
            m_tLegTwoMktData.ticker = _mktData.ticker;
            m_tLegTwoMktData.Last = _mktData.Last;
            // ��������һ��һ�ۺ���
            m_tLegTwoMktData.AskPrice[0] = _mktData.AskPrice[0];
            m_tLegTwoMktData.AskVol[0] = _mktData.AskVol[0];
            m_tLegTwoMktData.BidPrice[0] = _mktData.BidPrice[0];
            m_tLegTwoMktData.BidVol[0] = _mktData.BidVol[0];
            m_tLegTwoMktData.tmUpdateTime = _mktData.tmUpdateTime;
//            locker.unlock();

            // ������������������
            doArbitrage();
        }
    }

    // �����ǰΪreplyģʽ������sigUpdatedMktData�ź�
    if(m_Mode == REPLY)
        emit sigUpdatedMktData();
} // updateMktData

void CCrossArbitrage::updateCointegrationParam(TCointegrationParam _regressParam)
{
//    QMutexLocker locker(&m_RegressionParamMutex);
    if(_regressParam.strIndependentTicker != m_strLegOneTicker || _regressParam.strDependentTicker != m_strLegTwoTicker)
    {
        return;
    }

    m_tCointegrationParam = _regressParam;
    bIfUpdatedCointegrationParam = true;
}

// �����������׵�������
// �������£�
// Э���ع�����Ƿ��Ѹ��£���δ����return
// �����ֲ�����list��������ǡ��ղ֡�״̬�������Ƿ񴥷����֣������򿪲֡�һ�������������Ϳ�����һ�������ԣ���ô�ڱ�������������
// ���ٿ��ֵڶ��������ԡ�������ǡ�������ɡ�״̬�������Ƿ񴥷�ֹӯ��ֹ��������ƽ�֡������������״̬��Ҳ��Ҫ����������г����򲹵�����
void CCrossArbitrage::doArbitrage()
{
    // ���Э��������δ���£���������
    if(!bIfUpdatedCointegrationParam)
        return;

    // ������ڽ���ʱ�䣬������гֲֽ���ǿ��ƽ��


    // ע�������������������黥�������������ع������������������ܻ��������
//    QMutexLocker mktLocker(&m_MktMutex);
//    QMutexLocker regressLocker(&m_RegressionParamMutex);

    // �����ֲ�����list
    bool bIfHasPositionedOnePair = false;   // �������������Ƿ��ѿ�����һ��������
    for(QList<THoldingVolPair*>::const_iterator it = m_HoldingDataList.begin();it != m_HoldingDataList.end();++it)
    {
        THoldingVolPair* pHolding = *it;
        // ���ݳֲ����ݵ�״̬������ȡ��ͬ�Ĳ���
        switch(pHolding->eTradingStatus)
        {
        // ����״̬���� = �ղ�״̬�������Ƿ������������
        case EMPTYHOLDING:
            // ����������������Ѿ�����һ�β֣���ô�˳�
            if(bIfHasPositionedOnePair)
                break;
            // ������ϵ������Ч����֮�⣬��ô�˳�
            if(m_tCointegrationParam.fRho > m_fRhoUpperLimit || m_tCointegrationParam.fRho < m_fRhoLowerLimit)
                break;
            // ���㿪��spread
            // ��LegOne����LegTwo��spread
            double fSellSpread = m_tLegTwoMktData.BidPrice[0]*m_nLegTwoMultiple - (m_tCointegrationParam.fIntercept + m_tCointegrationParam.fSlope*m_tLegOneMktData.AskPrice[0]*m_nLegOneMultiple);
            // ��LegOne����LegTwo��spread
            double fBuySpread = m_tLegTwoMktData.AskPrice[0]*m_nLegTwoMultiple - (m_tCointegrationParam.fIntercept + m_tCointegrationParam.fSlope*m_tLegOneMktData.BidPrice[0]*m_nLegOneMultiple);
            // ���fSellSpread > ���ֲв�ֵ���ޣ���ô����short spread���֣���������legtwo�����뿪��legone��
            if(fSellSpread > m_tCointegrationParam.fOpenUpperLimit)
            {
                // ����legone��legtwo�Ľ�������
                int nLegOneTradeVol = pHolding->nMaxHoldingVol;
                int nLegTwoTradeVol = pHolding->nMaxHoldingVol;
                if(m_tCointegrationParam.fSlope > 1.05)
                {
                    nLegOneTradeVol = pHolding->nMaxHoldingVol;
                    nLegTwoTradeVol = (int)(pHolding->nMaxHoldingVol/m_tCointegrationParam.fSlope+0.5);
                }
                else
                    if(m_tCointegrationParam.fSlope < 0.95)
                    {
                        nLegOneTradeVol = (int)(pHolding->nMaxHoldingVol * m_tCointegrationParam.fSlope+0.5);
                        nLegTwoTradeVol = pHolding->nMaxHoldingVol;
                    }
                // �ж��̿ڹҵ����Ƿ��㹻
                bool bIfLegOneVolEnough = (m_tLegOneMktData.AskVol[0] >= 2*nLegOneTradeVol);
                bool bIfLegTwoVolEnough = (m_tLegTwoMktData.BidVol[0] >= 2*nLegTwoTradeVol);
                if(bIfLegOneVolEnough && bIfLegTwoVolEnough)
                {// �̿ڹҵ����㹻��short spread����
                    // ����short spread����ί��
                    SubmitOpenPositionOrder(SHORTSPREAD,pHolding,nLegOneTradeVol,nLegTwoTradeVol);
                    bIfHasPositionedOnePair = true;
                    // ���³ֲֲ���
                    pHolding->eArbType = SHORTSPREAD;   // ��������Ϊ������spread��
                    pHolding->nLegOnePlannedVol = nLegOneTradeVol;      // LegOne�ļƻ��ֲ���
                    pHolding->nLegTwoPlannedVol = nLegTwoTradeVol;      // LegTwo�ļƻ��ֲ���
                    pHolding->tRegressParam = m_tCointegrationParam;    // ��¼����ʱ�Ļع����
                    pHolding->eTradingStatus = SENDEDPOSITIONORDER;     // ����״̬���� = �ѷ�������ί��
                    pHolding->tmStatusSettingTime = QTime::currentTime();   // ����״̬�����趨ʱ��
                }
            }
            // ���fBuySpread > ���ֲв�ֵ���ޣ���ô����long spread���֣����뿪��legtwo����������legone��
            else if(fBuySpread < m_tCointegrationParam.fOpenLowerLimit)
            {
                // ����legone��legtwo�Ľ�������
                int nLegOneTradeVol = pHolding->nMaxHoldingVol;
                int nLegTwoTradeVol = pHolding->nMaxHoldingVol;
                if(m_tCointegrationParam.fSlope > 1.05)
                {
                    nLegOneTradeVol = pHolding->nMaxHoldingVol;
                    nLegTwoTradeVol = (int)(pHolding->nMaxHoldingVol/m_tCointegrationParam.fSlope+0.5);
                }
                else
                    if(m_tCointegrationParam.fSlope < 0.95)
                    {
                        nLegOneTradeVol = (int)(pHolding->nMaxHoldingVol * m_tCointegrationParam.fSlope+0.5);
                        nLegTwoTradeVol = pHolding->nMaxHoldingVol;
                    }
                // �ж��̿ڹҵ����Ƿ��㹻
                bool bIfLegOneVolEnough = (m_tLegOneMktData.BidVol[0] >= 2*nLegOneTradeVol);
                bool bIfLegTwoVolEnough = (m_tLegTwoMktData.AskVol[0] >= 2*nLegTwoTradeVol);
                if(bIfLegOneVolEnough && bIfLegTwoVolEnough)
                {// �̿ڹҵ����㹻��buy spread����
                    // ����buy spread����ί��
                    SubmitOpenPositionOrder(LONGSPREAD,pHolding,nLegOneTradeVol,nLegTwoTradeVol);
                    bIfHasPositionedOnePair =true;
                    //���³ֲֲ���
                    pHolding->eArbType = LONGSPREAD;    // ��������Ϊ������spread��
                    pHolding->nLegOnePlannedVol = nLegOneTradeVol;      // LegOne�ļƻ��ֲ���
                    pHolding->nLegTwoPlannedVol = nLegTwoTradeVol;      // LegTwo�ļƻ��г���
                    pHolding->tRegressParam = m_tCointegrationParam;    // ��¼����ʱ�Ļع����
                    pHolding->eTradingStatus = SENDEDPOSITIONORDER;     // ����״̬���� = �ѷ�������ί��
                    pHolding->tmStatusSettingTime = QTime::currentTime();   // ����״̬�����趨ʱ��
                }
            }
            break;
        // ����״̬���� = �ѷ�������ί��
        case SENDEDPOSITIONORDER:
            // �����ǰʱ����뷢������ί��ʱ�䳬��3���ӣ�3000���룩���򽫽���״̬��Ϊ�ղ�
            if(pHolding->tmStatusSettingTime.msecsTo(QTime::currentTime()) > ELAPSEMSECS)
            {
                pHolding->eTradingStatus = EMPTYHOLDING;   // ����״̬���� = �ղ�
                pHolding->tmStatusSettingTime = QTime::currentTime();   // ����״̬�����趨ʱ��
            }
            break;
        // ����״̬���� = LegOne���Ƚ���,LegTwoδ����
        case LEGONESINGLEPOSITIONED:
            // �����ǰʱ��������½���״̬�趨ʱ�䳬��3���ӣ�3000���룩,�����¼������¼۲������Ǵ��ڴ���������������ô��������LegTwo�������ѿ��ֵ�LegOneƽ��
            if(pHolding->tmStatusSettingTime.msecsTo(QTime::currentTime()) > ELAPSEMSECS)
            {
                double fLatestSpread = 0.0;
                double fLegOneUnitCost = pHolding->fLegOneHoldingCost / pHolding->nLegOneHoldingVol;
                if(SHORTSPREAD == pHolding->eArbType)
                {// ���ռ۲�����
                    fLatestSpread = m_tLegTwoMktData.BidPrice[0]*m_nLegTwoMultiple - (pHolding->tRegressParam.fIntercept + pHolding->tRegressParam.fSlope*fLegOneUnitCost*m_nLegOneMultiple);
                    if(fLatestSpread > pHolding->tRegressParam.fOpenUpperLimit)
                    {// ������µ����ռ۲����ɴ������֣���ô��������LegTwo
                        sendOrder(m_strLegTwoTicker,THOST_FTDC_D_Sell,THOST_FTDC_OF_Open,pHolding->nLegTwoPlannedVol,m_tLegTwoMktData.BidPrice[0],pHolding);
                        pHolding->tmStatusSettingTime = QTime::currentTime();
                    }
                    else
                    {// ������µ����ռ۲�δ�������֣���ô��LegOne����ƽ��(����ƽ��)
                        sendOrder(m_strLegOneTicker,THOST_FTDC_D_Sell,THOST_FTDC_OF_Close,pHolding->nLegOneHoldingVol,m_tLegOneMktData.BidPrice[0],pHolding);
                        pHolding->eTradingStatus = SENDEDLEGONELIQUIDATIONORDER;    // ����״̬���� = �ѷ���LegOne����ƽ��ί��
                        pHolding->tmStatusSettingTime = QTime::currentTime();
                    }
                }
                else if(LONGSPREAD == pHolding->eArbType)
                {// ����۲�����
                    fLatestSpread = m_tLegTwoMktData.AskPrice[0]*m_nLegTwoMultiple - (pHolding->tRegressParam.fIntercept + pHolding->tRegressParam.fSlope*fLegOneUnitCost*m_nLegOneMultiple);
                    if(fLatestSpread < pHolding->tRegressParam.fOpenLowerLimit)
                    {// ������µ�����۲����ɴ������֣���ô���뿪��LegTwo
                        sendOrder(m_strLegTwoTicker,THOST_FTDC_D_Buy,THOST_FTDC_OF_Open,pHolding->nLegTwoPlannedVol,m_tLegTwoMktData.AskPrice[0],pHolding);
                        pHolding->tmStatusSettingTime = QTime::currentTime();
                    }
                    else
                    {// ������µ�����۲�δ�������֣���ô��LegOne����ƽ��(����ƽ��)
                        sendOrder(m_strLegOneTicker,THOST_FTDC_D_Buy,THOST_FTDC_OF_Close,pHolding->nLegOneHoldingVol,m_tLegOneMktData.AskPrice[0],pHolding);
                        pHolding->eTradingStatus = SENDEDLEGONELIQUIDATIONORDER;    // ����״̬���� = �ѷ���LegOne����ƽ��ί��
                        pHolding->tmStatusSettingTime = QTime::currentTime();
                    }
                }
            }
            break;
        // ����״̬���� = LegTwo���Ƚ���,LegOneδ����
        case LEGTWOSINGLEPOSITIONED:
            // �����ǰʱ��������½���״̬�趨ʱ�����3���ӣ�3000���룩�������¼������¼۲������Ǵ��ڴ���������������ô��������LegOne�������ѿ��ֵ�LegTwoƽ��
            if(pHolding->tmStatusSettingTime.msecsTo(QTime::currentTime()) > ELAPSEMSECS)
            {
                double fLatestSpread = 0.0;
                double fLegTwoUnitCost = pHolding->fLegTwoHoldingCost / pHolding->nLegTwoHoldingVol;
                if(SHORTSPREAD == pHolding->eArbType)
                {// ���ռ۲�����
                    fLatestSpread = fLegTwoUnitCost*m_nLegTwoMultiple - (pHolding->tRegressParam.fIntercept + pHolding->tRegressParam.fSlope*m_tLegOneMktData.AskPrice[0]*m_nLegOneMultiple);
                    if(fLatestSpread > pHolding->tRegressParam.fOpenUpperLimit)
                    {// ������µ������۲����ɴ������֣���ô���뿪��LegOne
                        sendOrder(m_strLegOneTicker,THOST_FTDC_D_Buy,THOST_FTDC_OF_Open,pHolding->nLegOnePlannedVol,m_tLegOneMktData.AskPrice[0],pHolding);
                        pHolding->tmStatusSettingTime = QTime::currentTime();
                    }
                    else
                    {// ������µ������۲�δ�������֣���ô��LegTwo����ƽ��(����ƽ��)
                        sendOrder(m_strLegTwoTicker,THOST_FTDC_D_Buy,THOST_FTDC_OF_Close,pHolding->nLegTwoHoldingVol,m_tLegTwoMktData.AskPrice[0],pHolding);
                        pHolding->eTradingStatus = SENDEDLEGTWOLIQUIDATIONORDER;    // ����״̬���� = �ѷ���LegTwo����ƽ��
                        pHolding->tmStatusSettingTime = QTime.currentTime();
                    }
                }
                else if(LONGSPREAD == pHolding->eArbType)
                {// ����۲�����
                    fLatestSpread = fLegTwoUnitCost*m_nLegTwoMultiple - (pHolding->tRegressParam.fIntercept + pHolding->tRegressParam.fSlope*m_tLegOneMktData.BidPrice[0]*m_nLegOneMultiple);
                    if(fLatestSpread < pHolding->tRegressParam.fOpenLowerLimit)
                    {// ������µ�����۲����ɴ������֣���ô��������LegOne
                        sendOrder(m_strLegOneTicker,THOST_FTDC_D_Sell,THOST_FTDC_OF_Open,pHolding->nLegOnePlannedVol,m_tLegOneMktData.BidPrice[0],pHolding);
                        pHolding->tmStatusSettingTime = QTime::currentTime();
                    }
                    else
                    {// ������µ�����۲�δ�������֣���ô��LegTwo����ƽ��(����ƽ��)
                        sendOrder(m_strLegTwoTicker,THOST_FTDC_D_Sell,THOST_FTDC_OF_Close,pHolding->nLegTwoHoldingVol,m_tLegTwoMktData.BidPrice[0],pHolding);
                        pHolding->eTradingStatus = SENDEDLEGTWOLIQUIDATIONORDER;    // ����״̬���� = �ѷ���LegTwo����ƽ��
                        pHolding->tmStatusSettingTime = QTime::currentTime();
                    }
                }
            }
            break;
        // ����״̬���� = �������
        case POSITIONED:
            // �����Ƿ񴥷�ֹӯ��ֹ��
            if(SHORTSPREAD == pHolding->eArbType)
            {// ��ǰ�ֲ�Ϊ�����۲�ֲ�
                // ���ݽ���ʱ�Ļع�������������۲�ֲֵ�����ƽ�ּ۲�
                double fCloseSpread = m_tLegTwoMktData.AskPrice[0]*m_nLegTwoMultiple - (pHolding->tRegressParam.fIntercept+pHolding->tRegressParam.fSlope*m_tLegOneMktData.BidPrice[0]*m_nLegOneMultiple);
                // ��������۲�ֲֵ�����ƽ�ּ۲��(���ּ۲�+ֹӯֹ��ʱ��Ӧ�Ĳв�䶯ֵ)����ôƽ��ֹ��
                if(fCloseSpread > pHolding->fOpenError + pHolding->tRegressParam.fErrorChgForExitTrade)
                {
                    SubmitClosePositionOrder(SHORTSPREAD,pHolding,pHolding->nLegOneHoldingVol,pHolding->nLegTwoHoldingVol);
                    // ���³ֲֲ���
                    pHolding->eTradingStatus = SENDEDLIQUIDATIONORDER;  // ����״̬���� = �ѷ���ƽ��ί��
                    pHolding->tmStatusSettingTime = QTime::currentTime();   // ����״̬�����趨ʱ��
                }
                // ��������۲�ֲֵ�����ƽ�ּ۲�С��(���ּ۲�-ֹӯֹ���Ӧ�Ĳв�䶯ֵ)����ôƽ��ֹӯ
                else if(fCloseSpread < pHolding->fOpenError - pHolding->tRegressParam.fErrorChgForExitTrade)
                {
                    SubmitClosePositionOrder(SHORTSPREAD,pHolding,pHolding->nLegOneHoldingVol,pHolding->nLegTwoHoldingVol);
                    // ���³ֲֲ���
                    pHolding->eTradingStatus = SENDEDLIQUIDATIONORDER;  // �ѷ���ƽ��ί��
                    pHolding->tmStatusSettingTime = QTime::currentTime();   // ����״̬�����趨ʱ��
                }
            }
            else if(LONGSPREAD == pHolding->eArbType)
            {// ��ǰ�ֲ�Ϊ����۲�ֲ�
                // ���ݽ���ʱ�Ļع������������۲�ֲֵ�����ƽ�ּ۲�
                double fCloseSpread = m_tLegTwoMktData.BidPrice[0]*m_nLegTwoMultiple - (pHolding->tRegressParam.fIntercept+pHolding->tRegressParam.fSlope*m_tLegOneMktData.AskPrice[0]*m_nLegOneMultiple);
                // �������۲�ֲֵ�����ƽ�ּ۲�С��(���ּ۲�-ֹӯֹ���Ӧ�Ĳв�䶯ֵ)����ôƽ��ֹ��
                if(fCloseSpread < pHolding->fOpenError - pHolding->tRegressParam.fErrorChgForExitTrade)
                {
                    SubmitClosePositionOrder(LONGSPREAD,pHolding,pHolding->nLegOneHoldingVol,pHolding->nLegTwoHoldingVol);
                    // ����״̬����
                    pHolding->eTradingStatus = SENDEDLIQUIDATIONORDER;  // �ѷ���ƽ��ί��
                    pHolding->tmStatusSettingTime = QTime::currentTime();   // ����״̬�����趨ʱ��
                }
                // �������۲�ֲֵ�����ƽ�ּ۲����(���ּ۲�+ֹӯֹ���Ӧ�Ĳв�䶯ֵ)����ôƽ��ֹӯ
                else if(fCloseSpread > pHolding->fOpenError + pHolding->tRegressParam.fErrorChgForExitTrade)
                {
                    SubmitClosePositionOrder(LONGSPREAD,pHolding,pHolding->nLegOneHoldingVol,pHolding->nLegTwoHoldingVol);
                    // ����״̬����
                    pHolding->eTradingStatus = SENDEDLIQUIDATIONORDER;      // �ѷ���ƽ��ί��
                    pHolding->tmStatusSettingTime = QTime::currentTime();
                }
            }
            break;
        // ����״̬���� = �ѷ���LegOne����ƽ��ί��
        case SENDEDLEGONELIQUIDATIONORDER:
            // �����ǰʱ��������½���״̬�趨ʱ�䳬��3���ӣ�3000���룩�����ٴη���LegOne��ƽ��ί��
            if(pHolding->tmStatusSettingTime.msecsTo(QTime::currentTime()) > ELAPSEMSECS)
            {
                if(SHORTSPREAD == pHolding->eArbType)
                {// ��������ռ۲��LegOne����ƽ��
                    sendOrder(m_strLegOneTicker,THOST_FTDC_D_Sell,THOST_FTDC_OF_Close,pHolding->nLegOneHoldingVol,m_tLegOneMktData.BidPrice[0],pHolding);
                    pHolding->tmStatusSettingTime = QTime::currentTime();
                }
                else if(LONGSPREAD == pHolding->eArbType)
                {// ���������۲��LegOne����ƽ��
                    sendOrder(m_strLegOneTicker,THOST_FTDC_D_Buy,THOST_FTDC_OF_Close,pHolding->nLegOneHoldingVol,m_tLegOneMktData.AskPrice[0],pHolding);
                    pHolding->tmStatusSettingTime = QTime::currentTime();
                }
            }
            break;
        // ����״̬ = �ѷ���LegTwo����ƽ��ί��
        case SENDEDLEGTWOLIQUIDATIONORDER:
            // �����ǰʱ��������½���״̬�趨ʱ�䳬��3���ӣ�3000���룩�����ٴη���LegTwo��ƽ��ί��
            if(pHolding->tmStatusSettingTime.msecsTo(QTime::currentTime()) > ELAPSEMSECS)
            {
                if(SHORTSPREAD == pHolding->eArbType)
                {// ��������ռ۲��LegTwo����ƽ��
                    sendOrder(m_strLegTwoTicker,THOST_FTDC_D_Buy,THOST_FTDC_OF_Close,pHolding->nLegTwoHoldingVol,m_tLegTwoMktData.AskPrice[0],pHolding);
                }
                else if(LONGSPREAD == pHolding->eArbType)
                {// ���������۲��LegTwo����ƽ��
                    sendOrder(m_strLegTwoTicker,THOST_FTDC_D_Sell,THOST_FTDC_OF_Close,pHolding->nLegTwoHoldingVol,m_tLegTwoMktData.BidPrice[0],pHolding);
                }
                pHolding->tmStatusSettingTime = QTime::currentTime();
            }
            break;
        // ����״̬���� = �ѷ���ƽ��ί��
        case SENDEDLIQUIDATIONORDER:
            // �����ǰʱ��������½���״̬�趨ʱ�䳬��3���ӣ�3000���룩����ô���·���ƽ��ί��
            if(pHolding->tmStatusSettingTime.msecsTo(QTime::currentTime()) > ELAPSEMSECS)
            {
                SubmitClosePositionOrder(pHolding->eArbType,pHolding,pHolding->nLegOneHoldingVol,pHolding->nLegTwoHoldingVol);
                pHolding->tmStatusSettingTime = QTime::currentTime();
            }
            break;
        // ����״̬���� = LegOne����ƽ��,LegTwoδƽ��
        case LEGONELIQUIDATIONED:
            // �����ǰʱ��������½���״̬�趨ʱ�䳬��3���ӣ�3000���룩����ô���·���LegTwo��ƽ��ί��
            if(pHolding->tmStatusSettingTime.msecsTo(QTime::currentTime()) > ELAPSEMSECS)
            {
                if(SHORTSPREAD == pHolding->eArbType)
                {// ��������ռ۲��LegTwo����ƽ��
                    sendOrder(m_strLegTwoTicker,THOST_FTDC_D_Buy,THOST_FTDC_OF_Close,pHolding->nLegTwoHoldingVol,m_tLegTwoMktData.AskPrice[0],pHolding);
                }
                else if(LONGSPREAD == pHolding->eArbType)
                {// ���������۲��LegTow����ƽ��
                    sendOrder(m_strLegTwoTicker,THOST_FTDC_D_Sell,THOST_FTDC_OF_Close,pHolding->nLegTwoHoldingVol,m_tLegTwoMktData.BidPrice[0],pHolding);
                }
                pHolding->tmStatusSettingTime = QTime::currentTime();
            }
            break;
        // ����״̬���� = LegTwo����ƽ��,LegOneδƽ��
        case LEGTWOLIQUIDATIONED:
            // �����ǰʱ��������½���״̬�趨ʱ�䳬��3���ӣ�3000���룩���Ķ����·���LegOne��ƽ��ί��
            if(pHolding->tmStatusSettingTime.msecsTo(QTime::currentTime()) > ELAPSEMSECS)
            {
                if(SHORTSPREAD == pHolding->eArbType)
                {// ��������ռ۲��LegOne����ƽ��
                    sendOrder(m_strLegOneTicker,THOST_FTDC_D_Sell,THOST_FTDC_OF_Close,pHolding->nLegOneHoldingVol,m_tLegOneMktData.BidPrice[0],pHolding);
                }
                else if(LONGSPREAD == pHolding->eArbType)
                {// ���������۲��LegTOne����ƽ��
                    sendOrder(m_strLegOneTicker,THOST_FTDC_D_Buy,THOST_FTDC_OF_Close,pHolding->nLegOneHoldingVol,m_tLegOneMktData.AskPrice[0],pHolding);
                }
                pHolding->tmStatusSettingTime = QTime::currentTime();
            }
            break;
        default:
            break;
        }// switch
    }// for
}// doArbitrage

// ����ί��
// _arbtype = �������ͣ�����spread������spread��
// _nLegOneVol = legone��ί������
// _nLegTwoVol = legtwo��ί������
void CCrossArbitrage::SubmitOpenPositionOrder(ARBITRAGETYPE _arbtype,THoldingVolPair* _pHolding ,int _nLegOneVol, int _nLegTwoVol) const
{
    // ��LegOne��LegTwo�ĳֲ���������
    _pHolding->nLegOneHoldingVol = 0;
    _pHolding->nLegOnePlannedVol = 0;
    _pHolding->fLegOneHoldingCost = 0.0;
    _pHolding->nLegTwoHoldingVol = 0;
    _pHolding->nLegTwoPlannedVol = 0;
    _pHolding->fLegTwoHoldingCost = 0.0;

    if(SHORTSPREAD == _arbtype)
    {// �����۲LegTwo�������֡�LegOne���뿪��
        sendOrder(m_strLegOneTicker,THOST_FTDC_D_Buy,THOST_FTDC_OF_Open,_nLegOneVol,m_tLegOneMktData.AskPrice[0],_pHolding);
        sendOrder(m_strLegTwoTicker,THOST_FTDC_D_Sell,THOST_FTDC_OF_Open,_nLegTwoVol,m_tLegTwoMktData.BidPrice[0],_pHolding);
    }
    else if(LONGSPREAD == _arbtype)
    {// ����۲LegTwo���뿪�֡�LegOne��������
        sendOrder(m_strLegOneTicker,THOST_FTDC_D_Sell,THOST_FTDC_OF_Open,_nLegOneVol,m_tLegOneMktData.BidPrice[0],_pHolding);
        sendOrder(m_strLegTwoTicker,THOST_FTDC_D_Buy,THOST_FTDC_OF_Open,_nLegTwoVol,m_tLegTwoMktData.AskPrice[0],_pHolding);
    }
}

// ƽ��ί�У�����ֹӯ��ֹ������ǰƽ�֣�
// _arbtype = �������ͣ�����spread������spread��
// _nLegOneVol = legone��ί������
// _nLegTwoVol = legtwo��ί������
void CCrossArbitrage::SubmitClosePositionOrder(ARBITRAGETYPE _arbtype, THoldingVolPair *_pHolding, int _nLegOneVol, int _nLegTwoVol) const
{
    if(SHORTSPREAD == _arbtype)
    {// �����۲LegTwo����ƽ�֡�LegOne����ƽ��
        sendOrder(m_strLegOneTicker,THOST_FTDC_D_Sell,THOST_FTDC_OF_Close,_nLegOneVol,m_tLegOneMktData.BidPrice[0],_pHolding);
        sendOrder(m_strLegTwoTicker,THOST_FTDC_D_Buy,THOST_FTDC_OF_Close,_nLegTwoVol,m_tLegTwoMktData.AskPrice[0],_pHolding);
    }
    else if(LONGSPREAD == _arbtype)
    {// ����۲LegTwo����ƽ�֡�LegOne����ƽ��
        sendOrder(m_strLegOneTicker,THOST_FTDC_D_Buy,THOST_FTDC_OF_Close,_nLegOneVol,m_tLegOneMktData.AskPrice[0],_pHolding);
        sendOrder(m_strLegTwoTicker,THOST_FTDC_D_Sell,THOST_FTDC_OF_Close,_nLegTwoVol,m_tLegTwoMktData.BidPrice[0],_pHolding);
    }
}

void CCrossArbitrage::sendOrder(QString _strCode, TThostFtdcDirectionType _chDirection,
                                TThostFtdcOffsetFlagType _chOpenClose, int _nVol, double _fPrice, THoldingVolPair *_pHolding)
{
    int nRequestID = 0;
    int nOrderRef = 0;
    getValidOrderID(nRequestID,nOrderRef);
    m_mpOrderRefMapping.insert(nOrderRef,_pHolding);

    if(m_Mode == PRODUCTION)
    {// �����ǰ������ģʽ��������CTP����ί��
        CThostFtdcInputOrderField orderInsert;
        memset(&orderInsert,0,sizeof(orderInsert));
        strcpy(orderInsert.BrokerID,m_szBrokerID);
        strcpy(orderInsert.InvestorID,m_szInvestorID);
        strcpy(orderInsert.UserID,m_szInvestorID);
        strcpy(orderInsert.InstrumentID,_strCode.toStdString().c_str());

        snprintf(orderInsert.OrderRef,sizeof(orderInsert.OrderRef),"%d",nOrderRef);

        orderInsert.Direction = _chDirection;
        orderInsert.CombOffsetFlag[0] = _chOpenClose;
        orderInsert.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
        orderInsert.VolumeTotalOriginal = _nVol;

        // ����FOK����
        orderInsert.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
        orderInsert.TimeCondition = THOST_FTDC_TC_IOC;
        orderInsert.VolumeCondition = THOST_FTDC_VC_CV;
        orderInsert.MinVolume = _nVol;
        orderInsert.LimitPrice = _fPrice;

        orderInsert.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
        orderInsert.IsAutoSuspend = 0;
        orderInsert.UserForceClose = 0;
        orderInsert.ContingentCondition = THOST_FTDC_CC_Immediately;
        int ret = m_pUserTraderApi->ReqOrderInsert(&orderInsert,nRequestID);
    }
    else if(m_Mode == SIMNOW || m_Mode == REPLY)
    {// �����ǰ��simnow��replyģʽ������sendOrderOnSimlation
        sendOrderOnSimulation(nOrderRef,_strCode,_chDirection,_chOpenClose,_nVol,_fPrice);
    }
}

void CCrossArbitrage::handleRtnOrder(int _nOrderRef, CThostFtdcOrderField* _pOrder)
{
    // ȡ�ñ������ö�Ӧ�ĳֲ�����ָ��
//    QMutexLocker mappingLocker(&m_MappingMutex);
//    int nOrderRef = atoi(_pOrder->OrderRef);
    if(!m_mpOrderRefMapping.contains(_nOrderRef))
        return;
    THoldingVolPair* pHolding = m_mpOrderRefMapping[_nOrderRef];

    // ���ݱ����ύ״̬�ֱ���
    switch(_pOrder->OrderSubmitStatus)
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
        // �ӱ�������������ɾ����Ӧ�ļ�¼
        m_mpOrderRefMapping.remove(nOrderRef);
        break;
    case THOST_FTDC_OSS_CancelRejected:     // �����Ѿ����ܾ�
        break;
    case THOST_FTDC_OSS_ModifyRejected:     // �ĵ��Ѿ����ܾ�
        break;
    default:
        break;
    }

    // ���ݱ���״̬�ֱ���
    QString strInstrumentID = QString(QLatin1String(_pOrder->InstrumentID));
    switch(_pOrder->OrderStatus)
    {
    case THOST_FTDC_OST_AllTraded:      // ȫ���ɽ�
        // ���������Գֲ���������״̬����
        if(strInstrumentID == m_strLegOneTicker)
        {
            if(THOST_FTDC_OF_Open == _pOrder->CombOffsetFlag[0])
            {// ����ǿ��ֽ��ף���ôLegOne�ĳֲ��� = �ɽ���
                pHolding->nLegOneHoldingVol = _pOrder->VolumeTraded;
            }
            else if(THOST_FTDC_OF_Close == _pOrder->CombOffsetFlag[0] |
                    THOST_FTDC_OF_CloseToday == _pOrder->CombOffsetFlag[0] |
                    THOST_FTDC_OF_CloseYesterday == _pOrder->CombOffsetFlag[0])
            {// �����ƽ�ֽ��ף���ôLegOne�ĳֲ��� = 0��ͬʱ��LegOne�ĳֲֳɱ�����
                pHolding->nLegOneHoldingVol = 0;
                pHolding->fLegOneHoldingCost = 0.0;
            }

            // ���ճֲֵ�ʱ��ͬ�Ľ���״̬�����½���״̬
            switch(pHolding->eTradingStatus)
            {
            case EMPTYHOLDING:  // �ղ�
                if(THOST_FTDC_OF_Open == _pOrder->CombOffsetFlag[0])
                    pHolding->eTradingStatus = LEGONESINGLEPOSITIONED;  // LegOne���Ƚ��֣�LegTwoδ����
                break;
            case SENDEDPOSITIONORDER:   // �ѷ�������ί��
                if(THOST_FTDC_OF_Open == _pOrder->CombOffsetFlag[0])
                    pHolding->eTradingStatus = LEGONESINGLEPOSITIONED;  // LegOne���Ƚ��֣�LegTwoδ����
                break;
            case LEGONESINGLEPOSITIONED:    // LegOne���Ƚ��֣�LegTwoδ����
                // ��������
                break;
            case LEGTWOSINGLEPOSITIONED:    // LegTwo���Ƚ��֣�LegOneδ����
                if(THOST_FTDC_OF_Open == _pOrder->CombOffsetFlag[0])
                    pHolding->eTradingStatus = POSITIONED;  // �������
                break;
            case POSITIONED:    // �������
                // ��������
                break;
            case SENDEDLEGONELIQUIDATIONORDER:  // �ѷ���LegOne����ƽ��ί��
                if(THOST_FTDC_OF_Close == _pOrder->CombOffsetFlag[0] |
                   THOST_FTDC_OF_CloseToday == _pOrder->CombOffsetFlag[0] |
                   THOST_FTDC_OF_CloseYesterday == _pOrder->CombOffsetFlag[0])
                {
                    pHolding->eTradingStatus = EMPTYHOLDING;    // �ղ�
                    pHolding->fLegOneHoldingCost = 0.0;
                }
                break;
            case SENDEDLEGTWOLIQUIDATIONORDER:  // �ѷ���LegTwo����ƽ��ί��
                // ��������
                break;
            case SENDEDLIQUIDATIONORDER:    // �ѷ���ƽ��ί��
                if(THOST_FTDC_OF_Close == _pOrder->CombOffsetFlag[0] |
                   THOST_FTDC_OF_CloseToday == _pOrder->CombOffsetFlag[0] |
                   THOST_FTDC_OF_CloseYesterday == _pOrder->CombOffsetFlag[0])
                {
                    pHolding->eTradingStatus = LEGONELIQUIDATIONED;     // LegOne����ƽ�֣�LegTwoδƽ��
                    pHolding->fLegOneHoldingCost - 0.0;
                }
                break;
            case LEGONELIQUIDATIONED:       // LegOne����ƽ�֣�LegTwoδƽ��
                // ��������
                break;
            case LEGTWOLIQUIDATIONED:       // LegTwo����ƽ�֣�LegOneδƽ��
                if(THOST_FTDC_OF_Close == _pOrder->CombOffsetFlag[0] |
                   THOST_FTDC_OF_CloseToday == _pOrder->CombOffsetFlag[0] |
                   THOST_FTDC_OF_CloseYesterday == _pOrder->CombOffsetFlag[0])
                {
                    pHolding->eTradingStatus = EMPTYHOLDING;    // �ղ�
                    pHolding->fLegOneHoldingCost = 0.0;
                }
                break;
            default:
                break;
            }

        }
        else if(strInstrumentID == m_strLegTwoTicker)
        {
            if(THOST_FTDC_OF_Open == _pOrder->CombOffsetFlag[0])
            {// ����ǿ��ֽ��ף���ôLegTwo�ĳֲ��� = �ɽ���
                pHolding->nLegTwoHoldingVol = _pOrder->VolumeTraded;
            }
            else if(THOST_FTDC_OF_Close == _pOrder->CombOffsetFlag[0] |
                    THOST_FTDC_OF_CloseToday == _pOrder->CombOffsetFlag[0] |
                    THOST_FTDC_OF_CloseYesterday == _pOrder->CombOffsetFlag[0])
            {// �����ƽ�ֽ��ף���ôLegTow�ĳֲ��� = 0��ͬʱ��LegTwo�ĳֲֳɱ�����
                pHolding->nLegTwoHoldingVol = 0;
                pHolding->fLegTwoHoldingCost = 0.0;
            }

            // ���ճֲֵ�ʱ��ͬ�Ľ���״̬�����½���״̬
            switch(pHolding->eTradingStatus)
            {
            case EMPTYHOLDING:  // �ղ�
                if(THOST_FTDC_OF_Open == _pOrder->CombOffsetFlag[0])
                    pHolding->eTradingStatus = LEGTWOSINGLEPOSITIONED;  // LegTwo���Ƚ��֣�LegOneδ����
                break;
            case SENDEDPOSITIONORDER:   // �ѷ�������ί��
                if(THOST_FTDC_OF_Open == _pOrder->CombOffsetFlag[0])
                    pHolding->eTradingStatus = LEGTWOSINGLEPOSITIONED;  // LegTwo���Ƚ��֣�LegOneδ����
                break;
            case LEGONESINGLEPOSITIONED:    // LegOne���Ƚ��֣�LegTwoδ����
                if(THOST_FTDC_OF_Open == _pOrder->CombOffsetFlag[0])
                    pHolding->eTradingStatus = POSITIONED;  // �������
                break;
            case LEGTWOSINGLEPOSITIONED:    // LegTwo���Ƚ��֣�LegOneδ����
                // ��������
                break;
            case POSITIONED:    // �������
                // ��������
                break;
            case SENDEDLEGONELIQUIDATIONORDER:  // �ѷ���LegOne����ƽ��ί��
                // ��������
                break;
            case SENDEDLEGTWOLIQUIDATIONORDER:  // �ѷ���LegTwo����ƽ��ί��
                if(THOST_FTDC_OF_Close == _pOrder->CombOffsetFlag[0] |
                   THOST_FTDC_OF_CloseToday == _pOrder->CombOffsetFlag[0] |
                   THOST_FTDC_OF_CloseYesterday == _pOrder->CombOffsetFlag[0])
                {
                    pHolding->eTradingStatus = EMPTYHOLDING;    // �ղ�
                    pHolding->fLegTwoHoldingCost = 0.0;
                }
                break;
            case SENDEDLIQUIDATIONORDER:    // �ѷ���ƽί��
                if(THOST_FTDC_OF_Close == _pOrder->CombOffsetFlag[0] |
                   THOST_FTDC_OF_CloseToday == _pOrder->CombOffsetFlag[0] |
                   THOST_FTDC_OF_CloseYesterday == _pOrder->CombOffsetFlag[0])
                {
                    pHolding->eTradingStatus = LEGTWOLIQUIDATIONED; // LegTwo����ƽ�֣�LegOneδƽ��
                    pHolding->fLegTwoHoldingCost = 0.0;
                }
                break;
            case LEGONELIQUIDATIONED:   // LegOne����ƽ�֣�LegTwoδƽ��
                if(THOST_FTDC_OF_Close == _pOrder->CombOffsetFlag[0] |
                   THOST_FTDC_OF_CloseToday == _pOrder->CombOffsetFlag[0] |
                   THOST_FTDC_OF_CloseYesterday == _pOrder->CombOffsetFlag[0])
                {
                    pHolding->eTradingStatus = EMPTYHOLDING;    // �ղ�
                    pHolding->fLegTwoHoldingCost = 0.0;
                }
                break;
            case LEGTWOLIQUIDATIONED:   // LegTwo����ƽ�֣�LegOneδƽ��
                // ��������
                break;
            default:
                break;
            }
        }
        break;

    case THOST_FTDC_OST_PartTradedQueueing: // ���ֳɽ����ڶ�����
        // ��������
        break;
    case THOST_FTDC_OST_PartTradedNotQueueing:  // ���ֳɽ����ڶ�����
        // ��������
        break;
    case THOST_FTDC_OST_NoTradeQueueing:    // δ�ɽ����ڶ�����
        // ��������
        break;
    case THOST_FTDC_OST_NoTradeNotQueueing: // δ�ɽ����ڶ�����
        // ��������
        break;
    case THOST_FTDC_OST_Canceled:   // ����
        // ��������
        break;
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

    delete _pOrder;
}

void CCrossArbitrage::handleRtnTrade(int _nOrderRef, CThostFtdcTradeField* _pTrade)
{
//    int nOrderRef = atoi(_trade.OrderRef);
    if(!m_mpOrderRefMapping.contains(_nOrderRef))
        return;
    THoldingVolPair* pHolding = m_mpOrderRefMapping[_nOrderRef];

    // ����ǿ��ֽ��ף�������ۼ�ʵ�ʳֲֳɱ�
    QString strInstrumentID = QString(QLatin1String(_pTrade->InstrumentID));
    if(_pTrade->OffsetFlag == THOST_FTDC_OF_Open)
    {
        if(strInstrumentID == m_strLegOneTicker)
        {
            pHolding->fLegOneHoldingCost += _pTrade->Price * _pTrade->Volume;
        }
        else if(strInstrumentID == m_strLegTwoTicker)
        {
            pHolding->fLegTwoHoldingCost += _pTrade->Price * _pTrade->Volume;
        }
    }

    delete _pTrade;
}

void CCrossArbitrage::sendOrderOnSimulation(int _nOrderRef, QString _strCode, TThostFtdcDirectionType _chDirection,
                                            TThostFtdcOffsetFlagType _chOpenClose, int _nVol, double _fPrice) const
{
    // ���������Ϣ���ļ�

    // ����handleRtnOrder
    CThostFtdcOrderField* pOrder = new CThostFtdcOrderField;
    memset(pOrder,0,sizeof(*pOrder));
    pOrder->OrderSubmitStatus = THOST_FTDC_OSS_InsertSubmitted;
    pOrder->OrderStatus = THOST_FTDC_OST_AllTraded;
    strcpy(pOrder->InstrumentID,_strCode.toStdString().c_str());
    snprintf(pOrder->OrderRef,sizeof(pOrder->OrderRef),"%d",_nOrderRef);
    pOrder->Direction = _chDirection;
    pOrder->CombOffsetFlag[0] = _chOpenClose;
    pOrder->LimitPrice = _fPrice;
    pOrder->VolumeTotalOriginal = _nVol;
    strcpy(pOrder->TradingDay,QDate::currentDate().toString("yyyyMMdd").toStdString().c_str());
    pOrder->VolumeTraded = _nVol;
    pOrder->VolumeTotal = 0;
    strcpy(pOrder->InsertDate,QDate::currentDate().toString("yyyyMMdd").toStdString().c_str());
    strcpy(pOrder->InsertTime,QTime::currentTime().toString("hh:mm:ss").toStdString().c_str());

    handleRtnOrder(_nOrderRef,pOrder);

    // ����handleRtnTrade
    CThostFtdcTradeField* pTrade = new CThostFtdcTradeField;
    memset(pTrade,0,sizeof(*pTrade));
    strcpy(pTrade->InstrumentID,_strCode.toStdString().c_str());
    snprintf(pTrade->OrderRef,sizeof(pTrade->OrderRef),"%d",_nOrderRef);
    pTrade->Direction = _chDirection;
    pTrade->Price = _fPrice;
    pTrade->Volume = _nVol;
    strcpy(pTrade->TradeDate,QDate::currentDate().toString("yyyyMMdd").toStdString().c_str());
    strcpy(pTrade->TradeTime,QTime::currentTime().toString("hh:mm:ss").toStdString().c_str());

    handleRtnTrade(_nOrderRef,pTrade);
}

void CCrossArbitrage::getValidOrderID(int &_requestid, int &_orderref)
{
    QMutexLocker locker(&m_OrderIDMutex);

    _requestid = ++ m_nRequestID;
    _orderref = ++ m_nOrderRef;
//    snprintf(_orderref,sizeof(_orderref),"%d",++m_nOrderRef);
}

bool CCrossArbitrage::isAtTradingTime() const
{
    if(m_tLegOneMktData.tmUpdateTime <= QTime::fromString("09:00:00","hh:mm:ss"))
        return false;
    if(m_tLegOneMktData.tmUpdateTime >= QTime::fromString("11:29::30","hh:mm:ss") &&
            m_tLegOneMktData.tmUpdateTime <= QTime::fromString("13:30:00","hh:mm:ss"))
        return false;
    if(m_tLegOneMktData.tmUpdateTime >= QTime::fromString("14:59:30"))
        return false;
    if(m_tLegTwoMktData.tmUpdateTime <= QTime::fromString("09:00:00","hh:mm:ss"))
        return false;
    if(m_tLegTwoMktData.tmUpdateTime >= QTime::fromString("11:29::30","hh:mm:ss") &&
            m_tLegOneMktData.tmUpdateTime <= QTime::fromString("13:30:00","hh:mm:ss"))
        return false;
    if(m_tLegTwoMktData.tmUpdateTime >= QTime::fromString("14:59:30"))
        return false;
    return true;
}

}
