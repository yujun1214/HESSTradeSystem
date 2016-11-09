#ifndef CMKTDISTRIBUTORAPI_H
#define CMKTDISTRIBUTORAPI_H

#include <QObject>
#include "CTP/inc/ThostFtdcMdApi.h"

namespace HESS
{

// 行情分发类，用于策略回测中的行情重播
class CMktDistributorApi : public QObject,public CThostFtdcMdApi
{
    Q_OBJECT
public:
    CMktDistributorApi(){}

    ~CMktDistributorApi(){}

    ///删除接口对象本身
    ///@remark 不再使用本接口对象时，调用该函数删除接口对象
    virtual void Release(){}

    ///初始化
    ///@remark 初始化运行环境，只有调用后，接口才开始工作
    virtual void Init(){}

    ///等待接口线程结束运行
    ///@return 线程退出代码
    virtual int Join(){return 0;}

    // 获取当前交易日
    ///@return 只有登录成功后，才能得到正确的交易日
    virtual const char *GetTradingDay(){return "20301231";}

    ///注册前置网络地址
    ///@param pszFrontAddress: 前置机网络地址
    ///@remark 网络地址的格式为：“protocol://ipaddress:port”,如“tcp://127.0.0.1:17001”
    ///@remark “tcp”代表传输协议，“127.0.0.1”代表服务器地址。“17001”代表服务器端口号。
    virtual void RegisterFront(char *pszFrontAddress);

    ///注册名字服务器网络地址
    ///@param pszNsAddress：名字服务器网络地址。
    ///@remark 网络地址的格式为：“protocol://ipaddress:port”，如：”tcp://127.0.0.1:12001”。
    ///@remark “tcp”代表传输协议，“127.0.0.1”代表服务器地址。”12001”代表服务器端口号。
    ///@remark RegisterNameServer优先于RegisterFront
    virtual void RegisterNameServer(char *pszNsAddress){}

    ///注册名字服务器用户信息
    ///@param pFensUserInfo：用户信息。
    virtual void RegisterFensUserInfo(CThostFtdcFensUserInfoField * pFensUserInfo){}

    ///注册回调接口
    ///@param pSpi 派生自回调接口类的实例
    virtual void RegisterSpi(CThostFtdcMdSpi *pSpi){}

    ///订阅行情。
    ///@param ppInstrumentID 合约ID
    ///@param nCount 要订阅/退订行情的合约个数
    ///@remark
    virtual int SubscribeMarketData(char *ppInstrumentID[], int nCount);

    ///退订行情。
    ///@param ppInstrumentID 合约ID
    ///@param nCount 要订阅/退订行情的合约个数
    ///@remark
    virtual int UnSubscribeMarketData(char *ppInstrumentID[], int nCount);

    ///订阅询价。
    ///@param ppInstrumentID 合约ID
    ///@param nCount 要订阅/退订行情的合约个数
    ///@remark
    virtual int SubscribeForQuoteRsp(char *ppInstrumentID[], int nCount){return 0;}

    ///退订询价。
    ///@param ppInstrumentID 合约ID
    ///@param nCount 要订阅/退订行情的合约个数
    ///@remark
    virtual int UnSubscribeForQuoteRsp(char *ppInstrumentID[], int nCount){return 0;}

    ///用户登录请求
    virtual int ReqUserLogin(CThostFtdcReqUserLoginField *pReqUserLoginField, int nRequestID);


    ///登出请求
    virtual int ReqUserLogout(CThostFtdcUserLogoutField *pUserLogout, int nRequestID);

signals:
    // signal of RegisterFront
    void sigRegisterFront();

    // signal of SubscribeMarketData
    void sigSubscribeMktData(char *ppInstrumentID[],int nCount);

    // signal of UnSubscribeMarketData
    void sigUnSubscribeMktData(char *ppInstrumentID[],int nCount);

    // signal of user login
    void sigUserLogin(int nRequestID);

    // signal of user logout
    void sigUserLogout(int nRequestID);

}; // end of class CMktDistributorApi

}

#endif // CMKTDISTRIBUTORAPI_H
