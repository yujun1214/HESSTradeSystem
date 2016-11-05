#ifndef CCROSSARBITRAGE_H
#define CCROSSARBITRAGE_H

#include <QObject>
#include "TradeModule/CTP/ThostFtdcTraderApi.h"
#include "MarketData/CCTPMdSpi.h"
#include "Utility/DataStructDef.h"
#include <QString>
#include <QList>
#include <QMap>

namespace HESS
{

class CCrossArbitrage : public QObject,public CThostFtdcTraderSpi
{
    Q_OBJECT

public:
    // contructor
    CCrossArbitrage(CThostFtdcTraderApi* pUserTraderApi);

    // destrator
    ~CCrossArbitrage();

    // 实现的TraderSpi接口
    virtual void OnFrontConnected();

    virtual void OnFrontDisconnected(int nReason);

    virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);

    virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);

private:
    // 配置文件中的配置信息
    QMap<QString,QString> m_mpConfig;
    // CTP trade param
    TCTPTradeParam m_tCTPTradeParam;
    // initial request id in a trading session
    int m_nRequestID;
    // pointer to CThostFtdcTraderApi instance
    CThostFtdcTraderApi* m_pUserTraderApi;
    // pointer to CCTPMdSpi intance
    CCTPMdSpi* m_pCTPMdSpi;
    // pointer to CThostFtdcMDApi instance
    CThostFtdcMdApi* m_pUserMdApi;

    // 交易状态参数
    enum PAIRTRADINGSTATUS
    {
        // 空仓
        EMPTYHOLDING = 0,
        // 已发出建仓委托
        SENDEDPOSITIONORDER = 1,
        // 建仓完成
        POSITIONED = 2,
        // 建仓撤单
        CANCELINGPOSITION = 3,
        // 已发出平仓委托
        SENDEDLIQUIDATIONORDER=4,
        // 平仓撤单
        CANCELINGLIQUIDATION = 5
    };

    // 一笔套利交易的持仓量数据（包含建仓时的回归参数及交易状态参数）
    struct THoldingVolPair
    {
        // LegOne的实际持仓量
        int nLegOneHoldingVol;
        // LegOne的计划持仓量
        int nLegOnePlannedVol;

        // LegTwo的实际持仓量
        int nLegTwoHoldingVol;
        // LegTwo的计划持仓量
        int nLegTwoPlannedVol;

        // LegOne或LegTwo的单笔最大持仓手数
        int nMaxHoldingVol;

        // 建仓时该Pair的协整回归参数
        TCointegrationParam tRegressParam;

        // 状态参数
        PAIRTRADINGSTATUS eTradingStatus;

        THoldingVolPair(){memset(this,0,sizeof(*this));}
    };

    // 每个单一套利对的持仓数据
    // 一个套利对（如棕榈油和豆油）可含有多笔套利交易持仓，具体笔数可在配置文件中配置
    struct TSinglePairHoldingData
    {
        // ticker of leg one
        QString strLegOneTicker;
        // ticker of leg two
        QString strLegTwoTicker;
        // holding of this pair
        QList<THoldingVolPair*> HoldingVolList;

        TSinglePairHoldingData(){memset(this,0,sizeof(*this));}
    };

    // 套利策略的持仓数据
    QList<TSinglePairHoldingData*> m_HoldingDataList;

    // 每个单一套利对的最新协整回归参数
    // map<LegOneTicker&LegTwoTicker,TCointegrationParam>
    QMap<QString,TCointegrationParam> m_mpCointegrationParam;

    // 协整回归采用的行情tick数据个数
    int m_nTickNumForRegress;

    // 相关系数有效区间
    double m_fRhoLowerLimit;    // 相关系数下限
    double m_fRhoUpperLimit;    // 相关系数上限

    // 策略交易合约的最新行情数据索引
    // map<ticker,TDerivativeMktDataField>
    QMap<QString,TDerivativeMktDataField> m_mpLatestMktData;

    // 合约的连续行情数据结构体
    struct TMktSuccessiveData
    {
        // 最新价数组，默认设置为40000个元素
        double* arrPrice;
        // 当前需更新行情的位置
        int nPos;
        // 互斥锁

        // 更新行情数据
        void update(const CThostFtdcDepthMarketDataField& _mktData);

        TMktSuccessiveData(int _nDataLen = 40000)
        {
            arrPrice = new double[_nDataLen];
            nPos = 0;
        }
    };

    // 策略交易合约的连续行情数据索引
    // map<ticker,TMktSuccessiveData>
    QMap<QString,TMktSuccessiveData> m_mpSuccessiveData;

    // 策略交易合约对的行情数据指针结构体
    struct TPairMktDataPtr
    {
        // LegOne的连续行情数据结构体指针
        TMktSuccessiveData* pLegOneSuccessiveMktData;
        // LegTwo的连续行情数据结构体指针
        TMktSuccessiveData* pLegTwoSuccessiveMktData;

        TPairMktDataPtr()
        {
            pLegOneSuccessiveMktData = NULL;
            pLegTwoSuccessiveMktData = NULL;
        }
    };

    // 策略交易合约对的行情数据指针结构体索引
    // map<ticker1&ticker2,TPairMktDataPtr>
    QMap<QString,TPairMktDataPtr> m_mpPairMktDataPtr;

    // 交易请求编号与每笔套利交易的索引
    QMap<int,THoldingVolPair*> m_mpRequestIDMapping;

    // 交易报单引用与每笔套利交易的索引
    QMap<QString,THoldingVolPair*> m_mpOrderRefMapping;

private:
    // load strategy param
    bool LoadStrategyParam();
    // initialize strategy
    bool InitStrategyEnviron();
};

}

#endif // CCROSSARBITRAGE_H
