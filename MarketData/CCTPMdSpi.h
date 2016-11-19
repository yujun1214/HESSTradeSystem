#ifndef CCTPMDSPI_H
#define CCTPMDSPI_H

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QVector>
#include <QDate>
//#include "MarketData/API/CTP/ThostFtdcMdApi.h"
#include "CTP/inc/ThostFtdcMdApi.h"

namespace HESS
{

class CCTPMdSpi : public QObject,public CThostFtdcMdSpi
{
    Q_OBJECT
public:
    // ���캯����pUserApi = ָ��CThostFtdcMdApi���ָ��
    CCTPMdSpi(CThostFtdcMdApi* pUserApi);

    // ��������
    ~CCTPMdSpi();

    // set broker id
    void setBrokerID(TThostFtdcBrokerIDType chBrokerID);
    void setBrokerID(QString strBrokerID);

    // set user id
    void setUserID(TThostFtdcUserIDType chUserID);
    void setUserID(QString strUserID);

    // set user password
    void setPassword(TThostFtdcPasswordType chUserPwd);
    void setPassword(QString struserPwd);

    // ��Ӷ�������ĺ�Լ����
    void addSecuCode(QString _secucode);

    // set tick data dir(for backtest)
    void setTickDataDir(QString _dir);

    // �ͻ������ӽ��׺�̨�ɹ�������OnFrontConnected�ص�����
    // ���ڴ˻ص������н��е�¼����
    virtual void OnFrontConnected();

    // ���ͻ����뽻�׺�̨ͨ�����ӶϿ�ʱ���÷��������á���������������API���Զ��������ͻ��˿ɲ�������
    ///@param  nReason   ����ԭ��
    ///         0X1001   �����ʧ��
    ///         0x1002   ����дʧ��
    ///         0x2001   ����������ʱ
    ///         0x2002   ��������ʧ��
    ///         0x2003   �յ�������
    virtual void OnFrontDisconnected(int nReason);

    // ������ʱ���档����ʱ��δ�յ�����ʱ���÷������á�
    // @param nTimeLaps = �����ϴν��ձ��ĵ�ʱ��
    virtual void OnHeartBeatWarning(int nTimeLapse);

    // ��¼������Ӧ
    // ���ͻ��˷�����¼����֮�󣬽����й�ϵͳ������Ӧʱ���÷����ᱻ���ã�֪ͨ�ͻ��˵�¼�Ƿ�ɹ�
    virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    // �ǳ�������Ӧ
    // ���ͻ��˷����˳�����֮�󣬽����й�ϵͳ������Ӧʱ���÷����ᱻ���ã�֪ͨ�ͻ����˳��Ƿ�ɹ�
    virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    // ����Ӧ��
    virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    // ��������Ӧ��
    virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    // ȡ����������Ӧ��
    virtual void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    // ����ѯ��Ӧ��
    virtual void OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    // ȡ������ѯ��Ӧ��
    virtual void OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    // �����������
    virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

    // ѯ������
    virtual void OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp);

signals:
    // the signal of OnFrontConnected
    void sigOnFrontConnected();

    // the signal of OnFrontDisconnected
    void sigOnFrontDisconnected(QString strReason);

    // the signal of OnHeartBeatWarning
    void sigOnHeartBeatWarning(int nTimeLapse);

    // the signal of OnRspUserLogin
    void sigOnRspUserLogin(bool bIsError,CThostFtdcRspUserLoginField RspUserLogin, CThostFtdcRspInfoField RspInfo, int nRequestID, bool bIsLast);

    // the signal of OnRspUserLogout
    void sigOnRspUserLogout(bool bIsError,CThostFtdcUserLogoutField UserLogout, CThostFtdcRspInfoField RspInfo, int nRequestID, bool bIsLast);

    // the signal of OnRspError
    void sigOnRspError(CThostFtdcRspInfoField RspInfo, int nRequestID, bool bIsLast);

    // the signal of OnRspSubMarketData
    void sigOnRspSubMarketData(bool bIsError,CThostFtdcSpecificInstrumentField SpecificInstrument, CThostFtdcRspInfoField RspInfo, int nRequestID, bool bIsLast);

    // the signal of OnRspUnSubMarketData
    void sigOnRspUnSubMarketData(bool bIsError,CThostFtdcSpecificInstrumentField SpecificInstrument, CThostFtdcRspInfoField RspInfo, int nRequestID, bool bIsLast);

    // the signal of OnRspSubForQuoteRsp
    void sigOnRspSubForQuoteRsp(bool bIsError,CThostFtdcSpecificInstrumentField SpecificInstrument, CThostFtdcRspInfoField RspInfo, int nRequestID, bool bIsLast);

    // the signal of OnRspUnSubForQuoteRsp
    void sigOnRspUnSubForQuoteRsp(bool bIsError,CThostFtdcSpecificInstrumentField SpecificInstrument, CThostFtdcRspInfoField RspInfo, int nRequestID, bool bIsLast);

    // the signal of OnRtnDepthMarketData
    void sigOnRtnDepthMarketData(CThostFtdcDepthMarketDataField DepthMarketData);
    void sigOnRtnDepthMarketData(TThostFtdcInstrumentIDType chInstrumentID);

    // the signal of OnRtnForQuoteRsp
    void sigRtnForQuoteRsp(CThostFtdcForQuoteRspField ForQuoteRsp);

private:

    CThostFtdcMdApi *m_pUserApi;    // a pointer to CThostFtdcMdApi instance

    TThostFtdcBrokerIDType  m_chBrokerID;

    TThostFtdcUserIDType    m_chUserID;

    TThostFtdcPasswordType  m_chPassword;

//    TThostFtdcRequestIDType m_nRequestID;

    // ��¼��ᶩ�ĵĺ�Լ����list
    QList<QString> m_lstSecuCodes;

    // convert CThostFtdcRspInfoField* to CThostFtdcRspInfoField
    CThostFtdcRspInfoField DeepCpyCThostFtdcRspInfoField(const CThostFtdcRspInfoField* pRspInfo) const;

    // convert CThostFtdcRspUserLoginField* to CThostFtdcRspUserLoginField
    CThostFtdcRspUserLoginField DeepCpyCThostFtdcRspUserLoginField(const CThostFtdcRspUserLoginField* pUserLoginField) const;

    // convert CThostFtdcUserLogoutField* to CThostFtdcUserLogoutField
    CThostFtdcUserLogoutField DeepCpyCThostFtdcUserLogoutField(const CThostFtdcUserLogoutField* pUserLogoutField) const;

    // convert CThostFtdcSpecificInstrumentField* to CThostFtdcSpecificInstrumentField
    CThostFtdcSpecificInstrumentField DeepCpyCThostFtdcSpecificInstrumentField(const CThostFtdcSpecificInstrumentField* pSpecificInstrumentField) const;

    // convert CThostFtdcSpecificInstrumentField* to CThostFtdcSpecificInstrumentField
    CThostFtdcDepthMarketDataField DeepCpyCThostFtdcDepthMarketDataField(const CThostFtdcDepthMarketDataField* pDepthMarketData) const;

};

}

#endif // CCTPMDSPI_H
