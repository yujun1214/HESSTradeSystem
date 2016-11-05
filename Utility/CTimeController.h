#ifndef CTIMECONTROLLER_H
#define CTIMECONTROLLER_H

#include <QObject>
#include <QTime>
#include <QReadWriteLock>

namespace HESS
{

// 返回各个交易所当前时间对应的交易状态的基类
class CTimeController : public QObject
{
    Q_OBJECT
public:
    CTimeController():m_CurrTime(QTime(6,0,0,0)){}
    ~CTimeController(){}

    enum TimeStatus
    {
        // 未开盘
        PREOPENING = 0,
        // 开盘缓冲期
        OPENINGCUSHION = 1,
        // 正常交易
        NORMALTRADING = 2,
        // 小节休盘
        INTERSETIONCLOSING = 3,
        // 午间休盘
        NOONCLOSING = 4,
        // 收盘缓冲期
        PRECLOSINGCUSHION = 5,
        // 收盘平仓期
        PRECLOSINGLIQUIDATION = 6,
        // 已收盘
        CLOSED = 7
    };

    virtual TimeStatus CurrTimeStatus() const = 0;

public slots:
    // 设置当前时间
    void onSetCurrTime(QTime _currtime);

protected:
    // 当前行情时间(for backtest)
    QTime m_CurrTime;
    // read write lock
    mutable QReadWriteLock m_lock;

};

// 中金所交易时间状态类
class CCFFEXTimeController : public CTimeController
{
public:
    CCFFEXTimeController(){}
    ~CCFFEXTimeController(){}

    virtual TimeStatus CurrTimeStatus() const;
};

// 夜盘交易的时间状态类
class CNightTradingTimeController : public CTimeController
{
public:
    CNightTradingTimeController(){}
    ~CNightTradingTimeController(){}

    virtual TimeStatus CurrTimeStatus() const;
};

// 用于回测的时间状态类
class CBackTestTimeController : public CTimeController
{
public:
    CBackTestTimeController(){}
    ~CBackTestTimeController(){}

    virtual TimeStatus CurrTimeStatus() const;
};

} // namespace HESS

#endif // CTIMECONTROLLER_H
