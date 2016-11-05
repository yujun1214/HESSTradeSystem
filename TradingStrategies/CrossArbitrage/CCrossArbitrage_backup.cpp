#include "CCrossArbitrage.h"
#include "Utility/CConfig.h"
#include <stdio.h>
#include <QStringList>

namespace HESS
{

CCrossArbitrage::CCrossArbitrage(CThostFtdcTraderApi *pUserTraderApi)
{
    m_pUserTraderApi = pUserTraderApi;
    m_nRequestID = 0;

    m_pUserMdApi = CThostFtdcMdApi::CreateFtdcMdApi();
    m_pCTPMdSpi = new CCTPMdSpi(m_pUserMdApi);

    LoadStrategyParam();
    InitStrategyEnviron();
}

// load strategy param
bool CCrossArbitrage::LoadStrategyParam()
{
    CConfig theConfig("Config.ini");
    std::map<std::string,std::string> _mpConfig;
    if(!theConfig.getConfig(&_mpConfig))
    {
        printf("Failed to load configuration info.");
        return false;
    }
    m_mpConfig.clear();
    for(const auto& _pair : _mpConfig)
    {
        m_mpConfig[QString::fromStdString(_pair.first)] = QString::fromStdString(_pair.second);
    }
    return true;
}

// initialize strategy environment
bool CCrossArbitrage::InitStrategyEnviron()
{
    // ��ʼ���ֲ����ݡ���������
    if(!m_mpConfig.contains("ArbPairs"))
    {
        printf("Could not find config item named 'ArbPairs'.");
        return false;
    }
    QStringList arrArbPairs = m_mpConfig["ArbPairs"].split('|');    // ������

    if(!m_mpConfig.contains("MaxVolofSingleTrade"))
    {
        printf("Could not find config named 'MaxVolofSingleTrade'.");
        return false;
    }
    QStringList arrMaxVols = m_mpConfig["MaxVolofSingleTrade"].split('|');  // ÿ��������ÿ�ν��ף�һ�����ӣ��������
    if(arrArbPairs.size() != arrMaxVols.size())
    {
        printf("The number of arb pairs is not equal to the number of max vol in config.ini.");
        return false;
    }

    if(!m_mpConfig.contains("MaxBaskets"))
    {
        printf("Could not find config named 'MaxBaskets'.");
        return false;
    }
    QStringList arrMaxBaskets = m_mpConfig["MaxBaskets"].split('|');    // ÿ�������Ե���߳���������
    if(arrArbPairs.size() != arrMaxBaskets.size())
    {
        printf("The number of arb pairs is not equal to the number of max baskets in config.ini.");
        return false;
    }

    m_HoldingDataList.clear();
    m_mpLatestMktData.clear();
    int _nPairIndex = 0;
    for(QString& _strPair : arrArbPairs)
    {
        if(!_strPair.contains(','))
        {
            printf("Leg number of arb pair No.%d is less than two.",_nPairIndex+1);
            return false;
        }
        QStringList _arrPairLegTickers = _strPair.split(',');
        TSinglePairHoldingData* _pHoldingData = new TSinglePairHoldingData();
        _pHoldingData->strLegOneTicker = _arrPairLegTickers.at(0);
        _pHoldingData->strLegTwoTicker = _arrPairLegTickers.at(1);

        // ����ÿ�������Կɳ�����ߵ������������óֲ�������
        for(int i = 0;i < arrMaxBaskets.at(_nPairIndex).toInt();++i)
        {
            THoldingVolPair* _pSingleBasketVol = new THoldingVolPair();
            _pSingleBasketVol->nLegOneHoldingVol = 0;
            _pSingleBasketVol->nLegOnePlannedVol = 0;
            _pSingleBasketVol->nLegTwoHoldingVol = 0;
            _pSingleBasketVol->nLegTwoPlannedVol = 0;
            _pSingleBasketVol->nMaxHoldingVol = arrMaxVols.at(_nPairIndex).toInt();
            _pSingleBasketVol->eTradingStatus = EMPTYHOLDING;

            _pHoldingData->HoldingVolList.push_back(_pSingleBasketVol);
        }
        // ��Ӳ��Եĳֲ�����
        m_HoldingDataList.push_back(_pHoldingData);

        // ��ʼ��ÿ�������Ե�Э���ع����
        m_mpCointegrationParam[_arrPairLegTickers.at(0)+_arrPairLegTickers.at(1)] = TCointegrationParam();
        // ��ʼ�����Խ��׺�Լ������������������
        if(!m_mpLatestMktData.contains(_arrPairLegTickers.at(0)))
        {
            m_mpLatestMktData.insert(_arrPairLegTickers.at(0),TDerivativeMktDataField());
        }
        if(!m_mpLatestMktData.contains(_arrPairLegTickers.at(1)))
        {
            m_mpLatestMktData.insert(_arrPairLegTickers.at(1),TDerivativeMktDataField());
        }
        // ��ʼ�����Խ��׺�Լ������������������
        if(!m_mpSuccessiveData.contains(_arrPairLegTickers.at(0)))
        {
            m_mpSuccessiveData.insert(_arrPairLegTickers.at(0),TMktSuccessiveData());
        }
        if(!m_mpSuccessiveData.contains(_arrPairLegTickers.at(1)))
        {
            m_mpSuccessiveData.insert(_arrPairLegTickers.at(1),TMktSuccessiveData());
        }
        // ��ʼ�����Խ��׺�Լ�Ե���������ָ��ṹ������
        QString _tmpPairTickerCmb = _arrPairLegTickers.at(0)+_arrPairLegTickers.at(1);
        if(!m_mpPairMktDataPtr.contains(_tmpPairTickerCmb))
        {
            m_mpPairMktDataPtr.insert(_tmpPairTickerCmb,TPairMktDataPtr());
            m_mpPairMktDataPtr[_tmpPairTickerCmb].pLegOneSuccessiveMktData = &m_mpSuccessiveData[_arrPairLegTickers.at(0)];
            m_mpPairMktDataPtr[_tmpPairTickerCmb].pLegTwoSuccessiveMktData = &m_mpSuccessiveData[_arrPairLegTickers.at(1)];
        }

        ++_nPairIndex;
    }

    // ��ʼ��Э���ع���õ�����tick���ݸ���
    if(!m_mpConfig.contains("TickNumForRegress"))
    {
        printf("Could not find config named 'TickNumForRegress'.");
        return false;
    }
    m_nTickNumForRegress = m_mpConfig["TickNumForRegress"].toInt();

    // ��ʼ��CTP Trade Param
    if(!m_mpConfig.contains("UserID"))
    {// user id
        printf("Could not find config named 'UserID'.");
        return false;
    }
    strcpy(m_tCTPTradeParam.chUserID,m_mpConfig["UserID"].toStdString().c_str());

    if(!m_mpConfig.contains("UserPassword"))
    {// user password
        printf("Could not find config named 'UserPassword'.");
        return false;
    }
    strcpy(m_tCTPTradeParam.chUserPwd,m_mpConfig["UserPassword"].toStdString().c_str());

    // broker id
    if(!m_mpConfig.contains("BrokerID"))
    {
        printf("Could not find config named 'BrokerID'.");
        return false;
    }
    QStringList strBrokerIDInfoList = m_mpConfig["BrokerID"].split('|');
    bool _bExistBrokerID = false;
    for(QString _BrokerIDInfo:strBrokerIDInfoList)
    {
        if(_BrokerIDInfo.contains(','))
        {
            if(strcmp(_BrokerIDInfo.left(_BrokerIDInfo.indexOf(',')).toStdString().c_str(),m_tCTPTradeParam.chUserID) == 0)
            {
                strcpy(m_tCTPTradeParam.chBrokerID,_BrokerIDInfo.mid(_BrokerIDInfo.indexOf(',')+1).toStdString().c_str());
                _bExistBrokerID = true;
                break;
            }
        }
    }
    if(!_bExistBrokerID)
    {
        printf("Could not find broker id according to userid: %s",m_tCTPTradeParam.chUserID);
        return false;
    }

    // ����ǰ�ã����˿ڣ�
    if(!m_mpConfig.contains("MarketFrontAddress"))
    {
        printf("Could not find config named 'MarketFrontAddress'.");
        return false;
    }
    QStringList strMktFrontAddInfoList = m_mpConfig["MarketFrontAddress"].split('|');
    bool _bExistMktFrontAdd = false;
    for(QString _MktFrontAdd:strMktFrontAddInfoList)
    {
        if(_MktFrontAdd.contains(','))
        {
            if(strcmp(_MktFrontAdd.left(_MktFrontAdd.indexOf(',')).toStdString().c_str(),m_tCTPTradeParam.chBrokerID) == 0)
            {
                m_tCTPTradeParam.strMktFrontAddress = _MktFrontAdd.mid(_MktFrontAdd.indexOf(',')+1);
                _bExistMktFrontAdd = true;
                break;
            }
        }
    }

    // ����ǰ�ã����˿ڣ�
    if(!m_mpConfig.contains("TradeFrontAddress"))
    {
        printf("Could not find config names 'TradeFrontAddress'.");
        return false;
    }
    QStringList strTradeAddInfoList = m_mpConfig["TradeFrontAddress"].split('|');
    bool _bExistTradeFrontAdd = false;
    for(QString _TradeFrontAdd:strTradeAddInfoList)
    {
        if(_TradeFrontAdd.contains(','))
        {
            if(strcmp(_TradeFrontAdd.left(_TradeFrontAdd.indexOf(',')).toStdString().c_str(),m_tCTPTradeParam.chBrokerID) == 0)
            {
                m_tCTPTradeParam.strTradeFrontAddress = _TradeFrontAdd.mid(_TradeFrontAdd.indexOf(',')+1);
                _bExistTradeFrontAdd = true;
                break;
            }
        }
    }

    return true;
}

void CCrossArbitrage::TMktSuccessiveData::update(const CThostFtdcDepthMarketDataField &_mktData)
{
    // lock the mutex locker

    // add the last price to the array
    arrPrice[nPos++] = _mktData.LastPrice;
}

}
