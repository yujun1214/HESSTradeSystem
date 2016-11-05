#ifndef CCROSSARBITRAGE_H
#define CCROSSARBITRAGE_H

#include <QObject>
#include "TradeModule/CTP/ThostFtdcTraderApi.h"
#include "MarketData/CCTPMdSpi.h"
#include "Utility/DataStructDef.h"
#include <QString>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QTime>

namespace HESS
{

#define ELAPSEMSECS 3000     // 时间流逝（单位：毫秒）

class CCrossArbitrage : public QObject
{
    Q_OBJECT

public:
    // contructor
    // _nPairNum:可开仓的套利对数量
    // _nMaxHoldingNum:每个套利对中legone或legtwo的最大持仓量
    CCrossArbitrage(CThostFtdcTraderApi* pUserTraderApi,QString _LegOneTicker,
                    QString _LegTwoTicker,int _nPairNum,int _nMaxHoldingNum);

    // destrator
    ~CCrossArbitrage();

    // setter
    void setLegOneMultiple(int _multiple)
    {
        m_nLegOneMultiple = _multiple;
    }

    void setLegOnePriceTick(double _tick)
    {
        m_fLegOnePriceTick = _tick;
    }

    void setLegTwoMultiple(int _multiple)
    {
        m_nLegTwoMultiple = _multiple;
    }

    void setLegTwoPriceTick(double _tick)
    {
        m_fLegTwoPriceTick = _tick;
    }

    void setRhoLowerLimit(double _limit)
    {
        m_fRhoLowerLimit = _limit;
    }

    void setRhoUpperLimit(double _limit)
    {
        m_fRhoUpperLimit = _limit;
    }

    void setBrokerID(char _szBrokerID[])
    {
        strcpy(m_szBrokerID,_szBrokerID);
    }

    void setInvestorID(char _szInvestorID[])
    {
        strcpy(m_szInvestorID,_szInvestorID);
    }

    static void setProductionMode(PRODUCTIONMODE _mode)
    {
        m_Mode = _mode;
    }

private:
    // ticker of leg one, independent variable
    QString m_strLegOneTicker;
    // multiple of leg one
    int m_nLegOneMultiple;
    // price tick of leg one
    double m_fLegOnePriceTick;
    // ticker of leg two, dependent variable
    QString m_strLegTwoTicker;
    // multiple of leg two
    int m_nLegTwoMultiple;
    // price tick of leg two
    double m_fLegTwoPriceTick;

    static int m_nRequestID;
    static int m_nOrderRef;
    QMutex m_OrderIDMutex;
    void getValidOrderID(int& _requestid, int& _orderref);

    // 生产模式
    static PRODUCTIONMODE m_Mode;

    // 在simnow和reply模式下的委托函数，直接调用HandleRtnOrder和HandleRtnTrade，并保存交易信息至文件
    void sendOrderOnSimulation(int _nOrderRef,QString _strCode,TThostFtdcDirectionType _chDirection,
                               TThostFtdcOffsetFlagType _chOpenClose,int _nVol,double _fPrice) const;

    // broker id
    TThostFtdcBrokerIDType m_szBrokerID;
    // investor id
    TThostFtdcInvestorIDType m_szInvestorID;

    // pointer to CThostFtdcTraderApi instance
    CThostFtdcTraderApi* m_pUserTraderApi;
    // 不需要交易spi、行情spi，直接与外部的交易spi、行情spi通过signal、slot交互

    // 交易状态参数
    enum PAIRTRADINGSTATUS
    {
        // 空仓
        EMPTYHOLDING = 0,
        // 已发出建仓委托
        SENDEDPOSITIONORDER = 1,
        // LegOne单腿建仓,LegTwo未建仓
        LEGONESINGLEPOSITIONED = 2,
        // LegTwo单腿建仓,LegOne未建仓
        LEGTWOSINGLEPOSITIONED = 3,
        // 建仓完成
        POSITIONED = 4,
        // 已发出LegOne单腿平仓委托
        SENDEDLEGONELIQUIDATIONORDER = 5,
        // 已发出LegTwo单腿平仓委托
        SENDEDLEGTWOLIQUIDATIONORDER = 6,
        // 已发出平仓委托
        SENDEDLIQUIDATIONORDER=7,
        // LegOne单腿平仓，LegTwo未平仓
        LEGONELIQUIDATIONED = 8,
        // LegTwo单腿平仓，LegOne未平仓
        LEGTWOLIQUIDATIONED = 9
    };

    // 套利类型
    enum ARBITRAGETYPE
    {
        // 卖出spread,spread = price of leg two - price of leg one
        SHORTSPREAD = -1,
        // 买入spread
        LONGSPREAD = 1
    };

    // 一笔套利交易的持仓量数据（包含建仓时的回归参数及交易状态参数）
    struct THoldingVolPair
    {
        // 套利类型
        ARBITRAGETYPE eArbType;

        // LegOne的实际持仓量
        int nLegOneHoldingVol;
        // LegOne的实际持仓成本 = sum(成交手数i * 成交价格i)
        double fLegOneHoldingCost;
        // LegOne的计划持仓量
        int nLegOnePlannedVol;
        // LegOne的交易请求编号
//        int nLegOneRequestID;
        // LegOne的报单引用
//        QString szLegOneOrderRef;

