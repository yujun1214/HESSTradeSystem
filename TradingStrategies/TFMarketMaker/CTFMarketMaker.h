#ifndef CTFMARKETMAKER_H
#define CTFMARKETMAKER_H

#include <QObject>
#include <QString>
#include "CMMDispatcher.h"
//#include "TradeModule/CTP/ThostFtdcTraderApi.h"
#include "CTP/inc/ThostFtdcTraderApi.h"
#include <QVector>
#include "Utility/CTimeController.h"
#include "Utility/DataStructDef.h"

namespace HESS
{

// long short type
enum LONGSHORTTYPE
{
    LONGTRADING = 0,
    SHORTTRADING = 1
};

// ��ծ�ڻ���������
class CTFMarketMaker : public QObject
{
    Q_OBJECT

public:
    // constructor
    CTFMarketMaker(QString _strSecuCode,LONGSHORTTYPE _type,CMMDispatcher* _ptrMMDisptcher):
        m_strSecuCode(_strSecuCode),m_eTradingType(_type),m_ptrMMDispatcher(_ptrMMDisptcher)
    {
        m_eTradingStatus = EMPTYHOLDING;
    }

    // destructor
    ~CTFMarketMaker(){}

//    // long short type
//    enum LONGSHORTTYPE
//    {
//        LONGTRADING = 0,
//        SHORTTRADING = 1
//    };

    void setPriceTick(int _nTick)
    {
        m_nPriceTick = _nTick;
    }

    void setPricePrecision(int _nPrecision)
    {
        m_nPricePrecision = _nPrecision;
    }

    void setStoplossTicks(int _nTicks)
    {
        m_nStoplossTicks = _nTicks;
    }

//    void setMMDispatcher(CMMDispatcher* _ptrMMDispatcher)
//    {
//        m_ptrMMDispatcher = _ptrMMDispatcher;
//    }

    void setUserTradeApi(CThostFtdcTraderApi* _ptrUserTradeApi)
    {
        m_ptrUserTradeApi = _ptrUserTradeApi;
    }

    void setTimeController(CTimeController* _ptrTimeController)
    {
        m_ptrTimeController = _ptrTimeController;
    }

    void setBrokerID(TThostFtdcBrokerIDType _szBrokerID)
    {
        if(strlen(_szBrokerID) <= 0)
            return;
        strcpy(m_szBrokerID,_szBrokerID);
    }

    void setBrokerID(QString strBrokerID)
    {
        if(strBrokerID.length() <= 0)
            return;
        strcpy(m_szBrokerID,strBrokerID.toStdString().c_str());
    }

    void setInvestorID(TThostFtdcInvestorIDType _szInvestorID)
    {
        if(strlen(_szInvestorID) <= 0)
            return;
        strcpy(m_szInvestorID,_szInvestorID);
    }

    void setInvestorID(QString strInvestorID)
    {
        if(strInvestorID.length() <= 0)
            return;
        strcpy(m_szInvestorID,strInvestorID.toStdString().c_str());
    }

    LONGSHORTTYPE TradingType() const
    {
        return m_eTradingType;
    }

private:
    // secu code
    QString m_strSecuCode;
    // long short type
    LONGSHORTTYPE m_eTradingType;
    // trading status enum
    enum MMTRADINGSTATUS
    {
        // �ղ�
        EMPTYHOLDING = 0,
        // �ѷ�������ί��
        SENDEDPOSITIONORDER = 1,
        // �ѷ�������ί�г�������
        SENDEDCANCELPOSITIONORDER = 2,
        // �������
        POSITIONED = 3,
        // �ѷ���ƽ��ί��
        SENDEDLIQUIDATIONORDER = 4,
        // �ѷ���ƽ��ί�г�������ƽ��ί�г���һ����Ϊ�˳��������ֹ��
        SENDEDCANCELLIQUIDATIONORDER = 5,
        // �ѷ���ֹ��ί��
        SENDEDSTOPLOSSORDER = 6
    };
    // trading status
    MMTRADINGSTATUS m_eTradingStatus;
    // һ��price tickֵ
    int m_nPriceTick;
    // ����ľ��ȣ�������ʵ���������������ת��
    int m_nPricePrecision;
    // holding cost
    double m_fHoldingCost;
    // stop loss tick number
    int m_nStoplossTicks;
    // pointer to CMMDispatcher class
    CMMDispatcher* m_ptrMMDispatcher;
    // pointer to user trade api
    CThostFtdcTraderApi* m_ptrUserTradeApi;
    // time controller
    CTimeController* m_ptrTimeController;
    // ���ױ�������
    mutable int m_nTradeOrderRef;
    // ������������
    int m_nCancelOrderRef;
    // front id
    TThostFtdcFrontIDType m_nFrontID;
    // session id
    TThostFtdcSessionIDType m_nSessionID;
    // broker id
    TThostFtdcBrokerIDType m_szBrokerID;
    // investor id
    TThostFtdcInvestorIDType m_szInvestorID;

    // the index acquired from Dispatcher for open/close order price
    long m_nOrderIndex;

private:
    // ����ί��
    void submitOpenPositionOrder();
    // ƽ��ί��
    void submitClosePositionOrder();
    // ����ί��
    void submitCancelOrder(int _nOrderRef);
    // ����ֹ��
    // ���̿ڼ۸���ί�м۸���볬��m_nStoplossTick��tickʱ������ֹ��
    void handleStoploss();
    // �����޼�ί��
    void sendLimitOrder(TThostFtdcDirectionType _chDirection,
                        TThostFtdcOffsetFlagType _chOpenClose,int _nVol,double _fPrice) const;
    // �����м�ί��(�ڶ��ּۻ����ϼ��ϻ��ȥ10��ticks)
    void sendMktPriceOrder(TThostFtdcDirectionType _chDirection,
                           TThostFtdcOffsetFlagType _chOpenClose,int _nvol) const;


// �źš��ۣ������顢�ɽ��ر����ͽ�����Ӧ
public slots:
    // ����������£���ֹӯ��ֹ�𣩣����жϴ����Ƿ�һ��
    void handleUpdateMktData(QString _strSecuCode);
    // ����RtnOrder
    // _ptrOrder����tradespi�ڶ��д���������������delete��
    void handleRtnOrder(CThostFtdcOrderField* _ptrOrder);
    // ����RtnTrade
    // _ptrTrade����tradespi�ڶ��д���������������delete��
    void handleRtnTrade(CThostFtdcTradeField* _ptrTrade);
    // ����OnErrRtnOrderAction
    // _ptrOrderAction����tradespi�ڶ��д���������������delete��
    void handleErrRtnOrderAction(CThostFtdcOrderActionField* _ptrOrderAction);

    // ����frontid��sessionid
    void setFrontSessionID(TThostFtdcFrontIDType _nFrontID,TThostFtdcSessionIDType _nSessionID);

signals:
    // �������
    void sigPositioned(long _nIndex);
    // ����ί�б��ܾ�
    void sigOpenOrderDenied(long _nIndex);
    // ƽ�����
    void sigLiquidationed(long _nIndex);
    // ƽ��ί�б��ܾ�
    void sigCloseOrderDenied(long _nIndex);
    // ����ί�г������
    void sigOpenOrderCanceled(long _nIndex);
    // ƽ��ί�г������
    void sigCloseOrderCanceled(long _nIndex);

    // �����־������̨
    void sigToConsole(QString _loginfo);

};

}

#endif // CTFMARKETMAKER_H
