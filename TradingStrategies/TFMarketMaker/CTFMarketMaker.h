#ifndef CTFMARKETMAKER_H
#define CTFMARKETMAKER_H

#include <QObject>
#include <QString>
#include "CMMDispatcher.h"
//#include "TradeModule/CTP/ThostFtdcTraderApi.h"
#include "CTP/inc/ThostFtdcTraderApi.h"
#include <QVector>
#include "Utility/CTimeController.h"
#include "Utility/DataStructDef.h"

namespace HESS
{

// long short type
enum LONGSHORTTYPE
{
    LONGTRADING = 0,
    SHORTTRADING = 1
};

// 国债期货做市商类
class CTFMarketMaker : public QObject
{
    Q_OBJECT

public:
    // constructor
    CTFMarketMaker(QString _strSecuCode,LONGSHORTTYPE _type,CMMDispatcher* _ptrMMDisptcher):
        m_strSecuCode(_strSecuCode),m_eTradingType(_type),m_ptrMMDispatcher(_ptrMMDisptcher)
    {
        m_eTradingStatus = EMPTYHOLDING;
    }

    // destructor
    ~CTFMarketMaker(){}

//    // long short type
//    enum LONGSHORTTYPE
//    {
//        LONGTRADING = 0,
//        SHORTTRADING = 1
//    };

    void setPriceTick(int _nTick)
    {
        m_nPriceTick = _nTick;
    }

    void setPricePrecision(int _nPrecision)
    {
        m_nPricePrecision = _nPrecision;
    }

    void setStoplossTicks(int _nTicks)
    {
        m_nStoplossTicks = _nTicks;
    }

//    void setMMDispatcher(CMMDispatcher* _ptrMMDispatcher)
//    {
//        m_ptrMMDispatcher = _ptrMMDispatcher;
//    }

    void setUserTradeApi(CThostFtdcTraderApi* _ptrUserTradeApi)
    {
        m_ptrUserTradeApi = _ptrUserTradeApi;
    }

    void setTimeController(CTimeController* _ptrTimeController)
    {
        m_ptrTimeController = _ptrTimeController;
    }

    void setBrokerID(TThostFtdcBrokerIDType _szBrokerID)
    {
        if(strlen(_szBrokerID) <= 0)
            return;
        strcpy(m_szBrokerID,_szBrokerID);
    }

    void setBrokerID(QString strBrokerID)
    {
        if(strBrokerID.length() <= 0)
            return;
        strcpy(m_szBrokerID,strBrokerID.toStdString().c_str());
    }

    void setInvestorID(TThostFtdcInvestorIDType _szInvestorID)
    {
        if(strlen(_szInvestorID) <= 0)
            return;
        strcpy(m_szInvestorID,_szInvestorID);
    }

    void setInvestorID(QString strInvestorID)
    {
        if(strInvestorID.length() <= 0)
            return;
        strcpy(m_szInvestorID,strInvestorID.toStdString().c_str());
    }

    LONGSHORTTYPE TradingType() const
    {
        return m_eTradingType;
    }

private:
    // secu code
    QString m_strSecuCode;
    // long short type
    LONGSHORTTYPE m_eTradingType;
    // trading status enum
    enum MMTRADINGSTATUS
    {
        // 空仓
        EMPTYHOLDING = 0,
        // 已发出建仓委托
        SENDEDPOSITIONORDER = 1,
        // 已发出建仓委托撤单请求
        SENDEDCANCELPOSITIONORDER = 2,
        // 建仓完成
        POSITIONED = 3,
        // 已发出平仓委托
        SENDEDLIQUIDATIONORDER = 4,
        // 已发出平仓委托撤单请求，平仓委托撤单一般是为了撤单后进行止损
        SENDEDCANCELLIQUIDATIONORDER = 5,
        // 已发出止损委托
        SENDEDSTOPLOSSORDER = 6
    };
    // trading status
    MMTRADINGSTATUS m_eTradingStatus;
    // 一个price tick值
    int m_nPriceTick;
    // 行情的精度，用于真实行情和整数行情间的转换
    int m_nPricePrecision;
    // holding cost
    double m_fHoldingCost;
    // stop loss tick number
    int m_nStoplossTicks;
    // pointer to CMMDispatcher class
    CMMDispatcher* m_ptrMMDispatcher;
    // pointer to user trade api
    CThostFtdcTraderApi* m_ptrUserTradeApi;
    // time controller
    CTimeController* m_ptrTimeController;
    // 交易报单引用
    mutable int m_nTradeOrderRef;
    // 撤单报单引用
    int m_nCancelOrderRef;
    // front id
    TThostFtdcFrontIDType m_nFrontID;
    // session id
    TThostFtdcSessionIDType m_nSessionID;
    // broker id
    TThostFtdcBrokerIDType m_szBrokerID;
    // investor id
    TThostFtdcInvestorIDType m_szInvestorID;

    // the index acquired from Dispatcher for open/close order price
    long m_nOrderIndex;

private:
    // 开仓委托
    void submitOpenPositionOrder();
    // 平仓委托
    void submitClosePositionOrder();
    // 撤单委托
    void submitCancelOrder(int _nOrderRef);
    // 处理止损
    // 当盘口价格与委托价格距离超过m_nStoplossTick个tick时，触发止损
    void handleStoploss();
    // 发送限价委托
    void sendLimitOrder(TThostFtdcDirectionType _chDirection,
                        TThostFtdcOffsetFlagType _chOpenClose,int _nVol,double _fPrice) const;
    // 发送市价委托(在对手价基础上加上或减去10个ticks)
    void sendMktPriceOrder(TThostFtdcDirectionType _chDirection,
                           TThostFtdcOffsetFlagType _chOpenClose,int _nvol) const;


// 信号、槽，对行情、成交回报推送进行响应
public slots:
    // 处理行情更新（如止盈、止损），需判断代码是否一致
    void handleUpdateMktData(QString _strSecuCode);
    // 处理RtnOrder
    // _ptrOrder是由tradespi在堆中创建，本函数负责delete它
    void handleRtnOrder(CThostFtdcOrderField* _ptrOrder);
    // 处理RtnTrade
    // _ptrTrade是由tradespi在堆中创建，本函数负责delete它
    void handleRtnTrade(CThostFtdcTradeField* _ptrTrade);
    // 处理OnErrRtnOrderAction
    // _ptrOrderAction是由tradespi在堆中创建，本函数负责delete它
    void handleErrRtnOrderAction(CThostFtdcOrderActionField* _ptrOrderAction);

    // 设置frontid和sessionid
    void setFrontSessionID(TThostFtdcFrontIDType _nFrontID,TThostFtdcSessionIDType _nSessionID);

signals:
    // 开仓完成
    void sigPositioned(long _nIndex);
    // 开仓委托被拒绝
    void sigOpenOrderDenied(long _nIndex);
    // 平仓完成
    void sigLiquidationed(long _nIndex);
    // 平仓委托被拒绝
    void sigCloseOrderDenied(long _nIndex);
    // 开仓委托撤单完成
    void sigOpenOrderCanceled(long _nIndex);
    // 平仓委托撤单完成
    void sigCloseOrderCanceled(long _nIndex);

    // 输出日志到控制台
    void sigToConsole(QString _loginfo);

};

}

#endif // CTFMARKETMAKER_H
