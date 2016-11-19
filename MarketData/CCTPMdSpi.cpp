#include "CCTPMdSpi.h"
//#include <string.h>
#include <cstring>
#include <stdio.h>
#include <iostream>
#include <QString>
#include <QDate>
#include <QTime>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include "Utility/cstr.h"
#include "Utility/Utilities.h"
#include "Utility/CLogSys.h"
//#include "Utility/COrderRef.h"
#include "Utility/CRequestID.h"

namespace HESS
{

CCTPMdSpi::CCTPMdSpi(CThostFtdcMdApi *pUserApi)
{
    m_pUserApi = pUserApi;
    memset(m_chUserID,0,sizeof(m_chUserID));
    memset(m_chPassword,0,sizeof(m_chPassword));
//    m_nRequestID = 0;
}

CCTPMdSpi::~CCTPMdSpi()
{

}

void CCTPMdSpi::setBrokerID(TThostFtdcBrokerIDType chBrokerID)
{
    if(strlen(chBrokerID) <= 0)
        return;
    strcpy(m_chBrokerID,chBrokerID);
}

void CCTPMdSpi::setBrokerID(QString strBrokerID)
{
    if(strBrokerID.length() <= 0)
        return;
    strcpy(m_chBrokerID,strBrokerID.toStdString().c_str());
}

void CCTPMdSpi::setUserID(TThostFtdcUserIDType chUserID)
{
    if(strlen(chUserID) <= 0)
        return;
    strcpy(m_chUserID,chUserID);
}

void CCTPMdSpi::setUserID(QString strUserID)
{
    if(strUserID.length() <= 0)
        return;
    strcpy(m_chUserID,strUserID.toStdString().c_str());
}

void CCTPMdSpi::setPassword(TThostFtdcPasswordType chUserPwd)
{
    if(strlen(chUserPwd) <= 0)
        return;
    strcpy(m_chPassword,chUserPwd);
}

void CCTPMdSpi::setPassword(QString strUserPwd)
{
    if(strUserPwd.length() <= 0)
        return;
    strcpy(m_chPassword,strUserPwd.toStdString().c_str());
}

void CCTPMdSpi::addSecuCode(QString _secucode)
{
    if(_secucode.length() <= 0)
        return;
    if(m_lstSecuCodes.contains(_secucode))
        return;
    m_lstSecuCodes.push_back(_secucode);
}

void CCTPMdSpi::OnFrontConnected()
{
    // if client has connected to the server, then send the login request
    emit sigOnFrontConnected();
    std::cout << "Has connected to the market data server." << std::endl;

    // setup the login parameters
//    CConfig theConfig("Config.ini");
//    TThostFtdcBrokerIDType chBrokerID;
//    if(theConfig.getConfig("BrokerID",chBrokerID))
//    {
//        std::cout << "Failed to read Broker ID." << std::endl;
//        return;
//    }

    CThostFtdcReqUserLoginField reqUserLoginParam = {0};
    strcpy(reqUserLoginParam.BrokerID,m_chBrokerID);
    strcpy(reqUserLoginParam.UserID,m_chUserID);
    strcpy(reqUserLoginParam.Password,m_chPassword);

    // send the login request
    m_pUserApi->ReqUserLogin(&reqUserLoginParam,CRequestID::getCRequestIDPtr()->getValidRequestID());
}

void CCTPMdSpi::OnFrontDisconnected(int nReason)
{
    QString strReason = "";
    switch(nReason)
    {
    case 0x1001:
//        strReason = Cstr("ÍøÂç¶ÁÊ§°Ü");
        strReason = "NetWork Reading failed";
    case 0x1002:
//        strReason = Cstr("ÍøÂçÐ´Ê§°Ü");
        strReason = "NetWork Writing failed";
    case 0x2001:
//        strReason = Cstr("½ÓÊÕÐÄÌø³¬Ê±");
        strReason = "receive heartbeat timeouted";
    case 0x2002:
//        strReason = Cstr("·¢ËÍÐÄÌøÊ§°Ü");
        strReason = "send heartbeat timeouted";
    case 0x2003:
//        strReason = Cstr("ÊÕµ½´íÎó±¨ÎÄ");
        strReason = "receive wrong message";
    }
    emit sigOnFrontDisconnected(strReason);
}

void CCTPMdSpi::OnHeartBeatWarning(int nTimeLapse)
{
    emit sigOnHeartBeatWarning(nTimeLapse);
}

void CCTPMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
//    if(pRspInfo->ErrorID != 0)
//    {// µÇÂ¼Ê§°Ü
//        std::cout << "Failed to login the mkt server. " << std::endl;
//        printf("ErrorCode=%d ErrorMsg=%s Chain=%d",pRspInfo->ErrorID,pRspInfo->ErrorMsg,bIsLast);
//        return;
//    }