        // LegTwo的实际持仓量
        int nLegTwoHoldingVol;
        // LegTwo的实际持仓成本 = sum(成交手数i * 成交价格i)
        double fLegTwoHoldingCost;
        // LegTwo的计划持仓量
        int nLegTwoPlannedVol;
        // LegTwo的交易请求编号
//        int nLegTwoRequestID;
        // LegTwo的报单引用
//        QString szLegTwoOrderRef;

        // LegOne或LegTwo的单笔最大持仓手数
        int nMaxHoldingVol;

        // 建仓时该Pair的协整回归参数
        TCointegrationParam tRegressParam;
        // 本笔持仓开仓时的残差 = unit cost of legtwo - （intercept + slope*unit cost of legone）
        double fOpenError;

        // 交易状态参数
        PAIRTRADINGSTATUS eTradingStatus;

        // 最新交易状态设定时间
        QTime tmStatusSettingTime;

        THoldingVolPair()
        {
            memset(this,0,sizeof(*this));
            eArbType = 0;
            nLegOneHoldingVol = 0;
            fLegOneHoldingCost = 0.0;
            nLegOnePlannedVol = 0;
//            nLegOneRequestID = 0;
            nLegTwoHoldingVol = 0;
            fLegTwoHoldingCost = 0.0;
            nLegTwoPlannedVol = 0;
//            nLegTwoRequestID = 0;
            nMaxHoldingVol = 0;
            eTradingStatus = EMPTYHOLDING;
        }
    };  // THoldingVolPair

    // 套利策略的持仓数据list
    // 一个套利对（如棕榈油和豆油）可含有多笔套利交易持仓，具体笔数可在配置文件中配置
    QList<THoldingVolPair*> m_HoldingDataList;

    // 套利策略的最新协整回归参数
    TCointegrationParam m_tCointegrationParam;
    // 协整回归参数是否已更新，如已更新则可进行建仓
    bool bIfUpdatedCointegrationParam;
    // 协整回归参数的互斥锁
//    QMutex m_RegressionParamMutex;

    // 相关系数有效区间
    double m_fRhoLowerLimit;    // 相关系数下限
    double m_fRhoUpperLimit;    // 相关系数上限

    // 本套利对的最新行情
    TDerivativeMktDataField m_tLegOneMktData;
    TDerivativeMktDataField m_tLegTwoMktData;
    // 行情数据的互斥锁
//    QMutex m_MktMutex;

    // 交易请求编号与每笔套利交易的索引
//    QMap<int,THoldingVolPair*> m_mpRequestIDMapping;
    // 交易报单引用与每笔套利交易的索引
    QMap<int,THoldingVolPair*> m_mpOrderRefMapping;
    // 索引的互斥锁
//    QMutex m_MappingMutex;

private:
    // 进行套利交易的主函数，采用市价单进行委托
    // 流程如下：
    // 协整回归参数是否已更新，若未更新return
    // 遍历持仓数据list，①如果是“空仓”状态，计算是否触发开仓，若是则开仓。一旦本次行情推送开仓了一对套利对，那么在本次行情推送中
    // 不再开仓第二个套利对。②如果是“建仓完成”状态，计算是否触发止盈、止损，若是则平仓。③如果是其他状态，也需要根据情况进行撤单或补单操作
    void doArbitrage();

    // 开仓委托
    // _arbtype = 套利类型（卖出spread，买入spread）
    // _nLegOneVol = legone的委托数量
    // _nLegTwoVol = legtwo的委托数量
    void SubmitOpenPositionOrder(ARBITRAGETYPE _arbtype, THoldingVolPair* _pHolding,int _nLegOneVol, int _nLegTwoVol) const;

    // 平仓委托（包括止盈、止损、收盘前平仓）
    // _arbtype = 套利类型（卖出spread，买入spread）
    // _nLegOneVol = legone的委托数量
    // _nLegTwoVol = legtwo的委托数量
    void SubmitClosePositionOrder(ARBITRAGETYPE _arbtype,THoldingVolPair* _pHolding,int _nLegOneVol,int _nLegTwoVol) const;

    // 发出撤单委托
    void SubmitCancelOrder();

    // 发送委托
    void sendOrder(QString _strCode, TThostFtdcDirectionType _chDirection,
                   TThostFtdcOffsetFlagType _chOpenClose, int _nVol,
                   double _fPrice,THoldingVolPair* _pHolding);

    // 判断是否当前是否处于商品期货正常交易时间
    bool isAtTradingTime() const;


// 信号、槽，对行情、成交回报推送进行响应
public slots:
    // 更新行情，需判断代码是否一致
    void updateMktData(TDerivativeMktDataField _mktData);
    // 更新协整回归参数，需判断代码是否一致
    void updateCointegrationParam(TCointegrationParam _regressParam);
    // 处理RtnOrder
    // _pOrder是由tradespi在堆中创建的，本函数负责delete它
    void handleRtnOrder(int _nOrderRef,CThostFtdcOrderField* _pOrder);
    // 处理RtnTrade
    // _pTrade是由tradespi在堆中创建的，本函数负责delete它
    void handleRtnTrade(int _nOrderRef,CThostFtdcTradeField* _pTrade);

signals:
    // 行情更新完毕，在当前模式为reply时发送
    void sigUpdatedMktData();
};  // CCrossArbitrage

}   // HESS

#endif // CCROSSARBITRAGE_H
