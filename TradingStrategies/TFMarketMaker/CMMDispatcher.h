#ifndef CMMDISPATCHER_H
#define CMMDISPATCHER_H

#include <QObject>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QVector>
#include <QString>
#include <QMap>

namespace HESS
{

// 做市商委托价格的调度类
class CMMDispatcher : public QObject
{
    Q_OBJECT

public:
    // contructor
    CMMDispatcher(QString _code):m_strSecuCode(_code),m_nIndex(0)
    {
        m_lstOrderInterval.clear();
        m_mpBuyOrderPrice.clear();
        m_mpSellCloseOrderPrice.clear();
        m_mpSellOrderPrice.clear();
        m_mpBuyCloseOrderPrice.clear();
    }
    // destructor
    ~CMMDispatcher(){}

    void setPriceTick(int _nTick)
    {
        QMutexLocker locker(&m_lock);
        m_nPriceTick = _nTick;
    }

    void setPricePrecision(int _nPrecision)
    {
        QMutexLocker locker(&m_lock);
        m_nPricePrecision = _nPrecision;
    }

    void addOrderInterval(int _nInterval);

    // 请求获取可用的买入开仓委托价格
//    bool acquireBuyOrderPrice(double& _fOrderPrice);
    bool acquireBuyOrderPrice(long& _nIndex,double& _fOrderPrice);
    // 请求获得可用的卖出平仓委托价格
//    bool acquireSellCloseOrderPrice(double& _fClosePrice);
    bool acquireSellCloseOrderPrice(long& _nIndex,double& _fClosePrice);

    // 请求获取可用的卖出开仓委托价格
//    bool acquireSellOrderPrice(double& _fOrderPrice);
    bool acquireSellOrderPrice(long& _nIndex,double& _fOrderPrice);
    // 请求获取可用的买入平仓委托价格
//    bool acquireBuyCloseOrderPrice(double& _fClosePrice);
    bool acquireBuyCloseOrderPrice(long& _nIndex,double& _fClosePrice);

    // 指定index对应的开仓委托价格是否超出了Intervals范围
    // _longshort = 0:为买入开仓,_longshort = 1:为卖出委托开仓
    // 超出返回return true, 否则return false
    bool exceedIntervalRange(int _longshort,long _index) const;

public slots:
    // delete the buy order price tied with the given index
    void deleteBuyOrderPrice(long _nIndex);

    // delete the sell close order price tied with the given index
    void deleteSellCloseOrderPrice(long _nIndex);

    // delete the sell order price tied with the given index
    void deleteSellOrderPrice(long _nIndex);

    // delete the buy close order price tied with the given index
    void deleteBuyCloseOrderPrice(long _nIndex);

private:
    // 做市调度对象的代码
    QString m_strSecuCode;
    // 存储委托价格距离买一卖一价多少个tick的list,按升序排列
    QList<int> m_lstOrderInterval;
    // 一个price tickh值
    int m_nPriceTick;
    // 行情的精度，用于真实行情与整数行情间的转换
    int m_nPricePrecision;
    // index
    long m_nIndex;
    // 买入开仓委托价格向量
//    QVector<int> m_vBuyOrderPrice;
    QMap<long,double> m_mpBuyOrderPrice;
    // 卖出平仓委托价格向量
//    QVector<int> m_vSellCloseOrderPrice;
    QMap<long,double> m_mpSellCloseOrderPrice;
    // 卖出开仓委托价格向量
//    QVector<int> m_vSellOrderPrice;
    QMap<long,double> m_mpSellOrderPrice;
    // 买入平仓委托价格向量
//    QVector<int> m_vBuyCloseOrderPrice;
    QMap<long,double> m_mpBuyCloseOrderPrice;
    // 互斥锁
    mutable QMutex m_lock;

    // 给定买一卖一价和委托价格，返回两者的距离（单位：tick）
    int calcInterval(int _nBidAskPrice,int _nOrderPrice) const;
};

} // HESS

#endif // CMMDISPATCHER_H