    bool bIsError = false;
    if(pRspInfo->ErrorID != 0)
        bIsError = true;
    CThostFtdcRspUserLoginField RspUserLogin = DeepCpyCThostFtdcRspUserLoginField(pRspUserLogin);
    CThostFtdcRspInfoField RspInfo = DeepCpyCThostFtdcRspInfoField(pRspInfo);
    emit sigOnRspUserLogin(bIsError,RspUserLogin,RspInfo,nRequestID,bIsLast);

    // ¶©ÔÄÐÐÇé
    if(!m_lstSecuCodes.empty())
    {
        char* _ppSecuCode[128];
        int i = 0;
        for(QList<QString>::const_iterator it = m_lstSecuCodes.begin();it != m_lstSecuCodes.end();++it,++i)
        {
            _ppSecuCode[i] = new char[31];
            strcpy(_ppSecuCode[i],(*it).toStdString().c_str());
        }
        m_pUserApi->SubscribeMarketData(_ppSecuCode,i);
        // ÊÍ·ÅÄÚ´æ
        for(int j = 0;j < i;++j)
        {
            delete _ppSecuCode[j];
        }
    }
}

void CCTPMdSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    bool bIsError = false;
    if(pRspInfo->ErrorID != 0)
        bIsError = true;
    CThostFtdcUserLogoutField UserLogout = DeepCpyCThostFtdcUserLogoutField(pUserLogout);
    CThostFtdcRspInfoField RspInfo = DeepCpyCThostFtdcRspInfoField(pRspInfo);
    emit sigOnRspUserLogout(bIsError,UserLogout,RspInfo,nRequestID,bIsLast);
}

void CCTPMdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if(pRspInfo == NULL)
        return;
    CThostFtdcRspInfoField RspInfo = DeepCpyCThostFtdcRspInfoField(pRspInfo);
    emit sigOnRspError(RspInfo,nRequestID,bIsLast);
}

void CCTPMdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    bool bIsError = false;
    if(pRspInfo->ErrorID != 0)
        bIsError = true;
    CThostFtdcSpecificInstrumentField SpecificInstrument = DeepCpyCThostFtdcSpecificInstrumentField(pSpecificInstrument);
    CThostFtdcRspInfoField RspInfo = DeepCpyCThostFtdcRspInfoField(pRspInfo);
    emit sigOnRspSubMarketData(bIsError,SpecificInstrument,RspInfo,nRequestID,bIsLast);
}

void CCTPMdSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    bool bIsError = false;
    if(pRspInfo->ErrorID != 0)
        bIsError = true;
    CThostFtdcSpecificInstrumentField SpecificInstrument = DeepCpyCThostFtdcSpecificInstrumentField(pSpecificInstrument);
    CThostFtdcRspInfoField RspInfo = DeepCpyCThostFtdcRspInfoField(pRspInfo);
    emit sigOnRspUnSubMarketData(bIsError,SpecificInstrument,RspInfo,nRequestID,bIsLast);
}

