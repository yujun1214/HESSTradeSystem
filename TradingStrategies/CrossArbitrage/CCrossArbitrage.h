#ifndef CCROSSARBITRAGE_H
#define CCROSSARBITRAGE_H

#include <QObject>
#include "TradeModule/CTP/ThostFtdcTraderApi.h"
#include "MarketData/CCTPMdSpi.h"
#include "Utility/DataStructDef.h"
#include <QString>
#include <QList>
#include <QMap>
#include <QMutex>
#include <QTime>

namespace HESS
{

#define ELAPSEMSECS 3000     // ʱ�����ţ���λ�����룩

class CCrossArbitrage : public QObject
{
    Q_OBJECT

public:
    // contructor
    // _nPairNum:�ɿ��ֵ�����������
    // _nMaxHoldingNum:ÿ����������legone��legtwo�����ֲ���
    CCrossArbitrage(CThostFtdcTraderApi* pUserTraderApi,QString _LegOneTicker,
                    QString _LegTwoTicker,int _nPairNum,int _nMaxHoldingNum);

    // destrator
    ~CCrossArbitrage();

    // setter
    void setLegOneMultiple(int _multiple)
    {
        m_nLegOneMultiple = _multiple;
    }

    void setLegOnePriceTick(double _tick)
    {
        m_fLegOnePriceTick = _tick;
    }

    void setLegTwoMultiple(int _multiple)
    {
        m_nLegTwoMultiple = _multiple;
    }

    void setLegTwoPriceTick(double _tick)
    {
        m_fLegTwoPriceTick = _tick;
    }

    void setRhoLowerLimit(double _limit)
    {
        m_fRhoLowerLimit = _limit;
    }

    void setRhoUpperLimit(double _limit)
    {
        m_fRhoUpperLimit = _limit;
    }

    void setBrokerID(char _szBrokerID[])
    {
        strcpy(m_szBrokerID,_szBrokerID);
    }

    void setInvestorID(char _szInvestorID[])
    {
        strcpy(m_szInvestorID,_szInvestorID);
    }

    static void setProductionMode(PRODUCTIONMODE _mode)
    {
        m_Mode = _mode;
    }

private:
    // ticker of leg one, independent variable
    QString m_strLegOneTicker;
    // multiple of leg one
    int m_nLegOneMultiple;
    // price tick of leg one
    double m_fLegOnePriceTick;
    // ticker of leg two, dependent variable
    QString m_strLegTwoTicker;
    // multiple of leg two
    int m_nLegTwoMultiple;
    // price tick of leg two
    double m_fLegTwoPriceTick;

    static int m_nRequestID;
    static int m_nOrderRef;
    QMutex m_OrderIDMutex;
    void getValidOrderID(int& _requestid, int& _orderref);

    // ����ģʽ
    static PRODUCTIONMODE m_Mode;

    // ��simnow��replyģʽ�µ�ί�к�����ֱ�ӵ���HandleRtnOrder��HandleRtnTrade�������潻����Ϣ���ļ�
    void sendOrderOnSimulation(int _nOrderRef,QString _strCode,TThostFtdcDirectionType _chDirection,
                               TThostFtdcOffsetFlagType _chOpenClose,int _nVol,double _fPrice) const;

    // broker id
    TThostFtdcBrokerIDType m_szBrokerID;
    // investor id
    TThostFtdcInvestorIDType m_szInvestorID;

    // pointer to CThostFtdcTraderApi instance
    CThostFtdcTraderApi* m_pUserTraderApi;
    // ����Ҫ����spi������spi��ֱ�����ⲿ�Ľ���spi������spiͨ��signal��slot����

    // ����״̬����
    enum PAIRTRADINGSTATUS
    {
        // �ղ�
        EMPTYHOLDING = 0,
        // �ѷ�������ί��
        SENDEDPOSITIONORDER = 1,
        // LegOne���Ƚ���,LegTwoδ����
        LEGONESINGLEPOSITIONED = 2,
        // LegTwo���Ƚ���,LegOneδ����
        LEGTWOSINGLEPOSITIONED = 3,
        // �������
        POSITIONED = 4,
        // �ѷ���LegOne����ƽ��ί��
        SENDEDLEGONELIQUIDATIONORDER = 5,
        // �ѷ���LegTwo����ƽ��ί��
        SENDEDLEGTWOLIQUIDATIONORDER = 6,
        // �ѷ���ƽ��ί��
        SENDEDLIQUIDATIONORDER=7,
        // LegOne����ƽ�֣�LegTwoδƽ��
        LEGONELIQUIDATIONED = 8,
        // LegTwo����ƽ�֣�LegOneδƽ��
        LEGTWOLIQUIDATIONED = 9
    };

    // ��������
    enum ARBITRAGETYPE
    {
        // ����spread,spread = price of leg two - price of leg one
        SHORTSPREAD = -1,
        // ����spread
        LONGSPREAD = 1
    };

    // һ���������׵ĳֲ������ݣ���������ʱ�Ļع����������״̬������
    struct THoldingVolPair
    {
        // ��������
        ARBITRAGETYPE eArbType;

        // LegOne��ʵ�ʳֲ���
        int nLegOneHoldingVol;
        // LegOne��ʵ�ʳֲֳɱ� = sum(�ɽ�����i * �ɽ��۸�i)
        double fLegOneHoldingCost;
        // LegOne�ļƻ��ֲ���
        int nLegOnePlannedVol;
        // LegOne�Ľ���������
//        int nLegOneRequestID;
        // LegOne�ı�������
//        QString szLegOneOrderRef;

