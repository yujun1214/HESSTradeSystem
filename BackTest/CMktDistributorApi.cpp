#include "CMktDistributorApi.h"

namespace HESS
{

void CMktDistributorApi::RegisterFront(char *pszFrontAddress)
{
    emit sigRegisterFront();
}

int CMktDistributorApi::SubscribeMarketData(char *ppInstrumentID[], int nCount)
{
    emit sigSubscribeMktData(ppInstrumentID,nCount);
    return 0;
}

int CMktDistributorApi::UnSubscribeMarketData(char *ppInstrumentID[], int nCount)
{
    emit sigUnSubscribeMktData(ppInstrumentID,nCount);
    return 0;
}

int CMktDistributorApi::ReqUserLogin(CThostFtdcReqUserLoginField *pReqUserLoginField, int nRequestID)
{
    emit sigUserLogin(nRequestID);
    return 0;
}

int CMktDistributorApi::ReqUserLogout(CThostFtdcUserLogoutField *pUserLogout, int nRequestID)
{
    emit sigUserLogout(nRequestID);
    return 0;
}

}
