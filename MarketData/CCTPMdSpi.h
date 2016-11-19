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
    // 构造函数，pUserApi = 指向CThostFtdcMdApi类的指针
    CCTPMdSpi(CThostFtdcMdApi* pUserApi);

    // 析构函数
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

    // 添加订阅行情的合约代码
    void addSecuCode(QString _secucode);

    // set tick data dir(for backtest)
    void setTickDataDir(QString _dir);

    // 客户端连接交易后台成功后会调用OnFrontConnected回调函数
    // 可在此回调函数中进行登录操作
    virtual void OnFrontConnected();

    // 当客户端与交易后台通信连接断开时，该方法被调用。当发生这个情况后，API会自动重连，客户端可不做处理
    ///@param  nReason   错误原因
    ///         0X1001   网络读失败
    ///         0x1002   网络写失败
    ///         0x2001   接收心跳超时
    ///         0x2002   发送心跳失败
    ///         0x2003   收到错误报文
    virtual void OnFrontDisconnected(int nReason);

    // 心跳超时警告。当长时间未收到报文时，该方法调用。
    // @param nTimeLaps = 距离上次接收报文的时间
    virtual void OnHeartBeatWarning(int nTimeLapse);

    // 登录请求响应
    // 当客户端发出登录请求之后，交易托管系统返回响应时，该方法会被调用，通知客户端登录是否成功
    virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    // 登出请求响应
    // 当客户端发出退出请求之后，交易托管系统返回响应时，该方法会被调用，通知客户端退出是否成功
    virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    // 错误应答
    virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    // 订阅行情应答
    virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    // 取消订阅行情应答
    virtual void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    // 订阅询价应答
    virtual void OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    // 取消订阅询价应答
    virtual void OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    // 深度行情推送
    virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

    // 询价推送
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

    // 登录后会订阅的合约代码list
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
