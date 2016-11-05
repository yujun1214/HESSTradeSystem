#include "CMMDispatcher.h"
#include <QSet>
#include <cstdlib>
#include "MarketData/CMktDataBuffer.h"

namespace HESS
{

void CMMDispatcher::addOrderInterval(int _nInterval)
{
    if(m_lstOrderInterval.contains(_nInterval))
        return;
    m_lstOrderInterval.push_back(_nInterval);
}
/*
bool CMMDispatcher::acquireBuyOrderPrice(double &_fOrderPrice)
{
    // 取得最新买一价
    int nCurrBidPrice = (int)(CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getDepthMktData(m_strSecuCode).BidPrice[0] * m_nPricePrecision + 0.5);

    QMutexLocker locker(&m_lock);
    // 遍历买入委托价格向量，计算每个买入委托价格与买一价的距离，存放到set中
    QSet<int> setUsedInterval;
    for(QVector<int>::const_iterator itBuyOrder = m_vBuyOrderPrice.begin();itBuyOrder != m_vBuyOrderPrice.end();++itBuyOrder)
    {
        int nInterval = calcInterval(nCurrBidPrice,*itBuyOrder);
        for(QList<int>::const_iterator itInterval = m_lstOrderInterval.begin();itInterval != m_lstOrderInterval.end();++itInterval)
        {
            if(nInterval <= *itInterval)
            {
                setUsedInterval.insert(*itInterval);
                break;
            }
        }
    }

    // 取得未使用的最小距离值
    QList<int> lstRemainInterval = QSet::fromList(m_lstOrderInterval).subtract(setUsedInterval).toList();
    int nMinRemainInterval;
    if(lstRemainInterval.empty())
        return false;
    else
    {
        nMinRemainInterval = lstRemainInterval.front();
        for(QList<int>::const_iterator it = lstRemainInterval.begin();it != lstRemainInterval.end();++it)
        {
            if(*it < nMinRemainInterval)
                nMinRemainInterval = *it;
        }
    }

    // 计算未使用的最小距离值对应的买入委托价格，并将该价格添加至买入委托价格向量
    m_vBuyOrderPrice.push_back(nCurrBidPrice - nMinRemainInterval * m_nPriceTick);
    _fOrderPrice = (nCurrBidPrice - nMinRemainInterval * m_nPriceTick) / (double)m_nPricePrecision;
    return true;

} // acquireBuyOrderPrice
*/
bool CMMDispatcher::acquireBuyOrderPrice(long &_nIndex, double &_fOrderPrice)
{
    // 取得最新买一价
//    int nCurrBidPrice = (int)(CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getDepthMktData(m_strSecuCode).BidPrice[0] * m_nPricePrecision + 0.5);
    int nCurrBidPrice = (int)(CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(m_strSecuCode).BidPrice * m_nPricePrecision + 0.5);

    QMutexLocker locker(&m_lock);
    QSet<int> setUsedInterval;
    // 遍历买入委托价格map，计算每个买入委托价格与买一价之间的距离，存放到set中
    for(QMap<long,double>::const_iterator itBuyOrder = m_mpBuyOrderPrice.begin();itBuyOrder != m_mpBuyOrderPrice.end();++itBuyOrder)
    {
        int nInterval = calcInterval(nCurrBidPrice,(int)(itBuyOrder.value() * m_nPricePrecision + 0.5));
        if(nInterval > m_lstOrderInterval.back())
            setUsedInterval.insert(m_lstOrderInterval.back());
        else
        {
            for(QList<int>::const_iterator itInterval = m_lstOrderInterval.begin();itInterval != m_lstOrderInterval.end();++itInterval)
            {
                if(nInterval <= *itInterval)
                {
                    setUsedInterval.insert(*itInterval);
                    break;
                }
            }
        }
    }

    // 取得未使用的最小距离值
    QList<int> lstRemainInterval = QSet<int>::fromList(m_lstOrderInterval).subtract(setUsedInterval).toList();
    int nMinRemainInterval;
    if(lstRemainInterval.empty())
        return false;
    else
    {
        nMinRemainInterval = lstRemainInterval.front();
        for(QList<int>::const_iterator it = lstRemainInterval.begin();it != lstRemainInterval.end();++it)
        {
            if(*it < nMinRemainInterval)
                nMinRemainInterval = *it;
        }
    }

    // 计算未使用的最小距离值对应的买入委托价格，并将该价格添加至买入委托价格map
    _fOrderPrice = (nCurrBidPrice - nMinRemainInterval * m_nPriceTick) / (double)m_nPricePrecision;
    _nIndex = ++m_nIndex;
    m_mpBuyOrderPrice.insert(_nIndex,_fOrderPrice);
    return true;
} // acquireBuyOrderPrice

bool CMMDispatcher::acquireSellCloseOrderPrice(long &_nIndex, double &_fClosePrice)
{
    // 取得最新卖一价
//    int nCurrAskPrice = (int)(CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getDepthMktData(m_strSecuCode).AskPrice[0] * m_nPricePrecision + 0.5);
    int nCurrAskPrice = (int)(CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(m_strSecuCode).AskPrice + 0.5);
    QMutexLocker locker(&m_lock);

    // 遍历卖出平仓委托价格map，计算每个卖出平仓委托价格与卖一价的距离，存放到set中
    QSet<int> setUsedInterval;
    for(QMap<long,double>::const_iterator itSellClsOrder = m_mpSellCloseOrderPrice.begin();itSellClsOrder != m_mpSellCloseOrderPrice.end();++itSellClsOrder)
    {
        int nInterval = calcInterval(nCurrAskPrice,(int)(itSellClsOrder.value() * m_nPricePrecision + 0.5));
        if(nInterval > m_lstOrderInterval.back())
            setUsedInterval.insert(m_lstOrderInterval.back());
        else
        {
            for(QList<int>::const_iterator itInterval = m_lstOrderInterval.begin();itInterval != m_lstOrderInterval.end();++itInterval)
            {
                if(nInterval <= *itInterval)
                {
                    setUsedInterval.insert(*itInterval);
                    break;
                }
            }
        }
    }

    // 取得未使用的最小距离值
    QList<int> lstRemainInterval = QSet<int>::fromList(m_lstOrderInterval).subtract(setUsedInterval).toList();
    int nMinRemainInterval;
    if(lstRemainInterval.empty())
        return false;
    else
    {
        nMinRemainInterval = lstRemainInterval.front();
        for(QList<int>::const_iterator it = lstRemainInterval.begin();it != lstRemainInterval.end();++it)
        {
            if(*it < nMinRemainInterval)
                nMinRemainInterval = *it;
        }
    }

    // 计算未使用的最小距离值对应的卖出平仓委托价格，并将该价格添加至卖出平仓委托价格map中
    _fClosePrice = (nCurrAskPrice + nMinRemainInterval * m_nPriceTick) / (double)m_nPricePrecision;
    _nIndex = ++m_nIndex;
    m_mpSellCloseOrderPrice.insert(_nIndex,_fClosePrice);
    return true;
} // acquireSellCloseOrderPrice

/*
bool CMMDispatcher::acquireSellOrderPrice(double &_fOrderPrice)
{
    // 取得最新卖一价
    int nCurrAskPrice = (int)(CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getDepthMktData(m_strSecuCode).AskPrice[0] * m_nPricePrecision + 0.5);

    QMutexLocker locker(&m_lock);
    // 遍历卖出委托价格向量，计算每个卖出委托价格与卖一价的距离，存放到set中
    QSet<int> setUsedInterval;
    for(QVector<int>::const_iterator itSellOrder = m_vSellOrderPrice.begin();itSellOrder != m_vSellOrderPrice.end();++itSellOrder)
    {
        int nInterval = calcInterval(nCurrAskPrice,*itSellOrder);
        for(QList<int>::const_iterator itInterval = m_lstOrderInterval.begin();itInterval != m_lstOrderInterval.end();++itInterval)
        {
            if(nInterval <= *itInterval)
            {
                setUsedInterval.insert(*itInterval);
                break;
            }
        }
    }

    // 取得未使用的最小距离值
    QList<int> lstRemainInterval = QSet::fromList(m_lstOrderInterval).subtract(setUsedInterval).toList();
    int nMinRemainInterval;
    if(lstRemainInterval.empty())
        return false;
    else
    {
        nMinRemainInterval = lstRemainInterval.front();
        for(QList<int>::const_iterator it = lstRemainInterval.begin();it != lstRemainInterval.end();++it)
        {
            if(*it < nMinRemainInterval)
                nMinRemainInterval = *it;
        }
    }

    // 计算未使用的最小距离值对应的卖出委托价格，并将该价格添加至卖出委托价格向量
    m_vSellOrderPrice.push_back(nCurrAskPrice + nMinRemainInterval * m_nPriceTick);
    _fOrderPrice = (nCurrAskPrice + nMinRemainInterval * m_nPriceTick) / (double)m_nPricePrecision;
    return true;

} // acquireSellOrderPrice
*/
bool CMMDispatcher::acquireSellOrderPrice(long &_nIndex, double &_fOrderPrice)
{
    // 取得最新卖一价
//    int nCurrAskPrice = (int)(CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getDepthMktData(m_strSecuCode).AskPrice[0] * m_nPricePrecision + 0.5);
    int nCurrAskPrice = (int)(CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(m_strSecuCode).AskPrice * m_nPricePrecision + 0.5);

    QMutexLocker locker(&m_lock);
    // 遍历卖出委托价格map，计算每个卖出委托价格与卖一价的距离，存放到set中
    QSet<int> setUsedInterval;
    for(QMap<long,double>::const_iterator itSellOrder = m_mpSellOrderPrice.begin();itSellOrder != m_mpSellOrderPrice.end();++itSellOrder)
    {
        int nInterval = calcInterval(nCurrAskPrice,(int)(itSellOrder.value() * m_nPricePrecision + 0.5));
        if(nInterval > m_lstOrderInterval.back())
            setUsedInterval.insert(m_lstOrderInterval.back());
        else
        {
            for(QList<int>::const_iterator itInterval = m_lstOrderInterval.begin();itInterval != m_lstOrderInterval.end();++itInterval)
            {
                if(nInterval <= *itInterval)
                {
                    setUsedInterval.insert(*itInterval);
                    break;
                }
            }
        }
    }

    // 取得未使用的最小距离值
    QList<int> lstRemainInterval = QSet<int>::fromList(m_lstOrderInterval).subtract(setUsedInterval).toList();
    int nMinRemainInterval;
    if(lstRemainInterval.empty())
        return false;
    else
    {
        nMinRemainInterval = lstRemainInterval.front();
        for(QList<int>::const_iterator it = lstRemainInterval.begin();it != lstRemainInterval.end();++it)
        {
            if(*it < nMinRemainInterval)
                nMinRemainInterval = *it;
        }
    }

    // 计算未使用的最小距离值对应的卖出委托价格，并将该价格添加至卖出委托价格map总
    _fOrderPrice = (nCurrAskPrice + nMinRemainInterval * m_nPriceTick) / (double)m_nPricePrecision;
    _nIndex = ++m_nIndex;
    m_mpSellOrderPrice.insert(_nIndex,_fOrderPrice);
    return true;
} // acquireSellOrderPrice

bool CMMDispatcher::acquireBuyCloseOrderPrice(long &_nIndex, double &_fClosePrice)
{
    // 取得最新买一价
//    int nCurrBidPrice = (int)(CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getDepthMktData(m_strSecuCode).BidPrice[0] * m_nPricePrecision + 0.5);
    int nCurrBidPrice = (int)(CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(m_strSecuCode).BidPrice * m_nPricePrecision + 0.5);
    QMutexLocker locker(&m_lock);

    // 遍历买入平仓委托价格map，计算每个买入平仓委托价格与买一价之间的距离，存放到set中
    QSet<int> setUsedInterval;
    for(QMap<long,double>::const_iterator itBuyClsOrder = m_mpBuyCloseOrderPrice.begin();itBuyClsOrder != m_mpBuyCloseOrderPrice.end();++itBuyClsOrder)
    {
        int nInterval = calcInterval(nCurrBidPrice,(int)(itBuyClsOrder.value() * m_nPricePrecision + 0.5));
        if(nInterval > m_lstOrderInterval.back())
            setUsedInterval.insert(m_lstOrderInterval.back());
        else
        {
            for(QList<int>::const_iterator itInterval = m_lstOrderInterval.begin();itInterval != m_lstOrderInterval.end();++itInterval)
            {
                if(nInterval <= *itInterval)
                {
                    setUsedInterval.insert(*itInterval);
                    break;
                }
            }
        }
    }

    // 取得未使用的最小距离值
    QList<int> lstRemainInterval = QSet<int>::fromList(m_lstOrderInterval).subtract(setUsedInterval).toList();
    int nMinRemainInterval;
    if(lstRemainInterval.empty())
        return false;
    else
    {
        nMinRemainInterval = lstRemainInterval.front();
        for(QList<int>::const_iterator it = lstRemainInterval.begin();it != lstRemainInterval.end();++it)
        {
            if(*it < nMinRemainInterval)
                nMinRemainInterval = *it;
        }
    }

    // 计算未使用的最小距离对应的买入平仓委托价格，并将该价格信息添加至买入平仓委托价格map
    _fClosePrice = (nCurrBidPrice - nMinRemainInterval * m_nPriceTick) / (double)m_nPricePrecision;
    _nIndex = ++m_nIndex;
    m_mpBuyCloseOrderPrice.insert(_nIndex,_fClosePrice);
    return true;

} // acquireBuyCloseOrderPrice

bool CMMDispatcher::exceedIntervalRange(int _longshort, long _index) const
{
    QMutexLocker locker(&m_lock);

    bool isExceed = false;
    if(0 == _longshort)
    {
        if(m_mpBuyOrderPrice.find(_index) != m_mpBuyOrderPrice.end())
        {
            int nCurrBidPrice = (int)(CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(m_strSecuCode).BidPrice * m_nPricePrecision + 0.5);
            int _interval = calcInterval(nCurrBidPrice,(int)(m_mpBuyOrderPrice[_index] * m_nPricePrecision + 0.5));
            if(_interval > m_lstOrderInterval.back())
                isExceed = true;
        }
    }
    else if(1 == _longshort)
    {
        if(m_mpSellOrderPrice.find(_index) != m_mpSellOrderPrice.end())
        {
            int nCurrAskPrice = (int)(CDerivativeMktDataBuffer::getMktDataBufferInstPtr()->getSimpleMktData(m_strSecuCode).AskPrice * m_nPricePrecision + 0.5);
            int _interval = calcInterval(nCurrAskPrice,(int)(m_mpSellOrderPrice[_index] * m_nPricePrecision + 0.5));
            if(_interval > m_lstOrderInterval.back())
                isExceed = true;
        }
    }
    return isExceed;
} // exceedIntervalRange

void CMMDispatcher::deleteBuyOrderPrice(long _nIndex)
{
    QMutexLocker locker(&m_lock);
//    int nBuyOrderPrice = (int)(_fprice * m_nPricePrecision + 0.5);
//    m_vBuyOrderPrice.removeAll(nBuyOrderPrice);

    if(m_mpBuyOrderPrice.contains(_nIndex))
        m_mpBuyOrderPrice.remove(_nIndex);

} // deleteBuyOrderPrice

void CMMDispatcher::deleteSellCloseOrderPrice(long _nIndex)
{
    QMutexLocker locker(&m_lock);
    if(m_mpSellCloseOrderPrice.contains(_nIndex))
        m_mpSellCloseOrderPrice.remove(_nIndex);

} // deleteSellCloseOrderPrice

void CMMDispatcher::deleteSellOrderPrice(long _nIndex)
{
    QMutexLocker locker(&m_lock);
//    int nSellOrderPrice = (int)(_nIndex * m_nPricePrecision + 0.5);
//    m_vSellOrderPrice.removeAll(nSellOrderPrice);

    if(m_mpSellOrderPrice.contains(_nIndex))
        m_mpSellOrderPrice.remove(_nIndex);

} // deleteSellOrderPrice

void CMMDispatcher::deleteBuyCloseOrderPrice(long _nIndex)
{
    QMutexLocker locker(&m_lock);
    if(m_mpBuyCloseOrderPrice.contains(_nIndex))
        m_mpBuyCloseOrderPrice.remove(_nIndex);

} // deleteBuyCloseOrderPrice

int CMMDispatcher::calcInterval(int _nBidAskPrice, int _nOrderPrice) const
{
    return abs(_nOrderPrice - _nBidAskPrice) / m_nPriceTick;
} // calcInterval

}