void CCTPMdSpi::OnRspSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    return;
}

void CCTPMdSpi::OnRspUnSubForQuoteRsp(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    return;
}

void CCTPMdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
    if(pDepthMarketData == NULL)
        return;
//    std::cout << "OnRtnDepthMarketData...,last price = " << pDepthMarketData->LastPrice << std::endl;
    CThostFtdcDepthMarketDataField DepthMarketData = DeepCpyCThostFtdcDepthMarketDataField(pDepthMarketData);
    emit sigOnRtnDepthMarketData(pDepthMarketData->InstrumentID);
    emit sigOnRtnDepthMarketData(DepthMarketData);
}

void CCTPMdSpi::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp)
{
    return;
}

CThostFtdcRspInfoField CCTPMdSpi::DeepCpyCThostFtdcRspInfoField(const CThostFtdcRspInfoField *pRspInfo) const
{
    CThostFtdcRspInfoField RspInfo = {0};
    if(pRspInfo != NULL)
    {
        RspInfo.ErrorID = pRspInfo->ErrorID;
        strcpy(RspInfo.ErrorMsg,pRspInfo->ErrorMsg);
    }
    return RspInfo;
}

CThostFtdcRspUserLoginField CCTPMdSpi::DeepCpyCThostFtdcRspUserLoginField(const CThostFtdcRspUserLoginField *pUserLoginField) const
{
    CThostFtdcRspUserLoginField UserLogin = {0};
    if(pUserLoginField != NULL)
    {
        strcpy(UserLogin.TradingDay,pUserLoginField->TradingDay);
        strcpy(UserLogin.LoginTime,pUserLoginField->LoginTime);
        strcpy(UserLogin.BrokerID,pUserLoginField->BrokerID);
        strcpy(UserLogin.UserID,pUserLoginField->UserID);
        strcpy(UserLogin.SystemName,pUserLoginField->SystemName);
        UserLogin.FrontID = pUserLoginField->FrontID;
        UserLogin.SessionID = pUserLoginField->SessionID;
        strcpy(UserLogin.MaxOrderRef,pUserLoginField->MaxOrderRef);
        strcpy(UserLogin.SHFETime,pUserLoginField->SHFETime);
        strcpy(UserLogin.DCETime,pUserLoginField->DCETime);
        strcpy(UserLogin.CZCETime,pUserLoginField->CZCETime);
        strcpy(UserLogin.FFEXTime,pUserLoginField->FFEXTime);
        strcpy(UserLogin.INETime,pUserLoginField->INETime);
    }
    return UserLogin;
}

CThostFtdcUserLogoutField CCTPMdSpi::DeepCpyCThostFtdcUserLogoutField(const CThostFtdcUserLogoutField *pUserLogoutField) const
{
    CThostFtdcUserLogoutField UserLogout = {0};
    if(pUserLogoutField != NULL)
    {
        strcpy(UserLogout.BrokerID,pUserLogoutField->BrokerID);
        strcpy(UserLogout.UserID,pUserLogoutField->UserID);
    }
    return UserLogout;
}

CThostFtdcSpecificInstrumentField CCTPMdSpi::DeepCpyCThostFtdcSpecificInstrumentField(const CThostFtdcSpecificInstrumentField *pSpecificInstrumentField) const
{
    CThostFtdcSpecificInstrumentField SpecificInstrument = {0};
    if(pSpecificInstrumentField != 0)
    {
        strcpy(SpecificInstrument.InstrumentID,pSpecificInstrumentField->InstrumentID);
    }
    return SpecificInstrument;
}

