#ifndef CMKTDISTRIBUTORAPI_H
#define CMKTDISTRIBUTORAPI_H

#include <QObject>
#include "CTP/inc/ThostFtdcMdApi.h"

namespace HESS
{

// ����ַ��࣬���ڲ��Իز��е������ز�
class CMktDistributorApi : public QObject,public CThostFtdcMdApi
{
    Q_OBJECT
public:
    CMktDistributorApi(){}

    ~CMktDistributorApi(){}

    ///ɾ���ӿڶ�����
    ///@remark ����ʹ�ñ��ӿڶ���ʱ�����øú���ɾ���ӿڶ���
    virtual void Release(){}

    ///��ʼ��
    ///@remark ��ʼ�����л�����ֻ�е��ú󣬽ӿڲſ�ʼ����
    virtual void Init(){}

    ///�ȴ��ӿ��߳̽�������
    ///@return �߳��˳�����
    virtual int Join(){return 0;}

    // ��ȡ��ǰ������
    ///@return ֻ�е�¼�ɹ��󣬲��ܵõ���ȷ�Ľ�����
    virtual const char *GetTradingDay(){return "20301231";}

    ///ע��ǰ�������ַ
    ///@param pszFrontAddress: ǰ�û������ַ
    ///@remark �����ַ�ĸ�ʽΪ����protocol://ipaddress:port��,�硰tcp://127.0.0.1:17001��
    ///@remark ��tcp��������Э�飬��127.0.0.1�������������ַ����17001������������˿ںš�
    virtual void RegisterFront(char *pszFrontAddress);

    ///ע�����ַ����������ַ
    ///@param pszNsAddress�����ַ����������ַ��
    ///@remark �����ַ�ĸ�ʽΪ����protocol://ipaddress:port�����磺��tcp://127.0.0.1:12001����
    ///@remark ��tcp��������Э�飬��127.0.0.1�������������ַ����12001������������˿ںš�
    ///@remark RegisterNameServer������RegisterFront
    virtual void RegisterNameServer(char *pszNsAddress){}

    ///ע�����ַ������û���Ϣ
    ///@param pFensUserInfo���û���Ϣ��
    virtual void RegisterFensUserInfo(CThostFtdcFensUserInfoField * pFensUserInfo){}

    ///ע��ص��ӿ�
    ///@param pSpi �����Իص��ӿ����ʵ��
    virtual void RegisterSpi(CThostFtdcMdSpi *pSpi){}

    ///�������顣
    ///@param ppInstrumentID ��ԼID
    ///@param nCount Ҫ����/�˶�����ĺ�Լ����
    ///@remark
    virtual int SubscribeMarketData(char *ppInstrumentID[], int nCount);

    ///�˶����顣
    ///@param ppInstrumentID ��ԼID
    ///@param nCount Ҫ����/�˶�����ĺ�Լ����
    ///@remark
    virtual int UnSubscribeMarketData(char *ppInstrumentID[], int nCount);

    ///����ѯ�ۡ�
    ///@param ppInstrumentID ��ԼID
    ///@param nCount Ҫ����/�˶�����ĺ�Լ����
    ///@remark
    virtual int SubscribeForQuoteRsp(char *ppInstrumentID[], int nCount){return 0;}

    ///�˶�ѯ�ۡ�
    ///@param ppInstrumentID ��ԼID
    ///@param nCount Ҫ����/�˶�����ĺ�Լ����
    ///@remark
    virtual int UnSubscribeForQuoteRsp(char *ppInstrumentID[], int nCount){return 0;}

    ///�û���¼����
    virtual int ReqUserLogin(CThostFtdcReqUserLoginField *pReqUserLoginField, int nRequestID);


    ///�ǳ�����
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