        // LegTwo��ʵ�ʳֲ���
        int nLegTwoHoldingVol;
        // LegTwo��ʵ�ʳֲֳɱ� = sum(�ɽ�����i * �ɽ��۸�i)
        double fLegTwoHoldingCost;
        // LegTwo�ļƻ��ֲ���
        int nLegTwoPlannedVol;
        // LegTwo�Ľ���������
//        int nLegTwoRequestID;
        // LegTwo�ı�������
//        QString szLegTwoOrderRef;

        // LegOne��LegTwo�ĵ������ֲ�����
        int nMaxHoldingVol;

        // ����ʱ��Pair��Э���ع����
        TCointegrationParam tRegressParam;
        // ���ʳֲֿ���ʱ�Ĳв� = unit cost of legtwo - ��intercept + slope*unit cost of legone��
        double fOpenError;

        // ����״̬����
        PAIRTRADINGSTATUS eTradingStatus;

        // ���½���״̬�趨ʱ��
        QTime tmStatusSettingTime;

        THoldingVolPair()
        {
            memset(this,0,sizeof(*this));
            eArbType = 0;
            nLegOneHoldingVol = 0;
            fLegOneHoldingCost = 0.0;
            nLegOnePlannedVol = 0;
//            nLegOneRequestID = 0;
            nLegTwoHoldingVol = 0;
            fLegTwoHoldingCost = 0.0;
            nLegTwoPlannedVol = 0;
//            nLegTwoRequestID = 0;
            nMaxHoldingVol = 0;
            eTradingStatus = EMPTYHOLDING;
        }
    };  // THoldingVolPair

    // �������Եĳֲ�����list
    // һ�������ԣ�������ͺͶ��ͣ��ɺ��ж���������׳ֲ֣�����������������ļ�������
    QList<THoldingVolPair*> m_HoldingDataList;

    // �������Ե�����Э���ع����
    TCointegrationParam m_tCointegrationParam;
    // Э���ع�����Ƿ��Ѹ��£����Ѹ�����ɽ��н���
    bool bIfUpdatedCointegrationParam;
    // Э���ع�����Ļ�����
//    QMutex m_RegressionParamMutex;

    // ���ϵ����Ч����
    double m_fRhoLowerLimit;    // ���ϵ������
    double m_fRhoUpperLimit;    // ���ϵ������

    // �������Ե���������
    TDerivativeMktDataField m_tLegOneMktData;
    TDerivativeMktDataField m_tLegTwoMktData;
    // �������ݵĻ�����
//    QMutex m_MktMutex;

    // ������������ÿ���������׵�����
//    QMap<int,THoldingVolPair*> m_mpRequestIDMapping;
    // ���ױ���������ÿ���������׵�����
    QMap<int,THoldingVolPair*> m_mpOrderRefMapping;
    // �����Ļ�����
//    QMutex m_MappingMutex;

private:
    // �����������׵��������������м۵�����ί��
    // �������£�
    // Э���ع�����Ƿ��Ѹ��£���δ����return
    // �����ֲ�����list��������ǡ��ղ֡�״̬�������Ƿ񴥷����֣������򿪲֡�һ�������������Ϳ�����һ�������ԣ���ô�ڱ�������������
    // ���ٿ��ֵڶ��������ԡ�������ǡ�������ɡ�״̬�������Ƿ񴥷�ֹӯ��ֹ��������ƽ�֡������������״̬��Ҳ��Ҫ����������г����򲹵�����
    void doArbitrage();

    // ����ί��
    // _arbtype = �������ͣ�����spread������spread��
    // _nLegOneVol = legone��ί������
    // _nLegTwoVol = legtwo��ί������
    void SubmitOpenPositionOrder(ARBITRAGETYPE _arbtype, THoldingVolPair* _pHolding,int _nLegOneVol, int _nLegTwoVol) const;

    // ƽ��ί�У�����ֹӯ��ֹ������ǰƽ�֣�
    // _arbtype = �������ͣ�����spread������spread��
    // _nLegOneVol = legone��ί������
    // _nLegTwoVol = legtwo��ί������
    void SubmitClosePositionOrder(ARBITRAGETYPE _arbtype,THoldingVolPair* _pHolding,int _nLegOneVol,int _nLegTwoVol) const;

    // ��������ί��
    void SubmitCancelOrder();

    // ����ί��
    void sendOrder(QString _strCode, TThostFtdcDirectionType _chDirection,
                   TThostFtdcOffsetFlagType _chOpenClose, int _nVol,
                   double _fPrice,THoldingVolPair* _pHolding);

    // �ж��Ƿ�ǰ�Ƿ�����Ʒ�ڻ���������ʱ��
    bool isAtTradingTime() const;


// �źš��ۣ������顢�ɽ��ر����ͽ�����Ӧ
public slots:
    // �������飬���жϴ����Ƿ�һ��
    void updateMktData(TDerivativeMktDataField _mktData);
    // ����Э���ع���������жϴ����Ƿ�һ��
    void updateCointegrationParam(TCointegrationParam _regressParam);
    // ����RtnOrder
    // _pOrder����tradespi�ڶ��д����ģ�����������delete��
    void handleRtnOrder(int _nOrderRef,CThostFtdcOrderField* _pOrder);
    // ����RtnTrade
    // _pTrade����tradespi�ڶ��д����ģ�����������delete��
    void handleRtnTrade(int _nOrderRef,CThostFtdcTradeField* _pTrade);

signals:
    // ���������ϣ��ڵ�ǰģʽΪreplyʱ����
    void sigUpdatedMktData();
};  // CCrossArbitrage

}   // HESS

#endif // CCROSSARBITRAGE_H