CThostFtdcDepthMarketDataField CCTPMdSpi::DeepCpyCThostFtdcDepthMarketDataField(const CThostFtdcDepthMarketDataField *pDepthMarketData) const
{
    CThostFtdcDepthMarketDataField DepthMarketData = {0};
    if(pDepthMarketData != NULL)
    {
        strcpy(DepthMarketData.TradingDay,pDepthMarketData->TradingDay);
        strcpy(DepthMarketData.InstrumentID,pDepthMarketData->InstrumentID);
        strcpy(DepthMarketData.ExchangeID,pDepthMarketData->ExchangeID);
        strcpy(DepthMarketData.ExchangeInstID,pDepthMarketData->ExchangeInstID);
        DepthMarketData.LastPrice = pDepthMarketData->LastPrice;
        DepthMarketData.PreSettlementPrice = pDepthMarketData->PreSettlementPrice;
        DepthMarketData.PreClosePrice = pDepthMarketData->PreClosePrice;
        DepthMarketData.PreOpenInterest = pDepthMarketData->PreOpenInterest;
        DepthMarketData.OpenPrice = pDepthMarketData->OpenPrice;
        DepthMarketData.HighestPrice = pDepthMarketData->HighestPrice;
        DepthMarketData.LowestPrice = pDepthMarketData->LowestPrice;
        DepthMarketData.Volume = pDepthMarketData->Volume;
        DepthMarketData.Turnover = pDepthMarketData->Turnover;
        DepthMarketData.OpenInterest = pDepthMarketData->OpenInterest;
        DepthMarketData.ClosePrice = pDepthMarketData->ClosePrice;
        DepthMarketData.SettlementPrice = pDepthMarketData->SettlementPrice;
        DepthMarketData.UpperLimitPrice = pDepthMarketData->UpperLimitPrice;
        DepthMarketData.LowerLimitPrice = pDepthMarketData->LowerLimitPrice;
        DepthMarketData.PreDelta = pDepthMarketData->PreDelta;
        DepthMarketData.CurrDelta = pDepthMarketData->CurrDelta;
        strcpy(DepthMarketData.UpdateTime,pDepthMarketData->UpdateTime);
        DepthMarketData.UpdateMillisec = pDepthMarketData->UpdateMillisec;

        DepthMarketData.BidPrice1 = pDepthMarketData->BidPrice1;
        DepthMarketData.BidVolume1 = pDepthMarketData->BidVolume1;
        DepthMarketData.AskPrice1 = pDepthMarketData->AskPrice1;
        DepthMarketData.AskVolume1 = pDepthMarketData->AskVolume1;

        DepthMarketData.BidPrice2 = pDepthMarketData->BidPrice2;
        DepthMarketData.BidVolume2 = pDepthMarketData->BidVolume2;
        DepthMarketData.AskPrice2 = pDepthMarketData->AskPrice2;
        DepthMarketData.AskVolume2 = pDepthMarketData->AskVolume2;

        DepthMarketData.BidPrice3 = pDepthMarketData->BidPrice3;
        DepthMarketData.BidVolume3 = pDepthMarketData->BidVolume3;
        DepthMarketData.AskPrice3 = pDepthMarketData->AskPrice3;
        DepthMarketData.AskVolume3 = pDepthMarketData->AskVolume3;

        DepthMarketData.BidPrice4 = pDepthMarketData->BidPrice4;
        DepthMarketData.BidVolume4 = pDepthMarketData->BidVolume4;
        DepthMarketData.AskPrice4 = pDepthMarketData->AskPrice4;
        DepthMarketData.AskVolume4 = pDepthMarketData->AskVolume4;

        DepthMarketData.BidPrice5 = pDepthMarketData->BidPrice5;
        DepthMarketData.BidVolume5 = pDepthMarketData->BidVolume5;
        DepthMarketData.AskPrice5 = pDepthMarketData->AskPrice5;
        DepthMarketData.AskVolume5 = pDepthMarketData->AskVolume5;

        DepthMarketData.AveragePrice = pDepthMarketData->AveragePrice;
        strcpy(DepthMarketData.ActionDay,pDepthMarketData->ActionDay);
    }
    return DepthMarketData;
}

} // namespace HESS
