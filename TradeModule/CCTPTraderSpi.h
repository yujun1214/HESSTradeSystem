#ifndef CCTPTRADERSPI_H
#define CCTPTRADERSPI_H

#include <QObject>
#include <QString>
#include <QMap>
#include <QMutex>
#include <QDate>
#include <QFile>
#include <QVector>
//#include "TradeModule/CTP/ThostFtdcTraderApi.h"
#include "CTP/inc/ThostFtdcTraderApi.h"
#include "Utility/DataStructDef.h"
#include "Utility/CLogSys.h"

namespace HESS
{

#define EPSILON 0.00001
#define MULTIPLIER 10000

class CCTPTraderSpi : public QObject,public CThostFtdcTraderSpi
{
    Q_OBJECT
public:
    // constructor
    CCTPTraderSpi(CThostFtdcTraderApi* _ptrTraderApi,PRODUCTIONMODE _mode) : m_ptrTraderApi(_ptrTraderApi),m_mpOrderPool(nullptr),m_fPnL(0.0),m_ptrPnLFile(nullptr)
    {
        m_mpOrderPool = new QMap<int,CThostFtdcInputOrderField*>();
        // 如果是回测模式，那么打开记录P&L的文件句柄
        if(BACKTEST == _mode)
        {
            m_ptrPnLFile = new QFile("PnL.txt");
        }
    }
    // destructor
    ~CCTPTraderSpi()
    {
        if(nullptr != m_mpOrderPool)
        {
            for(QMap<int,CThostFtdcInputOrderField*>::iterator it = m_mpOrderPool->begin();it != m_mpOrderPool->end();++it)
            {
                delete it.value();
            }
            m_mpOrderPool->clear();
            delete m_mpOrderPool;
            m_mpOrderPool = nullptr;
        }

        if(m_ptrPnLFile != nullptr)
        {
            if(m_ptrPnLFile->isOpen())
                m_ptrPnLFile->close();
            delete m_ptrPnLFile;
        }

    }
    // setter
    void setBrokerID(QString _strBrokerID)
    {
        if(_strBrokerID.length() == 0)
            return;
        strcpy(m_szBrokerID,_strBrokerID.toStdString().c_str());
    }

    void setInvestorID(QString _strInvestorID)
    {
        if(_strInvestorID.length() == 0)
            return;
        strcpy(m_szInvestorID,_strInvestorID.toStdString().c_str());
    }

    void setPassword(QString _strPassword)
    {
        if(_strPassword.length() == 0)
            return;
        strcpy(m_szPassword,_strPassword.toStdString().c_str());
    }

    void setBackTestInterval(QDate _begdate,QDate _enddate)
    {
        if(_begdate > _enddate)
        {
            CLogSys::getLogSysInstance()->toConsole("[Error] BackTest begin date should not greater than end date.");
        }
        m_tmBegDate = _begdate;
        m_tmEndDate = _enddate;
        loadTradingDays();
    }

    // getter
    QDate TradingDay() const
    {
        return m_tmTradingDay;
    }

    // overload functions of traderspi
    virtual void OnFrontConnected();
    virtual void OnFrontDisconnected();
    virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);
    virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);
    virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    virtual void OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction, CThostFtdcRspInfoField *pRspInfo);
    virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
    virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

signals:
    void sigOnRtnOrder(CThostFtdcOrderField* ptrOrder);
    void sigOnRtnTrade(CThostFtdcTradeField* ptrTrade);
    void sigOnRspOrderAction(CThostFtdcInputOrderActionField* ptrInputOrderAction);
    void sigOnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction);
    void sigOnUserLogin(TThostFtdcFrontIDType _nFrontID,TThostFtdcSessionIDType _nSessionID);

    // 发信号给CLogSys，保存P&L数值
    void sigOnSavePnL(QFile *_ptrfile,QString _data);
    // 发信号给CCTPMdSpi,每天初始化播放的行情数据
    void sigSwitchToNextTradingDay(QDate tmTradingDay);
    // 回测结束行行好
    void sigBackTestFinished();

    // slots for backtest
public slots:
    // slot of RegisterFront
    void OnRegisterFront();

    // slot of user login
    void OnUserLogin(int nRequestID);

    // slot of user logout
    void OnUserLogout(int nRequestID);

    // slot of order insert
    // ptrInputOrder为在堆中创建，注意对其的内存释放
    void OnOrderInsert(CThostFtdcInputOrderField *ptrInputOrder,int nRequestID);

    // slot of order action，只处理撤单情况
    // ptrOrderAction为在堆中创建，注意对其的内存释放
    void OnOrderAction(CThostFtdcInputOrderActionField *ptrOrderAction,int nRequestID);

    // slot of settlement info confirm
    // ptrSttlInfoConfirm为在堆中创建，注意对其的内存释放
    void OnSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *ptrSttlInfoConfirm,int nRequestID);

    // 处理订单撮合，该接口响应行情推送，对订单进行撮合
    void OnMatchOder();

    // 初始化信息
    void InitTradeInfo(QDate _tmTradingDay);

    // 保存P&L值
    void OnSavePnL();

private:
    // pointer to trader api
    CThostFtdcTraderApi* m_ptrTraderApi;
    // front id
    TThostFtdcFrontIDType m_nFrontID;
    // session id
    TThostFtdcSessionIDType m_nSessionID;
    // broker id
    TThostFtdcBrokerIDType m_szBrokerID;
    // investor id
    TThostFtdcInvestorIDType m_szInvestorID;
    // user pwd
    TThostFtdcPasswordType m_szPassword;

    // 订单池(for backtest)
    QMap<int,CThostFtdcInputOrderField*> *m_mpOrderPool;
    // mutex to order pool
    QMutex m_lock;
    // calc P&L(for backtest)
    void OnCalcPNL(){}
    // begin and end date of backtest
    QDate m_tmBegDate;
    QDate m_tmEndDate;
    // 当前交易日期(for backtest)
    QDate m_tmTradingDay;
    // 回测区间的交易日列表（for backtest）
    QVector<QDate> m_vTradingDays;
    // 指向当前交易日期的迭代器（for backtest）
    QVector<QDate>::const_iterator m_itTradingDay;

    // 加载开始结束日期间的交易日列表
    // 文件名为"TradingDay.csv"
    void loadTradingDays();
    // 订单即时撮合，用于报单提交时的撮合(for backtest)
    // 撮合规则：
    // 买入时：如果委托价大于等于卖一价，则成交，成交价为委托价、卖一价、最新价三者取中间；不能成交，则等待更优的行情才能成交
    // 卖出时：如果委托价小于等于买一价，则成交，成交价为委托价、买一价、最新价三者取中间；不能成交，则等待更优的行情才能成交
    bool MatchOrder_OnInsertOrder(CThostFtdcInputOrderField *ptrOrder);
    // 订单遍历撮合，用于对提交时未成交的保单进行撮合(for backtest)
    // 撮合规则：
    // 买入时：如果委托价大于卖一价，则成交，成交价为委托价
    // 卖出时：如果委托价小于买一价，则成交，成交价为委托价
    bool MatchOrder_OnTraverseOrder(CThostFtdcInputOrderField *ptrOrder);

    // for P&L calculation(for backtest)
    // P&L
    double m_fPnL;
    // calculate p&L,目前只支持当天平仓、不持仓过夜的策略P&L计算
    void CalcPnL(const CThostFtdcTradeField *ptrTrade);
    // pointer to file that save P&L data
    QFile *m_ptrPnLFile;

}; // CCTPTraderSpi
} // namespace HESS

#endif // CCTPTRADERSPI_H
