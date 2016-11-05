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
        // ����ǻز�ģʽ����ô�򿪼�¼P&L���ļ����
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

    // ���źŸ�CLogSys������P&L��ֵ
    void sigOnSavePnL(QFile *_ptrfile,QString _data);
    // ���źŸ�CCTPMdSpi,ÿ���ʼ�����ŵ���������
    void sigSwitchToNextTradingDay(QDate tmTradingDay);
    // �ز�������к�
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
    // ptrInputOrderΪ�ڶ��д�����ע�������ڴ��ͷ�
    void OnOrderInsert(CThostFtdcInputOrderField *ptrInputOrder,int nRequestID);

    // slot of order action��ֻ���������
    // ptrOrderActionΪ�ڶ��д�����ע�������ڴ��ͷ�
    void OnOrderAction(CThostFtdcInputOrderActionField *ptrOrderAction,int nRequestID);

    // slot of settlement info confirm
    // ptrSttlInfoConfirmΪ�ڶ��д�����ע�������ڴ��ͷ�
    void OnSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *ptrSttlInfoConfirm,int nRequestID);

    // ��������ϣ��ýӿ���Ӧ�������ͣ��Զ������д��
    void OnMatchOder();

    // ��ʼ����Ϣ
    void InitTradeInfo(QDate _tmTradingDay);

    // ����P&Lֵ
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

    // ������(for backtest)
    QMap<int,CThostFtdcInputOrderField*> *m_mpOrderPool;
    // mutex to order pool
    QMutex m_lock;
    // calc P&L(for backtest)
    void OnCalcPNL(){}
    // begin and end date of backtest
    QDate m_tmBegDate;
    QDate m_tmEndDate;
    // ��ǰ��������(for backtest)
    QDate m_tmTradingDay;
    // �ز�����Ľ������б�for backtest��
    QVector<QDate> m_vTradingDays;
    // ָ��ǰ�������ڵĵ�������for backtest��
    QVector<QDate>::const_iterator m_itTradingDay;

    // ���ؿ�ʼ�������ڼ�Ľ������б�
    // �ļ���Ϊ"TradingDay.csv"
    void loadTradingDays();
    // ������ʱ��ϣ����ڱ����ύʱ�Ĵ��(for backtest)
    // ��Ϲ���
    // ����ʱ�����ί�м۴��ڵ�����һ�ۣ���ɽ����ɽ���Ϊί�мۡ���һ�ۡ����¼�����ȡ�м䣻���ܳɽ�����ȴ����ŵ�������ܳɽ�
    // ����ʱ�����ί�м�С�ڵ�����һ�ۣ���ɽ����ɽ���Ϊί�мۡ���һ�ۡ����¼�����ȡ�м䣻���ܳɽ�����ȴ����ŵ�������ܳɽ�
    bool MatchOrder_OnInsertOrder(CThostFtdcInputOrderField *ptrOrder);
    // ����������ϣ����ڶ��ύʱδ�ɽ��ı������д��(for backtest)
    // ��Ϲ���
    // ����ʱ�����ί�м۴�����һ�ۣ���ɽ����ɽ���Ϊί�м�
    // ����ʱ�����ί�м�С����һ�ۣ���ɽ����ɽ���Ϊί�м�
    bool MatchOrder_OnTraverseOrder(CThostFtdcInputOrderField *ptrOrder);

    // for P&L calculation(for backtest)
    // P&L
    double m_fPnL;
    // calculate p&L,Ŀǰֻ֧�ֵ���ƽ�֡����ֲֹ�ҹ�Ĳ���P&L����
    void CalcPnL(const CThostFtdcTradeField *ptrTrade);
    // pointer to file that save P&L data
    QFile *m_ptrPnLFile;

}; // CCTPTraderSpi
} // namespace HESS

#endif // CCTPTRADERSPI_H
