#include "CCrossArbitrage.h"
#include "Utility/CConfig.h"
#include <stdio.h>
#include <QStringList>
#include <QMutexLocker>
#include <cstdio>

namespace HESS
{

// 初始化requestid和orderref
int CCrossArbitrage::m_nRequestID = 0;
QString CCrossArbitrage::m_strOrderRef = "000000000000";

CCrossArbitrage::CCrossArbitrage(CThostFtdcTraderApi *pUserTraderApi, QString _LegOneTicker,
                                 QString _LegTwoTicker, int _nPairNum, int _nMaxHoldingNum):
                                 m_pUserTraderApi(pUserTraderApi),m_strLegOneTicker(_LegOneTicker),
                                 m_strLegTwoTicker(_LegTwoTicker)
{
    // _nPairNum为本套利对可开仓的套利对数量
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
        // 仅更新买一卖一价和量
        m_tLegOneMktData.AskPrice[0] = _mktData.AskPrice[0];
        m_tLegOneMktData.AskVol[0] = _mktData.AskVol[0];
        m_tLegOneMktData.BidPrice[0] = _mktData.BidPrice[0];
        m_tLegOneMktData.BidVol[0] = _mktData.BidVol[0];
        m_tLegOneMktData.tmUpdateTime = _mktData.tmUpdateTime;
//        locker.unlock();

        // 调用套利交易主函数
        doArbitrage();
    }
    else
    {
        if(_mktData.ticker == m_strLegTwoTicker)
        {
            m_tLegTwoMktData.ticker = _mktData.ticker;
            m_tLegTwoMktData.Last = _mktData.Last;
            // 仅更新买一卖一价和量
            m_tLegTwoMktData.AskPrice[0] = _mktData.AskPrice[0];
            m_tLegTwoMktData.AskVol[0] = _mktData.AskVol[0];
            m_tLegTwoMktData.BidPrice[0] = _mktData.BidPrice[0];
            m_tLegTwoMktData.BidVol[0] = _mktData.BidVol[0];
            m_tLegTwoMktData.tmUpdateTime = _mktData.tmUpdateTime;
//            locker.unlock();

            // 调用套利交易主函数
            doArbitrage();
        }
    }

