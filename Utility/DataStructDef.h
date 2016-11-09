#ifndef DATASTRUCTDEF_H
#define DATASTRUCTDEF_H

#include <QString>
#include <QTime>
//#include "TradeModule/CTP/ThostFtdcTraderApi.h"
//#include "MarketData/API/CTP/ThostFtdcMdApi.h"

#include "CTP/inc/ThostFtdcTraderApi.h"
#include "CTP/inc/ThostFtdcMdApi.h"

namespace HESS
{

// 期货&期权深度行情
struct DerivDepthMktDataField
{
    QString TradingDay;            // 交易日期(YYYYMMDD)
    QString UpdateTime;            // 更新时间(HHMMSSmmm)
    QString InstrumentID;          // 合约代码
    QString ExchangeID;            // 交易所代码
    QString ExchangeInstrID;       // 合约在交易所代码
    double  Last;                  // 最新价
    double  PreSettlement;         // 上次结算价
    double  PreClose;              // 昨收盘价
    double  PreOpenInsterest;      // 昨持仓量
    double  Open;                  // 今开盘价
    double  High;                  // 最高价
    double  Low;                   // 最低价
    int     Volume;                // 成交量
    double  Turnover;              // 成交金额
    double  OpenInterest;          // 持仓量
    double  Close;                 // 今收盘价
    double  Settlement;            // 本次结算价
    double  UpperLimit;            // 涨停价
    double  LowerLimit;            // 跌停价
    double  PreDelta;              // 昨虚实度
    double  CurrDelta;             // 今虚实度
    double  AskPrice[5];           // 五档申卖价
    int     AskVol[5];             // 五档申卖量
    double  BidPrice[5];           // 五档申买价
    int     BidVol[5];             // 五档申买量
    double  AvgPrice;              // 当天均价
};

// simple期货&期权深度行情
struct SimpleDerivMktDataField
{
    QString InstrumentID;       // 合约代码
    double  Last;               // 最新价
    int     Volume;             // 成交量
    double  Turnover;           // 成交金额
    double  AskPrice;           // 卖一价
    int     AskVol;             // 卖一量
    double  BidPrice;           // 买一价
    int     BidVol;             // 买一量
};

// 拷贝衍生品深度行情数据
void CopyDerivativeDepthMktDataField(DerivDepthMktDataField& dest,const DerivDepthMktDataField& source);
void CopyDerivativeDepthMktDataField(DerivDepthMktDataField &dest, const CThostFtdcDepthMarketDataField &source);

// CTP trade param struct
struct TCTPTradeParam
{
    // broker id
    TThostFtdcBrokerIDType  chBrokerID;
    // user id
    TThostFtdcUserIDType    chUserID;
    // user password
    TThostFtdcPasswordType  chUserPwd;
    // front id
    TThostFtdcFrontIDType   nFrontID;
    // session id
    TThostFtdcSessionIDType nSessionID;
    // 行情前置地址（含端口）
    QString strMktFrontAddress;
    // 交易前置地址（含端口）
    QString strTradeFrontAddress;

    TCTPTradeParam(){memset(this,0,sizeof(*this));}
};

// 协整回归参数结构体
struct TCointegrationParam
{
    // 自变量代码
    QString strIndependentTicker;
    // 因变量代码
    QString strDependentTicker;
    // 截距项
    double fIntercept;
    // 斜率
    double fSlope;
    // 残差标准差
    double fErrorStd;
    // 相关系数
    double fRho;
    // 残差的偏度
    double fSkewness;
    // 残差的峰度
    double fKurtosis;

    // 开仓残差值上限
    double fOpenUpperLimit;
    // 开仓残差值下限
    double fOpenLowerLimit;

    // 止盈止损时对应的残差变动值
    double fErrorChgForExitTrade;

    TCointegrationParam(){memset(this,0,sizeof(*this));}
};

// 衍生品行情数据结构体
struct TDerivativeMktDataField
{
    QString ticker;          // 合约代码
    double  Last;            // 最新价
    double  AskPrice[5];     // 五档申卖价
    int     AskVol[5];       // 五档申卖量
    double  BidPrice[5];     // 五档申买价
    int     BidVol[5];       // 五档申买量
    QTime   tmUpdateTime;    // 更新时间

    TDerivativeMktDataField(){memset(this,0,sizeof(*this));}

    void update(const CThostFtdcDepthMarketDataField &_mktData);
};

// 生产模式
enum PRODUCTIONMODE
{
    PRODUCTION = 0,     // 生产模式
    SIMNOW = 1,         // simnow模拟模式
    BACKTEST = 2        // 回测模式
};

// 行情处理类型
enum MKTDATAHANDLERTYPE
{
    MARKETDATA_HANDLER = 0,
    ORDER_HANDLER = 1,
    TRADE_HANDLER = 2
};

// 证券行情K线类型
enum KDATATYPE
{
    KDATA_1M = 0,       // 1分钟K线
    KDATA_5M = 1,       // 5分钟K线
    KDATA_1D = 2        // 1日K线

};

// 证券行情数据(K线数据)
struct TSecuKMktData
{
    QDate   tmDate;     // 日期
    QTime   tmTime;     // 时间
    QString SecuCode;   // 证券代码
    double  Open;       // 开盘价
    double  High;       // 最高价
    double  Low;        // 最低价
    double  Close;      // 收盘价
    double  Vol;        // 成交量（股）
    double  Amount;     // 成交金额（元）
};

}

#endif // DATASTRUCTDEF_H
