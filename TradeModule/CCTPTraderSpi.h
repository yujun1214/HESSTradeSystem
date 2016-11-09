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

class CCTPTraderSpi : public QObject,public CThostFtdcTraderSpi
{
    Q_OBJECT
public:
    // constructor
    CCTPTraderSpi(CThostFtdcTraderApi* _ptrTraderApi,PRODUCTIONMODE _mode) : m_ptrTraderApi(_ptrTraderApi)
    {

    }
    // destructor
    ~CCTPTraderSpi()
    {

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

}; // CCTPTraderSpi
} // namespace HESS

#endif // CCTPTRADERSPI_H
