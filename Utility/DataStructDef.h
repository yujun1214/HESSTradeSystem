#ifndef DATASTRUCTDEF_H
#define DATASTRUCTDEF_H

#include <QString>
#include <QTime>
//#include "TradeModule/CTP/ThostFtdcTraderApi.h"
//#include "MarketData/API/CTP/ThostFtdcMdApi.h"

#include "CTP/inc/ThostFtdcTraderApi.h"
#include "CTP/inc/ThostFtdcMdApi.h"

namespace HESS
{

// �ڻ�&��Ȩ�������
struct DerivDepthMktDataField
{
    QString TradingDay;            // ��������(YYYYMMDD)
    QString UpdateTime;            // ����ʱ��(HHMMSSmmm)
    QString InstrumentID;          // ��Լ����
    QString ExchangeID;            // ����������
    QString ExchangeInstrID;       // ��Լ�ڽ���������
    double  Last;                  // ���¼�
    double  PreSettlement;         // �ϴν����
    double  PreClose;              // �����̼�
    double  PreOpenInsterest;      // ��ֲ���
    double  Open;                  // ���̼�
    double  High;                  // ��߼�
    double  Low;                   // ��ͼ�
    int     Volume;                // �ɽ���
    double  Turnover;              // �ɽ����
    double  OpenInterest;          // �ֲ���
    double  Close;                 // �����̼�
    double  Settlement;            // ���ν����
    double  UpperLimit;            // ��ͣ��
    double  LowerLimit;            // ��ͣ��
    double  PreDelta;              // ����ʵ��
    double  CurrDelta;             // ����ʵ��
    double  AskPrice[5];           // �嵵������
    int     AskVol[5];             // �嵵������
    double  BidPrice[5];           // �嵵�����
    int     BidVol[5];             // �嵵������
    double  AvgPrice;              // �������
};

// simple�ڻ�&��Ȩ�������
struct SimpleDerivMktDataField
{
    QString InstrumentID;       // ��Լ����
    double  Last;               // ���¼�
    int     Volume;             // �ɽ���
    double  Turnover;           // �ɽ����
    double  AskPrice;           // ��һ��
    int     AskVol;             // ��һ��
    double  BidPrice;           // ��һ��
    int     BidVol;             // ��һ��
};

// ��������Ʒ�����������
void CopyDerivativeDepthMktDataField(DerivDepthMktDataField& dest,const DerivDepthMktDataField& source);
void CopyDerivativeDepthMktDataField(DerivDepthMktDataField &dest, const CThostFtdcDepthMarketDataField &source);

// CTP trade param struct
struct TCTPTradeParam
{
    // broker id
    TThostFtdcBrokerIDType  chBrokerID;
    // user id
    TThostFtdcUserIDType    chUserID;
    // user password
    TThostFtdcPasswordType  chUserPwd;
    // front id
    TThostFtdcFrontIDType   nFrontID;
    // session id
    TThostFtdcSessionIDType nSessionID;
    // ����ǰ�õ�ַ�����˿ڣ�
    QString strMktFrontAddress;
    // ����ǰ�õ�ַ�����˿ڣ�
    QString strTradeFrontAddress;

    TCTPTradeParam(){memset(this,0,sizeof(*this));}
};

// Э���ع�����ṹ��
struct TCointegrationParam
{
    // �Ա�������
    QString strIndependentTicker;
    // ���������
    QString strDependentTicker;
    // �ؾ���
    double fIntercept;
    // б��
    double fSlope;
    // �в��׼��
    double fErrorStd;
    // ���ϵ��
    double fRho;
    // �в��ƫ��
    double fSkewness;
    // �в�ķ��
    double fKurtosis;

    // ���ֲв�ֵ����
    double fOpenUpperLimit;
    // ���ֲв�ֵ����
    double fOpenLowerLimit;

    // ֹӯֹ��ʱ��Ӧ�Ĳв�䶯ֵ
    double fErrorChgForExitTrade;

    TCointegrationParam(){memset(this,0,sizeof(*this));}
};

// ����Ʒ�������ݽṹ��
struct TDerivativeMktDataField
{
    QString ticker;          // ��Լ����
    double  Last;            // ���¼�
    double  AskPrice[5];     // �嵵������
    int     AskVol[5];       // �嵵������
    double  BidPrice[5];     // �嵵�����
    int     BidVol[5];       // �嵵������
    QTime   tmUpdateTime;    // ����ʱ��

    TDerivativeMktDataField(){memset(this,0,sizeof(*this));}

    void update(const CThostFtdcDepthMarketDataField &_mktData);
};

// ����ģʽ
enum PRODUCTIONMODE
{
    PRODUCTION = 0,     // ����ģʽ
    SIMNOW = 1,         // simnowģ��ģʽ
    BACKTEST = 2        // �ز�ģʽ
};

}

#endif // DATASTRUCTDEF_H
