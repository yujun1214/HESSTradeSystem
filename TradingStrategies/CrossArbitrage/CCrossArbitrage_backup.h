#ifndef CCROSSARBITRAGE_H
#define CCROSSARBITRAGE_H

#include <QObject>
#include "TradeModule/CTP/ThostFtdcTraderApi.h"
#include "MarketData/CCTPMdSpi.h"
#include "Utility/DataStructDef.h"
#include <QString>
#include <QList>
#include <QMap>

namespace HESS
{

class CCrossArbitrage : public QObject,public CThostFtdcTraderSpi
{
    Q_OBJECT

public:
    // contructor
    CCrossArbitrage(CThostFtdcTraderApi* pUserTraderApi);

    // destrator
    ~CCrossArbitrage();

    // ʵ�ֵ�TraderSpi�ӿ�
    virtual void OnFrontConnected();

    virtual void OnFrontDisconnected(int nReason);

    virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

    virtual void OnRtnOrder(CThostFtdcOrderField *pOrder);

    virtual void OnRtnTrade(CThostFtdcTradeField *pTrade);

private:
    // �����ļ��е�������Ϣ
    QMap<QString,QString> m_mpConfig;
    // CTP trade param
    TCTPTradeParam m_tCTPTradeParam;
    // initial request id in a trading session
    int m_nRequestID;
    // pointer to CThostFtdcTraderApi instance
    CThostFtdcTraderApi* m_pUserTraderApi;
    // pointer to CCTPMdSpi intance
    CCTPMdSpi* m_pCTPMdSpi;
    // pointer to CThostFtdcMDApi instance
    CThostFtdcMdApi* m_pUserMdApi;

    // ����״̬����
    enum PAIRTRADINGSTATUS
    {
        // �ղ�
        EMPTYHOLDING = 0,
        // �ѷ�������ί��
        SENDEDPOSITIONORDER = 1,
        // �������
        POSITIONED = 2,
        // ���ֳ���
        CANCELINGPOSITION = 3,
        // �ѷ���ƽ��ί��
        SENDEDLIQUIDATIONORDER=4,
        // ƽ�ֳ���
        CANCELINGLIQUIDATION = 5
    };

    // һ���������׵ĳֲ������ݣ���������ʱ�Ļع����������״̬������
    struct THoldingVolPair
    {
        // LegOne��ʵ�ʳֲ���
        int nLegOneHoldingVol;
        // LegOne�ļƻ��ֲ���
        int nLegOnePlannedVol;

        // LegTwo��ʵ�ʳֲ���
        int nLegTwoHoldingVol;
        // LegTwo�ļƻ��ֲ���
        int nLegTwoPlannedVol;

        // LegOne��LegTwo�ĵ������ֲ�����
        int nMaxHoldingVol;

        // ����ʱ��Pair��Э���ع����
        TCointegrationParam tRegressParam;

        // ״̬����
        PAIRTRADINGSTATUS eTradingStatus;

        THoldingVolPair(){memset(this,0,sizeof(*this));}
    };

    // ÿ����һ�����Եĳֲ�����
    // һ�������ԣ�������ͺͶ��ͣ��ɺ��ж���������׳ֲ֣�����������������ļ�������
    struct TSinglePairHoldingData
    {
        // ticker of leg one
        QString strLegOneTicker;
        // ticker of leg two
        QString strLegTwoTicker;
        // holding of this pair
        QList<THoldingVolPair*> HoldingVolList;

        TSinglePairHoldingData(){memset(this,0,sizeof(*this));}
    };

    // �������Եĳֲ�����
    QList<TSinglePairHoldingData*> m_HoldingDataList;

    // ÿ����һ�����Ե�����Э���ع����
    // map<LegOneTicker&LegTwoTicker,TCointegrationParam>
    QMap<QString,TCointegrationParam> m_mpCointegrationParam;

    // Э���ع���õ�����tick���ݸ���
    int m_nTickNumForRegress;

    // ���ϵ����Ч����
    double m_fRhoLowerLimit;    // ���ϵ������
    double m_fRhoUpperLimit;    // ���ϵ������

    // ���Խ��׺�Լ������������������
    // map<ticker,TDerivativeMktDataField>
    QMap<QString,TDerivativeMktDataField> m_mpLatestMktData;

    // ��Լ�������������ݽṹ��
    struct TMktSuccessiveData
    {
        // ���¼����飬Ĭ������Ϊ40000��Ԫ��
        double* arrPrice;
        // ��ǰ����������λ��
        int nPos;
        // ������

        // ������������
        void update(const CThostFtdcDepthMarketDataField& _mktData);

        TMktSuccessiveData(int _nDataLen = 40000)
        {
            arrPrice = new double[_nDataLen];
            nPos = 0;
        }
    };

    // ���Խ��׺�Լ������������������
    // map<ticker,TMktSuccessiveData>
    QMap<QString,TMktSuccessiveData> m_mpSuccessiveData;

    // ���Խ��׺�Լ�Ե���������ָ��ṹ��
    struct TPairMktDataPtr
    {
        // LegOne�������������ݽṹ��ָ��
        TMktSuccessiveData* pLegOneSuccessiveMktData;
        // LegTwo�������������ݽṹ��ָ��
        TMktSuccessiveData* pLegTwoSuccessiveMktData;

        TPairMktDataPtr()
        {
            pLegOneSuccessiveMktData = NULL;
            pLegTwoSuccessiveMktData = NULL;
        }
    };

    // ���Խ��׺�Լ�Ե���������ָ��ṹ������
    // map<ticker1&ticker2,TPairMktDataPtr>
    QMap<QString,TPairMktDataPtr> m_mpPairMktDataPtr;

    // ������������ÿ���������׵�����
    QMap<int,THoldingVolPair*> m_mpRequestIDMapping;

    // ���ױ���������ÿ���������׵�����
    QMap<QString,THoldingVolPair*> m_mpOrderRefMapping;

private:
    // load strategy param
    bool LoadStrategyParam();
    // initialize strategy
    bool InitStrategyEnviron();
};

}

#endif // CCROSSARBITRAGE_H
