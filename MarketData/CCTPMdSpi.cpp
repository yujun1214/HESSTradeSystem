#include "CCTPMdSpi.h"
//#include <string.h>
#include <iostream>
#include <QString>
#include <QDate>
#include <QTime>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <cstring>
#include <stdio.h>
#include "Utility/Utilities.h"
#include "Utility/cstr.h"

namespace HESS
{

CCTPMdSpi::CCTPMdSpi(CThostFtdcMdApi *pUserApi)
{
    m_pUserApi = pUserApi;
    memset(m_chUserID,0,sizeof(m_chUserID));
    memset(m_chPassword,0,sizeof(m_chPassword));
    m_nRequestID = 0;
    m_ptrDepthMktData = new QMap<QString,QVector<CThostFtdcDepthMarketDataField>*>();
}

CCTPMdSpi::~CCTPMdSpi()
{
    if(!m_ptrDepthMktData)
    {
        QMap<QString,QVector<CThostFtdcDepthMarketDataField>*>::iterator it = m_ptrDepthMktData->begin();
        for(;it != m_ptrDepthMktData->end();++it)
        {
            delete it.value();
        }
        delete m_ptrDepthMktData;
    }
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

void CCTPMdSpi::setTickDataDir(QString _dir)
{
    m_strTickDataDir = _dir;
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
    m_pUserApi->ReqUserLogin(&reqUserLoginParam,m_nRequestID++);
}

void CCTPMdSpi::OnFrontDisconnected(int nReason)
{
    QString strReason = "";
    switch(nReason)
    {
    case 0x1001:
//        strReason = Cstr("网络读失败");
        strReason = "NetWork Reading failed";
    case 0x1002:
//        strReason = Cstr("网络写失败");
        strReason = "NetWork Writing failed";
    case 0x2001:
//        strReason = Cstr("接收心跳超时");
        strReason = "receive heartbeat timeouted";
    case 0x2002:
//        strReason = Cstr("发送心跳失败");
        strReason = "send heartbeat timeouted";
    case 0x2003:
//        strReason = Cstr("收到错误报文");
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
//    {// 登录失败
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

    // 订阅行情
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
        // 释放内存
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

void CCTPMdSpi::OnRegisterFront()
{
    OnFrontConnected();
}

void CCTPMdSpi::OnSubscribeMktData(char *ppInstrumentID[], int nCount)
{
    QString strInstrumentID;
    for(int i = 0;i < nCount;++i)
    {
        strInstrumentID = QString(ppInstrumentID[i]);
        if(m_lstSecuCodes.contains(strInstrumentID))
            continue;
        m_lstSecuCodes.push_back(strInstrumentID);

        CThostFtdcSpecificInstrumentField InstrumentField;
        strcpy(InstrumentField.InstrumentID,*(ppInstrumentID+i));
        CThostFtdcRspInfoField RspInfoField;
        RspInfoField.ErrorID = 0;
        strcpy(RspInfoField.ErrorMsg,"Subscribed Mkt data succeeded.");

        OnRspSubMarketData(&InstrumentField,&RspInfoField,0,true);
    }
}

void CCTPMdSpi::OnUnSubscribeMktData(char *ppInstrumentID[], int nCount)
{
    QString strInstrumentID;
    for(int i = 0;i < nCount;++i)
    {
        strInstrumentID = QString(ppInstrumentID[i]);
        if(!m_lstSecuCodes.contains(strInstrumentID))
            continue;
        m_lstSecuCodes.removeAll(strInstrumentID);

        CThostFtdcSpecificInstrumentField InstrumentField;
        strcpy(InstrumentField.InstrumentID,*(ppInstrumentID+i));
        CThostFtdcRspInfoField RspInfoField;
        RspInfoField.ErrorID = 0;
        strcpy(RspInfoField.ErrorMsg,"UnSubscribed Mkt data succeeded.");

        OnRspUnSubMarketData(&InstrumentField,&RspInfoField,0,true);
    }
}

void CCTPMdSpi::OnUserLogin(int nRequestID)
{
    CThostFtdcRspUserLoginField LoginField;
    memset(&LoginField,0,sizeof(LoginField));
    strcpy(LoginField.TradingDay,QDate::currentDate().toString("yyyyMMdd").toStdString().c_str());
    strcpy(LoginField.LoginTime,QTime::currentTime().toString("HH:mm:ss").toStdString().c_str());
    strcpy(LoginField.BrokerID,m_chBrokerID);
    strcpy(LoginField.UserID,m_chUserID);
    LoginField.FrontID = 0;
    LoginField.SessionID = 0;
    snprintf(LoginField.MaxOrderRef,sizeof(LoginField.MaxOrderRef),"%d",0);

    CThostFtdcRspInfoField RspInfoField;
    RspInfoField.ErrorID = 0;
    strcpy(RspInfoField.ErrorMsg,"MDApi user login succeeded.");

    OnRspUserLogin(&LoginField,&RspInfoField,nRequestID,true);

}

void CCTPMdSpi::OnUserLogout(int nRequestID)
{
    CThostFtdcUserLogoutField LogoutField;
    memset(&LogoutField,0,sizeof(LogoutField));
    strcpy(LogoutField.BrokerID,m_chBrokerID);
    strcpy(LogoutField.UserID,m_chUserID);

    CThostFtdcRspInfoField RspInfoField;
    RspInfoField.ErrorID = 0;
    strcpy(RspInfoField.ErrorMsg,"MDApi user logout succeeded.");

    OnRspUserLogout(&LogoutField,&RspInfoField,nRequestID,true);
}

void CCTPMdSpi::OnDistributeMktData()
{
    // 遍历已订阅合约代码，
    // (1)定位到播放行情的位置，
    // (2)如果定位到的行情时间小于等于（精确到秒）“当前播放行情对应的时间”，那么播放该行情
    // (3)更新“当前播放行情对应的时间
    TThostFtdcTimeType szNextMktReplayTime; // 记录下一次播放行情对应的时间（精确到秒）
    memset(szNextMktReplayTime,'0',sizeof(szNextMktReplayTime));
    bool isSetNextTime = false;     // 是否已设置了下次播放行情的对应时间
    bool isReplayCompleted = true;  // 当天行情是否全部播放完毕
    // 遍历订阅的合约代码，逐个播放速度行情数据
    for(QList<QString>::const_iterator itcode = m_lstSecuCodes.begin();itcode != m_lstSecuCodes.end();++itcode)
    {
        // 如果位置索引中不含当前合约，忽略
        if(!m_mpReplayIndex.contains(*itcode))
            continue;
        int nReplayIndex = m_mpReplayIndex[*itcode];
        // 如果深度行情数据中不含当前合约，忽略
        if(!m_ptrDepthMktData->contains(*itcode))
            continue;
        // 如果当前合约的当天深度行情已经播放完毕，忽略
        if(nReplayIndex > m_ptrDepthMktData->value(*itcode)->size())
            continue;
        isReplayCompleted = false;
        // 如果当前合约定位到的深度行情对应的时间小于等于本次行情播放时间(m_szMktReplayTime)，那么播放该深度行情
        if(strcmp(m_ptrDepthMktData->value(*itcode)->at(nReplayIndex).UpdateTime,m_szMktReplayTime) <= 0)
        {
//            OnRtnDepthMarketData(&(m_ptrDepthMktData->value(*itcode)->at(nReplayIndex)));  // 播放行情
            OnRtnDepthMarketData(&(*(*m_ptrDepthMktData)[*itcode])[nReplayIndex]);   // 播放行情
            ++m_mpReplayIndex[*itcode]; // 位置索引递增
            ++nReplayIndex;
            if(!isSetNextTime)
            {
                strcpy(szNextMktReplayTime,m_ptrDepthMktData->value(*itcode)->at(nReplayIndex).UpdateTime);
                isSetNextTime = true;
            }
            else
            {
                if(strcmp(m_ptrDepthMktData->value(*itcode)->at(nReplayIndex).UpdateTime,szNextMktReplayTime) < 0)
                {
                    strcpy(szNextMktReplayTime,m_ptrDepthMktData->value(*itcode)->at(nReplayIndex).UpdateTime);
                }
            }
        }
        else
        {
            if(!isSetNextTime)
            {
                strcpy(szNextMktReplayTime,m_ptrDepthMktData->value(*itcode)->at(nReplayIndex).UpdateTime);
                isSetNextTime = true;
            }
            else
            {
                if(strcmp(m_ptrDepthMktData->value(*itcode)->at(nReplayIndex).UpdateTime,szNextMktReplayTime) < 0)
                {
                    strcpy(szNextMktReplayTime,m_ptrDepthMktData->value(*itcode)->at(nReplayIndex).UpdateTime);
                }
            }
        }
    }

    // 更新当前播放行情对应的时间
    if(isSetNextTime)
    {
        strcpy(m_szMktReplayTime,szNextMktReplayTime);
    }
    // 如果当天行情播放完毕，发出信号
    if(isReplayCompleted)
    {
        emit sigMktDataReplayCompleted();
    }
}

void CCTPMdSpi::InitMktDataReplay(QDate tmTradingDay)
{
    // 清空之前加载的深度行情数据
    if(!m_ptrDepthMktData)
    {
        QMap<QString,QVector<CThostFtdcDepthMarketDataField>*>::iterator it = m_ptrDepthMktData->begin();
        for(;it != m_ptrDepthMktData->end();++it)
        {
            delete it.value();
        }
        m_ptrDepthMktData->clear();
    }

    // 加载已订阅行情的合约的深度行情
    LoadMktData(tmTradingDay);

    // 行情播放的位置索引重置为0
    if(!m_mpReplayIndex.empty())
        m_mpReplayIndex.clear();
    for(QList<QString>::const_iterator it = m_lstSecuCodes.begin();it != m_lstSecuCodes.end();++it)
    {
        if(!m_mpReplayIndex.contains(*it))
            m_mpReplayIndex[*it] = 0;
    }

    // 行情播放时间设置为行情数据中的最小时间
    if(!m_ptrDepthMktData->empty())
    {
        QMap<QString,QVector<CThostFtdcDepthMarketDataField>*>::const_iterator it = m_ptrDepthMktData->begin();
        strcpy(m_szMktReplayTime,it.value()->front().UpdateTime);
        ++it;
        for(;it != m_ptrDepthMktData->end();++it)
        {
            if(strcmp(it.value()->front().UpdateTime,m_szMktReplayTime) < 0)
                strcpy(m_szMktReplayTime,it.value()->front().UpdateTime);
        }
    }

    // 播放行情
    OnDistributeMktData();
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

void CCTPMdSpi::LoadMktData(QDate tmTradingDay)
{
    // 遍历订阅的合约，导入当天的tick数据
    for(QList<QString>::const_iterator itcode = m_lstSecuCodes.begin();itcode != m_lstSecuCodes.end();++itcode)
    {
        // 如果已导入该合约当天的tick数据，则跳过
        if(m_ptrDepthMktData->contains(*itcode))
            continue;

        QString strTickFilePath = QString("%1%2_%3.csv").arg(m_strTickDataDir,*itcode,tmTradingDay.toString("yyyyMMdd"));
        QFile TickFile(strTickFilePath);
        if(!TickFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            continue;
        }
        QTextStream in(&TickFile);
        QVector<CThostFtdcDepthMarketDataField>* ptrvDepthMktData = new QVector<CThostFtdcDepthMarketDataField>();
        while(!in.atEnd())
        {
            QString tickline = in.readLine();
            QStringList tickdata = tickline.split(',');
            CThostFtdcDepthMarketDataField tDepthMktData;
            strcpy(tDepthMktData.TradingDay,tickdata[0].toStdString().c_str());
            strcpy(tDepthMktData.InstrumentID,tickdata[1].toStdString().c_str());
            tDepthMktData.LastPrice = atof(tickdata[4].toStdString().c_str());
            tDepthMktData.PreSettlementPrice = atof(tickdata[5].toStdString().c_str());
            tDepthMktData.PreClosePrice = atof(tickdata[6].toStdString().c_str());
            tDepthMktData.PreOpenInterest = atof(tickdata[7].toStdString().c_str());
            tDepthMktData.OpenPrice = atof(tickdata[8].toStdString().c_str());
            tDepthMktData.HighestPrice = atof(tickdata[9].toStdString().c_str());
            tDepthMktData.LowestPrice = atof(tickdata[10].toStdString().c_str());
            tDepthMktData.Volume = atoi(tickdata[11].toStdString().c_str());
            tDepthMktData.Turnover = atof(tickdata[12].toStdString().c_str());
            tDepthMktData.OpenInterest = atof(tickdata[13].toStdString().c_str());
            tDepthMktData.ClosePrice = atof(tickdata[14].toStdString().c_str());
            tDepthMktData.SettlementPrice = atof(tickdata[15].toStdString().c_str());
            tDepthMktData.UpperLimitPrice = atof(tickdata[16].toStdString().c_str());
            tDepthMktData.LowerLimitPrice = atof(tickdata[17].toStdString().c_str());
            tDepthMktData.PreDelta = atof(tickdata[18].toStdString().c_str());
            tDepthMktData.CurrDelta = atof(tickdata[19].toStdString().c_str());
            strcpy(tDepthMktData.UpdateTime,tickdata[20].toStdString().c_str());
            tDepthMktData.UpdateMillisec = atoi(tickdata[21].toStdString().c_str());
            tDepthMktData.BidPrice1 = atof(tickdata[22].toStdString().c_str());
            tDepthMktData.BidVolume1 = atoi(tickdata[23].toStdString().c_str());
            tDepthMktData.AskPrice1 = atof(tickdata[24].toStdString().c_str());
            tDepthMktData.AskVolume1 = atoi(tickdata[25].toStdString().c_str());
            tDepthMktData.BidPrice2 = atof(tickdata[26].toStdString().c_str());
            tDepthMktData.BidVolume2 = atoi(tickdata[27].toStdString().c_str());
            tDepthMktData.AskPrice2 = atof(tickdata[28].toStdString().c_str());
            tDepthMktData.AskVolume2 = atoi(tickdata[29].toStdString().c_str());
            tDepthMktData.BidPrice3 = atof(tickdata[30].toStdString().c_str());
            tDepthMktData.BidVolume3 = atoi(tickdata[31].toStdString().c_str());
            tDepthMktData.AskPrice3 = atof(tickdata[32].toStdString().c_str());
            tDepthMktData.AskVolume3 = atoi(tickdata[33].toStdString().c_str());
            tDepthMktData.BidPrice4 = atof(tickdata[34].toStdString().c_str());
            tDepthMktData.BidVolume4 = atoi(tickdata[35].toStdString().c_str());
            tDepthMktData.AskPrice4 = atof(tickdata[36].toStdString().c_str());
            tDepthMktData.AskVolume4 = atoi(tickdata[37].toStdString().c_str());
            tDepthMktData.BidPrice5 = atof(tickdata[38].toStdString().c_str());
            tDepthMktData.BidVolume5 = atoi(tickdata[39].toStdString().c_str());
            tDepthMktData.AskPrice5 = atof(tickdata[40].toStdString().c_str());
            tDepthMktData.AskVolume5 = atoi(tickdata[41].toStdString().c_str());
            tDepthMktData.AveragePrice = atof(tickdata[42].toStdString().c_str());
            strcpy(tDepthMktData.ActionDay,tickdata[43].toStdString().c_str());
            ptrvDepthMktData->push_back(tDepthMktData);
        }
        TickFile.close();
        m_ptrDepthMktData->insert(*itcode,ptrvDepthMktData);
    }
} // LoadMktData(QDate tmTradingDay)

} // namespace HESS