    // 如果当前为reply模式，则发送sigUpdatedMktData信号
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

// 进行套利交易的主函数
// 流程如下：
// 协整回归参数是否已更新，若未更新return
// 遍历持仓数据list，①如果是“空仓”状态，计算是否触发开仓，若是则开仓。一旦本次行情推送开仓了一对套利对，那么在本次行情推送中
// 不再开仓第二个套利对。②如果是“建仓完成”状态，计算是否触发止盈、止损，若是则平仓。③如果是其他状态，也需要根据情况进行撤单或补单操作
void CCrossArbitrage::doArbitrage()
{
    // 如果协整参数还未更新，则不做处理
    if(!bIfUpdatedCointegrationParam)
        return;

    // 如果不在交易时间，则对现有持仓进行强制平仓


    // 注意锁定次序，先锁定行情互斥锁、再锁定回归参数互斥锁，否则可能会造成死锁
//    QMutexLocker mktLocker(&m_MktMutex);
//    QMutexLocker regressLocker(&m_RegressionParamMutex);

    // 遍历持仓数据list
    bool bIfHasPositionedOnePair = false;   // 本次行情推送是否已开仓了一个套利对
    for(QList<THoldingVolPair*>::const_iterator it = m_HoldingDataList.begin();it != m_HoldingDataList.end();++it)
    {
        THoldingVolPair* pHolding = *it;
        // 根据持仓数据的状态参数采取不同的操作
        switch(pHolding->eTradingStatus)
        {
        // 交易状态参数 = 空仓状态：检验是否出发开仓条件
        case EMPTYHOLDING:
            // 如果本次行情推送已经开过一次仓，那么退出
            if(bIfHasPositionedOnePair)
                break;
            // 如果相关系数在有效区间之外，那么退出
            if(m_tCointegrationParam.fRho > m_fRhoUpperLimit || m_tCointegrationParam.fRho < m_fRhoLowerLimit)
                break;
            // 计算开仓spread
            // 多LegOne，空LegTwo的spread
            double fSellSpread = m_tLegTwoMktData.BidPrice[0]*m_nLegTwoMultiple - (m_tCointegrationParam.fIntercept + m_tCointegrationParam.fSlope*m_tLegOneMktData.AskPrice[0]*m_nLegOneMultiple);
            // 空LegOne，多LegTwo的spread
            double fBuySpread = m_tLegTwoMktData.AskPrice[0]*m_nLegTwoMultiple - (m_tCointegrationParam.fIntercept + m_tCointegrationParam.fSlope*m_tLegOneMktData.BidPrice[0]*m_nLegOneMultiple);
            // 如果fSellSpread > 开仓残差值上限，那么进行short spread开仓（卖出开仓legtwo，买入开仓legone）
            if(fSellSpread > m_tCointegrationParam.fOpenUpperLimit)
            {
                // 计算legone和legtwo的交易手数
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
                // 判断盘口挂单量是否足够
                bool bIfLegOneVolEnough = (m_tLegOneMktData.AskVol[0] >= 2*nLegOneTradeVol);
                bool bIfLegTwoVolEnough = (m_tLegTwoMktData.BidVol[0] >= 2*nLegTwoTradeVol);
                if(bIfLegOneVolEnough && bIfLegTwoVolEnough)
                {// 盘口挂单量足够，short spread开仓
                    // 发出short spread开仓委托
                    SubmitOpenPositionOrder(SHORTSPREAD,pHolding,nLegOneTradeVol,nLegTwoTradeVol);
                    bIfHasPositionedOnePair = true;
                    // 更新持仓参数
                    pHolding->eArbType = SHORTSPREAD;   // 套利类型为“卖出spread”
                    pHolding->nLegOnePlannedVol = nLegOneTradeVol;      // LegOne的计划持仓量
                    pHolding->nLegTwoPlannedVol = nLegTwoTradeVol;      // LegTwo的计划持仓量
                    pHolding->tRegressParam = m_tCointegrationParam;    // 记录开仓时的回归参数
                    pHolding->eTradingStatus = SENDEDPOSITIONORDER;     // 交易状态参数 = 已发出建仓委托
                    pHolding->tmStatusSettingTime = QTime::currentTime();   // 交易状态参数设定时间
                }
            }
            // 如果fBuySpread > 开仓残差值下限，那么进行long spread开仓（买入开仓legtwo，卖出开仓legone）
            else if(fBuySpread < m_tCointegrationParam.fOpenLowerLimit)
            {
                // 计算legone和legtwo的交易手数
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
                // 判断盘口挂单量是否足够
                bool bIfLegOneVolEnough = (m_tLegOneMktData.BidVol[0] >= 2*nLegOneTradeVol);
                bool bIfLegTwoVolEnough = (m_tLegTwoMktData.AskVol[0] >= 2*nLegTwoTradeVol);
                if(bIfLegOneVolEnough && bIfLegTwoVolEnough)
                {// 盘口挂单量足够，buy spread开仓
                    // 发出buy spread开仓委托
                    SubmitOpenPositionOrder(LONGSPREAD,pHolding,nLegOneTradeVol,nLegTwoTradeVol);
                    bIfHasPositionedOnePair =true;
                    //更新持仓参数
                    pHolding->eArbType = LONGSPREAD;    // 套利类型为“买入spread”
                    pHolding->nLegOnePlannedVol = nLegOneTradeVol;      // LegOne的计划持仓量
                    pHolding->nLegTwoPlannedVol = nLegTwoTradeVol;      // LegTwo的计划市场量
                    pHolding->tRegressParam = m_tCointegrationParam;    // 记录开仓时的回归参数
                    pHolding->eTradingStatus = SENDEDPOSITIONORDER;     // 交易状态参数 = 已发出建仓委托
                    pHolding->tmStatusSettingTime = QTime::currentTime();   // 交易状态参数设定时间
                }
            }
            break;
        // 交易状态参数 = 已发出建仓委托
        case SENDEDPOSITIONORDER:
            // 如果当前时间距离发出建仓委托时间超过3秒钟（3000毫秒），则将交易状态该为空仓
            if(pHolding->tmStatusSettingTime.msecsTo(QTime::currentTime()) > ELAPSEMSECS)
            {
                pHolding->eTradingStatus = EMPTYHOLDING;   // 交易状态参数 = 空仓
                pHolding->tmStatusSettingTime = QTime::currentTime();   // 交易状态参数设定时间
            }
            break;
        // 交易状态参数 = LegOne单腿建仓,LegTwo未建仓
        case LEGONESINGLEPOSITIONED:
            // 如果当前时间距离最新交易状态设定时间超过3秒钟（3000毫秒）,则重新计算最新价差，如果还是处于触发开仓条件，那么继续开仓LegTwo，否则将已开仓的LegOne平仓
            if(pHolding->tmStatusSettingTime.msecsTo(QTime::currentTime()) > ELAPSEMSECS)
            {
                double fLatestSpread = 0.0;
                double fLegOneUnitCost = pHolding->fLegOneHoldingCost / pHolding->nLegOneHoldingVol;
                if(SHORTSPREAD == pHolding->eArbType)
                {// 卖空价差的情况
                    fLatestSpread = m_tLegTwoMktData.BidPrice[0]*m_nLegTwoMultiple - (pHolding->tRegressParam.fIntercept + pHolding->tRegressParam.fSlope*fLegOneUnitCost*m_nLegOneMultiple);
                    if(fLatestSpread > pHolding->tRegressParam.fOpenUpperLimit)
                    {// 如果最新的卖空价差依旧触发开仓，那么卖出开仓LegTwo
                        sendOrder(m_strLegTwoTicker,THOST_FTDC_D_Sell,THOST_FTDC_OF_Open,pHolding->nLegTwoPlannedVol,m_tLegTwoMktData.BidPrice[0],pHolding);
                        pHolding->tmStatusSettingTime = QTime::currentTime();
                    }
                    else
                    {// 如果最新的卖空价差未触发开仓，那么对LegOne进行平仓(卖出平仓)
                        sendOrder(m_strLegOneTicker,THOST_FTDC_D_Sell,THOST_FTDC_OF_Close,pHolding->nLegOneHoldingVol,m_tLegOneMktData.BidPrice[0],pHolding);
                        pHolding->eTradingStatus = SENDEDLEGONELIQUIDATIONORDER;    // 交易状态参数 = 已发出LegOne单腿平仓委托
                        pHolding->tmStatusSettingTime = QTime::currentTime();
                    }
                }
                else if(LONGSPREAD == pHolding->eArbType)
                {// 买入价差的情况
                    fLatestSpread = m_tLegTwoMktData.AskPrice[0]*m_nLegTwoMultiple - (pHolding->tRegressParam.fIntercept + pHolding->tRegressParam.fSlope*fLegOneUnitCost*m_nLegOneMultiple);
                    if(fLatestSpread < pHolding->tRegressParam.fOpenLowerLimit)
                    {// 如果最新的买入价差依旧触发开仓，那么买入开仓LegTwo
                        sendOrder(m_strLegTwoTicker,THOST_FTDC_D_Buy,THOST_FTDC_OF_Open,pHolding->nLegTwoPlannedVol,m_tLegTwoMktData.AskPrice[0],pHolding);
                        pHolding->tmStatusSettingTime = QTime::currentTime();
                    }
                    else
                    {// 如果最新的买入价差未触发开仓，那么对LegOne进行平仓(买入平仓)
                        sendOrder(m_strLegOneTicker,THOST_FTDC_D_Buy,THOST_FTDC_OF_Close,pHolding->nLegOneHoldingVol,m_tLegOneMktData.AskPrice[0],pHolding);
                        pHolding->eTradingStatus = SENDEDLEGONELIQUIDATIONORDER;    // 交易状态参数 = 已发出LegOne单腿平仓委托
                        pHolding->tmStatusSettingTime = QTime::currentTime();
                    }
                }
            }
            break;
        // 交易状态参数 = LegTwo单腿建仓,LegOne未建仓
        case LEGTWOSINGLEPOSITIONED:
            // 如果当前时间距离最新交易状态设定时间操作3秒钟（3000毫秒），则重新计算最新价差，如果还是处于触发开仓条件，那么继续开仓LegOne，否则将已开仓的LegTwo平仓
            if(pHolding->tmStatusSettingTime.msecsTo(QTime::currentTime()) > ELAPSEMSECS)
            {
                double fLatestSpread = 0.0;
                double fLegTwoUnitCost = pHolding->fLegTwoHoldingCost / pHolding->nLegTwoHoldingVol;
                if(SHORTSPREAD == pHolding->eArbType)
                {// 卖空价差的情况
                    fLatestSpread = fLegTwoUnitCost*m_nLegTwoMultiple - (pHolding->tRegressParam.fIntercept + pHolding->tRegressParam.fSlope*m_tLegOneMktData.AskPrice[0]*m_nLegOneMultiple);
                    if(fLatestSpread > pHolding->tRegressParam.fOpenUpperLimit)
                    {// 如果最新的卖出价差依旧触发开仓，那么买入开仓LegOne
                        sendOrder(m_strLegOneTicker,THOST_FTDC_D_Buy,THOST_FTDC_OF_Open,pHolding->nLegOnePlannedVol,m_tLegOneMktData.AskPrice[0],pHolding);
                        pHolding->tmStatusSettingTime = QTime::currentTime();
                    }
                    else
                    {// 如果最新的卖出价差未触发开仓，那么对LegTwo进行平仓(买入平仓)
                        sendOrder(m_strLegTwoTicker,THOST_FTDC_D_Buy,THOST_FTDC_OF_Close,pHolding->nLegTwoHoldingVol,m_tLegTwoMktData.AskPrice[0],pHolding);
                        pHolding->eTradingStatus = SENDEDLEGTWOLIQUIDATIONORDER;    // 交易状态参数 = 已发出LegTwo单腿平仓
                        pHolding->tmStatusSettingTime = QTime.currentTime();
                    }
                }
                else if(LONGSPREAD == pHolding->eArbType)
                {// 买入价差的情况
                    fLatestSpread = fLegTwoUnitCost*m_nLegTwoMultiple - (pHolding->tRegressParam.fIntercept + pHolding->tRegressParam.fSlope*m_tLegOneMktData.BidPrice[0]*m_nLegOneMultiple);
                    if(fLatestSpread < pHolding->tRegressParam.fOpenLowerLimit)
                    {// 如果最新的买入价差依旧触发开仓，那么卖出开仓LegOne
                        sendOrder(m_strLegOneTicker,THOST_FTDC_D_Sell,THOST_FTDC_OF_Open,pHolding->nLegOnePlannedVol,m_tLegOneMktData.BidPrice[0],pHolding);
                        pHolding->tmStatusSettingTime = QTime::currentTime();
                    }
                    else
                    {// 如果最新的买入价差未触发开仓，那么对LegTwo进行平仓(卖出平仓)
                        sendOrder(m_strLegTwoTicker,THOST_FTDC_D_Sell,THOST_FTDC_OF_Close,pHolding->nLegTwoHoldingVol,m_tLegTwoMktData.BidPrice[0],pHolding);
                        pHolding->eTradingStatus = SENDEDLEGTWOLIQUIDATIONORDER;    // 交易状态参数 = 已发出LegTwo单腿平仓
                        pHolding->tmStatusSettingTime = QTime::currentTime();
                    }
                }
            }
            break;
        // 交易状态参数 = 建仓完成
        case POSITIONED:
            // 检验是否触发止盈或止损
            if(SHORTSPREAD == pHolding->eArbType)
            {// 当前持仓为卖出价差持仓
                // 根据建仓时的回归参数计算卖出价差持仓的最新平仓价差
                double fCloseSpread = m_tLegTwoMktData.AskPrice[0]*m_nLegTwoMultiple - (pHolding->tRegressParam.fIntercept+pHolding->tRegressParam.fSlope*m_tLegOneMktData.BidPrice[0]*m_nLegOneMultiple);
                // 如果卖出价差持仓的最新平仓价差超过(开仓价差+止盈止损时对应的残差变动值)，那么平仓止损
                if(fCloseSpread > pHolding->fOpenError + pHolding->tRegressParam.fErrorChgForExitTrade)
                {
                    SubmitClosePositionOrder(SHORTSPREAD,pHolding,pHolding->nLegOneHoldingVol,pHolding->nLegTwoHoldingVol);
                    // 更新持仓参数
                    pHolding->eTradingStatus = SENDEDLIQUIDATIONORDER;  // 交易状态参数 = 已发出平仓委托
                    pHolding->tmStatusSettingTime = QTime::currentTime();   // 交易状态参数设定时间
                }
                // 如果卖出价差持仓的最新平仓价差小于(开仓价差-止盈止损对应的残差变动值)，那么平仓止盈
                else if(fCloseSpread < pHolding->fOpenError - pHolding->tRegressParam.fErrorChgForExitTrade)
                {
                    SubmitClosePositionOrder(SHORTSPREAD,pHolding,pHolding->nLegOneHoldingVol,pHolding->nLegTwoHoldingVol);
                    // 更新持仓参数
                    pHolding->eTradingStatus = SENDEDLIQUIDATIONORDER;  // 已发出平仓委托
                    pHolding->tmStatusSettingTime = QTime::currentTime();   // 交易状态参数设定时间
                }
            }
            else if(LONGSPREAD == pHolding->eArbType)
            {// 当前持仓为买入价差持仓
                // 根据建仓时的回归参数计算买入价差持仓的最新平仓价差
                double fCloseSpread = m_tLegTwoMktData.BidPrice[0]*m_nLegTwoMultiple - (pHolding->tRegressParam.fIntercept+pHolding->tRegressParam.fSlope*m_tLegOneMktData.AskPrice[0]*m_nLegOneMultiple);
                // 如果买入价差持仓的最新平仓价差小于(开仓价差-止盈止损对应的残差变动值)，那么平仓止损
                if(fCloseSpread < pHolding->fOpenError - pHolding->tRegressParam.fErrorChgForExitTrade)
                {
                    SubmitClosePositionOrder(LONGSPREAD,pHolding,pHolding->nLegOneHoldingVol,pHolding->nLegTwoHoldingVol);
                    // 更新状态参数
                    pHolding->eTradingStatus = SENDEDLIQUIDATIONORDER;  // 已发出平仓委托
                    pHolding->tmStatusSettingTime = QTime::currentTime();   // 交易状态参数设定时间
                }
                // 如果买入价差持仓的最新平仓价差大于(开仓价差+止盈止损对应的残差变动值)，那么平仓止盈
                else if(fCloseSpread > pHolding->fOpenError + pHolding->tRegressParam.fErrorChgForExitTrade)
                {
                    SubmitClosePositionOrder(LONGSPREAD,pHolding,pHolding->nLegOneHoldingVol,pHolding->nLegTwoHoldingVol);
                    // 更新状态参数
                    pHolding->eTradingStatus = SENDEDLIQUIDATIONORDER;      // 已发出平仓委托
                    pHolding->tmStatusSettingTime = QTime::currentTime();
                }
            }
            break;
        // 交易状态参数 = 已发出LegOne单腿平仓委托
        case SENDEDLEGONELIQUIDATIONORDER:
            // 如果当前时间距离最新交易状态设定时间超过3秒钟（3000毫秒），则再次发出LegOne的平仓委托
            if(pHolding->tmStatusSettingTime.msecsTo(QTime::currentTime()) > ELAPSEMSECS)
            {
                if(SHORTSPREAD == pHolding->eArbType)
                {// 如果是卖空价差，则LegOne卖出平仓
                    sendOrder(m_strLegOneTicker,THOST_FTDC_D_Sell,THOST_FTDC_OF_Close,pHolding->nLegOneHoldingVol,m_tLegOneMktData.BidPrice[0],pHolding);
                    pHolding->tmStatusSettingTime = QTime::currentTime();
                }
                else if(LONGSPREAD == pHolding->eArbType)
                {// 如果是买入价差，则LegOne买入平仓
                    sendOrder(m_strLegOneTicker,THOST_FTDC_D_Buy,THOST_FTDC_OF_Close,pHolding->nLegOneHoldingVol,m_tLegOneMktData.AskPrice[0],pHolding);
                    pHolding->tmStatusSettingTime = QTime::currentTime();
                }
            }
            break;
        // 交易状态 = 已发出LegTwo单腿平仓委托
        case SENDEDLEGTWOLIQUIDATIONORDER:
            // 如果当前时间距离最新交易状态设定时间超过3秒钟（3000毫秒），则再次发出LegTwo的平仓委托
            if(pHolding->tmStatusSettingTime.msecsTo(QTime::currentTime()) > ELAPSEMSECS)
            {
                if(SHORTSPREAD == pHolding->eArbType)
                {// 如果是卖空价差，则LegTwo买入平仓
                    sendOrder(m_strLegTwoTicker,THOST_FTDC_D_Buy,THOST_FTDC_OF_Close,pHolding->nLegTwoHoldingVol,m_tLegTwoMktData.AskPrice[0],pHolding);
                }
                else if(LONGSPREAD == pHolding->eArbType)
                {// 如果是买入价差，则LegTwo卖出平仓
                    sendOrder(m_strLegTwoTicker,THOST_FTDC_D_Sell,THOST_FTDC_OF_Close,pHolding->nLegTwoHoldingVol,m_tLegTwoMktData.BidPrice[0],pHolding);
                }
                pHolding->tmStatusSettingTime = QTime::currentTime();
            }
            break;
        // 交易状态参数 = 已发出平仓委托
        case SENDEDLIQUIDATIONORDER:
            // 如果当前时间距离最新交易状态设定时间超过3秒钟（3000毫秒），那么重新发送平仓委托
            if(pHolding->tmStatusSettingTime.msecsTo(QTime::currentTime()) > ELAPSEMSECS)
            {
                SubmitClosePositionOrder(pHolding->eArbType,pHolding,pHolding->nLegOneHoldingVol,pHolding->nLegTwoHoldingVol);
                pHolding->tmStatusSettingTime = QTime::currentTime();
            }
            break;
        // 交易状态参数 = LegOne单腿平仓,LegTwo未平仓
        case LEGONELIQUIDATIONED:
            // 如果当前时间距离最新交易状态设定时间超过3秒钟（3000毫秒），那么重新发送LegTwo的平仓委托
            if(pHolding->tmStatusSettingTime.msecsTo(QTime::currentTime()) > ELAPSEMSECS)
            {
                if(SHORTSPREAD == pHolding->eArbType)
                {// 如果是卖空价差，则LegTwo买入平仓
                    sendOrder(m_strLegTwoTicker,THOST_FTDC_D_Buy,THOST_FTDC_OF_Close,pHolding->nLegTwoHoldingVol,m_tLegTwoMktData.AskPrice[0],pHolding);
                }
                else if(LONGSPREAD == pHolding->eArbType)
                {// 如果是买入价差，则LegTow卖出平仓
                    sendOrder(m_strLegTwoTicker,THOST_FTDC_D_Sell,THOST_FTDC_OF_Close,pHolding->nLegTwoHoldingVol,m_tLegTwoMktData.BidPrice[0],pHolding);
                }
                pHolding->tmStatusSettingTime = QTime::currentTime();
            }
            break;
        // 交易状态参数 = LegTwo单腿平仓,LegOne未平仓
        case LEGTWOLIQUIDATIONED:
            // 如果当前时间距离最新交易状态设定时间超过3秒钟（3000毫秒），哪儿重新发送LegOne的平仓委托
            if(pHolding->tmStatusSettingTime.msecsTo(QTime::currentTime()) > ELAPSEMSECS)
            {
                if(SHORTSPREAD == pHolding->eArbType)
                {// 如果是卖空价差，则LegOne卖出平仓
                    sendOrder(m_strLegOneTicker,THOST_FTDC_D_Sell,THOST_FTDC_OF_Close,pHolding->nLegOneHoldingVol,m_tLegOneMktData.BidPrice[0],pHolding);
                }
                else if(LONGSPREAD == pHolding->eArbType)
                {// 如果是买入价差，则LegTOne买入平仓
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

// 开仓委托
// _arbtype = 套利类型（卖出spread，买入spread）
// _nLegOneVol = legone的委托数量
// _nLegTwoVol = legtwo的委托数量
void CCrossArbitrage::SubmitOpenPositionOrder(ARBITRAGETYPE _arbtype,THoldingVolPair* _pHolding ,int _nLegOneVol, int _nLegTwoVol) const
{
    // 将LegOne和LegTwo的持仓数据清零
    _pHolding->nLegOneHoldingVol = 0;
    _pHolding->nLegOnePlannedVol = 0;
    _pHolding->fLegOneHoldingCost = 0.0;
    _pHolding->nLegTwoHoldingVol = 0;
    _pHolding->nLegTwoPlannedVol = 0;
    _pHolding->fLegTwoHoldingCost = 0.0;

    if(SHORTSPREAD == _arbtype)
    {// 卖出价差，LegTwo卖出开仓、LegOne买入开仓
        sendOrder(m_strLegOneTicker,THOST_FTDC_D_Buy,THOST_FTDC_OF_Open,_nLegOneVol,m_tLegOneMktData.AskPrice[0],_pHolding);
        sendOrder(m_strLegTwoTicker,THOST_FTDC_D_Sell,THOST_FTDC_OF_Open,_nLegTwoVol,m_tLegTwoMktData.BidPrice[0],_pHolding);
    }
    else if(LONGSPREAD == _arbtype)
    {// 买入价差，LegTwo买入开仓、LegOne卖出开仓
        sendOrder(m_strLegOneTicker,THOST_FTDC_D_Sell,THOST_FTDC_OF_Open,_nLegOneVol,m_tLegOneMktData.BidPrice[0],_pHolding);
        sendOrder(m_strLegTwoTicker,THOST_FTDC_D_Buy,THOST_FTDC_OF_Open,_nLegTwoVol,m_tLegTwoMktData.AskPrice[0],_pHolding);
    }
}

// 平仓委托（包括止盈、止损、收盘前平仓）
// _arbtype = 套利类型（卖出spread，买入spread）
// _nLegOneVol = legone的委托数量
// _nLegTwoVol = legtwo的委托数量
void CCrossArbitrage::SubmitClosePositionOrder(ARBITRAGETYPE _arbtype, THoldingVolPair *_pHolding, int _nLegOneVol, int _nLegTwoVol) const
{
    if(SHORTSPREAD == _arbtype)
    {// 卖出价差，LegTwo买入平仓、LegOne卖出平仓
        sendOrder(m_strLegOneTicker,THOST_FTDC_D_Sell,THOST_FTDC_OF_Close,_nLegOneVol,m_tLegOneMktData.BidPrice[0],_pHolding);
        sendOrder(m_strLegTwoTicker,THOST_FTDC_D_Buy,THOST_FTDC_OF_Close,_nLegTwoVol,m_tLegTwoMktData.AskPrice[0],_pHolding);
    }
    else if(LONGSPREAD == _arbtype)
    {// 买入价差，LegTwo卖出平仓、LegOne买入平仓
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
    {// 如果当前是生产模式，正常向CTP发送委托
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

        // 采用FOK报单
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
    {// 如果当前是simnow或reply模式，调用sendOrderOnSimlation
        sendOrderOnSimulation(nOrderRef,_strCode,_chDirection,_chOpenClose,_nVol,_fPrice);
    }
}

void CCrossArbitrage::handleRtnOrder(int _nOrderRef, CThostFtdcOrderField* _pOrder)
{
    // 取得报单引用对应的持仓数据指针
//    QMutexLocker mappingLocker(&m_MappingMutex);
//    int nOrderRef = atoi(_pOrder->OrderRef);
    if(!m_mpOrderRefMapping.contains(_nOrderRef))
        return;
    THoldingVolPair* pHolding = m_mpOrderRefMapping[_nOrderRef];

    // 根据报单提交状态分别处理
    switch(_pOrder->OrderSubmitStatus)
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
        // 从报单引用索引中删除对应的记录
        m_mpOrderRefMapping.remove(nOrderRef);
        break;
    case THOST_FTDC_OSS_CancelRejected:     // 撤单已经被拒绝
        break;
    case THOST_FTDC_OSS_ModifyRejected:     // 改单已经被拒绝
        break;
    default:
        break;
    }

    // 根据报单状态分别处理
    QString strInstrumentID = QString(QLatin1String(_pOrder->InstrumentID));
    switch(_pOrder->OrderStatus)
    {
    case THOST_FTDC_OST_AllTraded:      // 全部成交
        // 设置套利对持仓量及交易状态参数
        if(strInstrumentID == m_strLegOneTicker)
        {
            if(THOST_FTDC_OF_Open == _pOrder->CombOffsetFlag[0])
            {// 如果是开仓交易，那么LegOne的持仓量 = 成交量
                pHolding->nLegOneHoldingVol = _pOrder->VolumeTraded;
            }
            else if(THOST_FTDC_OF_Close == _pOrder->CombOffsetFlag[0] |
                    THOST_FTDC_OF_CloseToday == _pOrder->CombOffsetFlag[0] |
                    THOST_FTDC_OF_CloseYesterday == _pOrder->CombOffsetFlag[0])
            {// 如果是平仓交易，那么LegOne的持仓量 = 0，同时将LegOne的持仓成本清零
                pHolding->nLegOneHoldingVol = 0;
                pHolding->fLegOneHoldingCost = 0.0;
            }

            // 按照持仓当时不同的交易状态，更新交易状态
            switch(pHolding->eTradingStatus)
            {
            case EMPTYHOLDING:  // 空仓
                if(THOST_FTDC_OF_Open == _pOrder->CombOffsetFlag[0])
                    pHolding->eTradingStatus = LEGONESINGLEPOSITIONED;  // LegOne单腿建仓，LegTwo未建仓
                break;
            case SENDEDPOSITIONORDER:   // 已发出建仓委托
                if(THOST_FTDC_OF_Open == _pOrder->CombOffsetFlag[0])
                    pHolding->eTradingStatus = LEGONESINGLEPOSITIONED;  // LegOne单腿建仓，LegTwo未建仓
                break;
            case LEGONESINGLEPOSITIONED:    // LegOne单腿建仓，LegTwo未建仓
                // 不做处理
                break;
            case LEGTWOSINGLEPOSITIONED:    // LegTwo单腿建仓，LegOne未建仓
                if(THOST_FTDC_OF_Open == _pOrder->CombOffsetFlag[0])
                    pHolding->eTradingStatus = POSITIONED;  // 建仓完成
                break;
            case POSITIONED:    // 建仓完成
                // 不做处理
                break;
            case SENDEDLEGONELIQUIDATIONORDER:  // 已发出LegOne单腿平仓委托
                if(THOST_FTDC_OF_Close == _pOrder->CombOffsetFlag[0] |
                   THOST_FTDC_OF_CloseToday == _pOrder->CombOffsetFlag[0] |
                   THOST_FTDC_OF_CloseYesterday == _pOrder->CombOffsetFlag[0])
                {
                    pHolding->eTradingStatus = EMPTYHOLDING;    // 空仓
                    pHolding->fLegOneHoldingCost = 0.0;
                }
                break;
            case SENDEDLEGTWOLIQUIDATIONORDER:  // 已发出LegTwo单腿平仓委托
                // 不做处理
                break;
            case SENDEDLIQUIDATIONORDER:    // 已发出平仓委托
                if(THOST_FTDC_OF_Close == _pOrder->CombOffsetFlag[0] |
                   THOST_FTDC_OF_CloseToday == _pOrder->CombOffsetFlag[0] |
                   THOST_FTDC_OF_CloseYesterday == _pOrder->CombOffsetFlag[0])
                {
                    pHolding->eTradingStatus = LEGONELIQUIDATIONED;     // LegOne单腿平仓，LegTwo未平仓
                    pHolding->fLegOneHoldingCost - 0.0;
                }
                break;
            case LEGONELIQUIDATIONED:       // LegOne单腿平仓，LegTwo未平仓
                // 不做处理
                break;
            case LEGTWOLIQUIDATIONED:       // LegTwo单腿平仓，LegOne未平仓
                if(THOST_FTDC_OF_Close == _pOrder->CombOffsetFlag[0] |
                   THOST_FTDC_OF_CloseToday == _pOrder->CombOffsetFlag[0] |
                   THOST_FTDC_OF_CloseYesterday == _pOrder->CombOffsetFlag[0])
                {
                    pHolding->eTradingStatus = EMPTYHOLDING;    // 空仓
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
            {// 如果是开仓交易，那么LegTwo的持仓量 = 成交量
                pHolding->nLegTwoHoldingVol = _pOrder->VolumeTraded;
            }
            else if(THOST_FTDC_OF_Close == _pOrder->CombOffsetFlag[0] |
                    THOST_FTDC_OF_CloseToday == _pOrder->CombOffsetFlag[0] |
                    THOST_FTDC_OF_CloseYesterday == _pOrder->CombOffsetFlag[0])
            {// 如果是平仓交易，那么LegTow的持仓量 = 0，同时将LegTwo的持仓成本清零
                pHolding->nLegTwoHoldingVol = 0;
                pHolding->fLegTwoHoldingCost = 0.0;
            }

            // 按照持仓当时不同的交易状态，更新交易状态
            switch(pHolding->eTradingStatus)
            {
            case EMPTYHOLDING:  // 空仓
                if(THOST_FTDC_OF_Open == _pOrder->CombOffsetFlag[0])
                    pHolding->eTradingStatus = LEGTWOSINGLEPOSITIONED;  // LegTwo单腿建仓，LegOne未建仓
                break;
            case SENDEDPOSITIONORDER:   // 已发出建仓委托
                if(THOST_FTDC_OF_Open == _pOrder->CombOffsetFlag[0])
                    pHolding->eTradingStatus = LEGTWOSINGLEPOSITIONED;  // LegTwo单腿建仓，LegOne未建仓
                break;
            case LEGONESINGLEPOSITIONED:    // LegOne单腿建仓，LegTwo未建仓
                if(THOST_FTDC_OF_Open == _pOrder->CombOffsetFlag[0])
                    pHolding->eTradingStatus = POSITIONED;  // 建仓完成
                break;
            case LEGTWOSINGLEPOSITIONED:    // LegTwo单腿建仓，LegOne未建仓
                // 不做处理
                break;
            case POSITIONED:    // 建仓完成
                // 不做处理
                break;
            case SENDEDLEGONELIQUIDATIONORDER:  // 已发出LegOne单腿平仓委托
                // 不做处理
                break;
            case SENDEDLEGTWOLIQUIDATIONORDER:  // 已发出LegTwo单腿平仓委托
                if(THOST_FTDC_OF_Close == _pOrder->CombOffsetFlag[0] |
                   THOST_FTDC_OF_CloseToday == _pOrder->CombOffsetFlag[0] |
                   THOST_FTDC_OF_CloseYesterday == _pOrder->CombOffsetFlag[0])
                {
                    pHolding->eTradingStatus = EMPTYHOLDING;    // 空仓
                    pHolding->fLegTwoHoldingCost = 0.0;
                }
                break;
            case SENDEDLIQUIDATIONORDER:    // 已发出平委托
                if(THOST_FTDC_OF_Close == _pOrder->CombOffsetFlag[0] |
                   THOST_FTDC_OF_CloseToday == _pOrder->CombOffsetFlag[0] |
                   THOST_FTDC_OF_CloseYesterday == _pOrder->CombOffsetFlag[0])
                {
                    pHolding->eTradingStatus = LEGTWOLIQUIDATIONED; // LegTwo单腿平仓，LegOne未平仓
                    pHolding->fLegTwoHoldingCost = 0.0;
                }
                break;
            case LEGONELIQUIDATIONED:   // LegOne单腿平仓，LegTwo未平仓
                if(THOST_FTDC_OF_Close == _pOrder->CombOffsetFlag[0] |
                   THOST_FTDC_OF_CloseToday == _pOrder->CombOffsetFlag[0] |
                   THOST_FTDC_OF_CloseYesterday == _pOrder->CombOffsetFlag[0])
                {
                    pHolding->eTradingStatus = EMPTYHOLDING;    // 空仓
                    pHolding->fLegTwoHoldingCost = 0.0;
                }
                break;
            case LEGTWOLIQUIDATIONED:   // LegTwo单腿平仓，LegOne未平仓
                // 不做处理
                break;
            default:
                break;
            }
        }
        break;

    case THOST_FTDC_OST_PartTradedQueueing: // 部分成交还在队列中
        // 不做处理
        break;
    case THOST_FTDC_OST_PartTradedNotQueueing:  // 部分成交不在队列中
        // 不做处理
        break;
    case THOST_FTDC_OST_NoTradeQueueing:    // 未成交还在队列中
        // 不做处理
        break;
    case THOST_FTDC_OST_NoTradeNotQueueing: // 未成交不在队列中
        // 不做处理
        break;
    case THOST_FTDC_OST_Canceled:   // 撤单
        // 不做处理
        break;
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

    delete _pOrder;
}

void CCrossArbitrage::handleRtnTrade(int _nOrderRef, CThostFtdcTradeField* _pTrade)
{
//    int nOrderRef = atoi(_trade.OrderRef);
    if(!m_mpOrderRefMapping.contains(_nOrderRef))
        return;
    THoldingVolPair* pHolding = m_mpOrderRefMapping[_nOrderRef];

    // 如果是开仓交易，则计算累计实际持仓成本
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
    // 输出交易信息至文件

    // 调用handleRtnOrder
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

    // 调用handleRtnTrade
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
