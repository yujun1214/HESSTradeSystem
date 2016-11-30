#ifndef CTFMMBACKTEST_H
#define CTFMMBACKTEST_H

#include "CTP/inc/ThostFtdcTraderApi.h"
#include "Utility/CTimeController.h"
#include "CMMDispatcher.h"
#include "CTFMarketMaker.h"
#include <QList>
#include <QString>
#include <QMap>
#include <QVector>
#include <QDate>
#include <QFile>

namespace HESS
{

#define EPSILON 0.00001
#define MULTIPLIER 10000

// ��ծ�ڻ������̲��ԵĻز���
class CTFMMBackTest : public CThostFtdcTraderApi
{
public:
    CTFMMBackTest():m_ptrTimeController(nullptr),m_ptrDispatcher(nullptr),m_ptrOrderPool(nullptr),m_ptrPnLFile(nullptr),m_fPnL(0.0)
    {
        initParam();
    }
    ~CTFMMBackTest();

    // ִ�лز�
    void doBackTest();

    /* �����Ǵ�CThostFtdcTraderApi�̳еĽӿ� */
    ///ɾ���ӿڶ�����
    ///@remark ����ʹ�ñ��ӿڶ���ʱ,���øú���ɾ���ӿڶ���
    virtual void Release(){}

    ///��ʼ��
    ///@remark ��ʼ�����л���,ֻ�е��ú�,�ӿڲſ�ʼ����
    virtual void Init(){}

    ///�ȴ��ӿ��߳̽�������
    ///@return �߳��˳�����
    virtual int Join(){return 0;}

    ///��ȡ��ǰ������
    ///@retrun ��ȡ���Ľ�����
    ///@remark ֻ�е�¼�ɹ���,���ܵõ���ȷ�Ľ�����
    virtual const char *GetTradingDay(){return "20301231";}

    ///ע��ǰ�û������ַ
    ///@param pszFrontAddress��ǰ�û������ַ��
    ///@remark �����ַ�ĸ�ʽΪ����protocol://ipaddress:port�����磺��tcp://127.0.0.1:17001����
    ///@remark ��tcp��������Э�飬��127.0.0.1�������������ַ����17001������������˿ںš�
    virtual void RegisterFront(char *pszFrontAddress){}

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
    virtual void RegisterSpi(CThostFtdcTraderSpi *pSpi){}

    ///����˽������
    ///@param nResumeType ˽�����ش���ʽ
    ///        THOST_TERT_RESTART:�ӱ������տ�ʼ�ش�
    ///        THOST_TERT_RESUME:���ϴ��յ�������
    ///        THOST_TERT_QUICK:ֻ���͵�¼��˽����������
    ///@remark �÷���Ҫ��Init����ǰ���á����������򲻻��յ�˽���������ݡ�
    virtual void SubscribePrivateTopic(THOST_TE_RESUME_TYPE nResumeType){}

    ///���Ĺ�������
    ///@param nResumeType �������ش���ʽ
    ///        THOST_TERT_RESTART:�ӱ������տ�ʼ�ش�
    ///        THOST_TERT_RESUME:���ϴ��յ�������
    ///        THOST_TERT_QUICK:ֻ���͵�¼�󹫹���������
    ///@remark �÷���Ҫ��Init����ǰ���á����������򲻻��յ������������ݡ�
    virtual void SubscribePublicTopic(THOST_TE_RESUME_TYPE nResumeType){}

    ///�ͻ�����֤����
    virtual int ReqAuthenticate(CThostFtdcReqAuthenticateField *pReqAuthenticateField, int nRequestID){return 0;}

    ///�û���¼����
    virtual int ReqUserLogin(CThostFtdcReqUserLoginField *pReqUserLoginField, int nRequestID){return 0;}

    ///�ǳ�����
    virtual int ReqUserLogout(CThostFtdcUserLogoutField *pUserLogout, int nRequestID){return 0;}

    ///�û������������
    virtual int ReqUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate, int nRequestID){return 0;}

    ///�ʽ��˻������������
    virtual int ReqTradingAccountPasswordUpdate(CThostFtdcTradingAccountPasswordUpdateField *pTradingAccountPasswordUpdate, int nRequestID){return 0;}

    ///����¼������
    virtual int ReqOrderInsert(CThostFtdcInputOrderField *pInputOrder, int nRequestID);

    ///Ԥ��¼������
    virtual int ReqParkedOrderInsert(CThostFtdcParkedOrderField *pParkedOrder, int nRequestID){return 0;}

    ///Ԥ�񳷵�¼������
    virtual int ReqParkedOrderAction(CThostFtdcParkedOrderActionField *pParkedOrderAction, int nRequestID){return 0;}

    ///������������
    virtual int ReqOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, int nRequestID);

    ///��ѯ��󱨵���������
    virtual int ReqQueryMaxOrderVolume(CThostFtdcQueryMaxOrderVolumeField *pQueryMaxOrderVolume, int nRequestID){return 0;}

    ///Ͷ���߽�����ȷ��
    virtual int ReqSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm, int nRequestID){return 0;}

    ///����ɾ��Ԥ��
    virtual int ReqRemoveParkedOrder(CThostFtdcRemoveParkedOrderField *pRemoveParkedOrder, int nRequestID){return 0;}

    ///����ɾ��Ԥ�񳷵�
    virtual int ReqRemoveParkedOrderAction(CThostFtdcRemoveParkedOrderActionField *pRemoveParkedOrderAction, int nRequestID){return 0;}

    ///ִ������¼������
    virtual int ReqExecOrderInsert(CThostFtdcInputExecOrderField *pInputExecOrder, int nRequestID){return 0;}

    ///ִ�������������
    virtual int ReqExecOrderAction(CThostFtdcInputExecOrderActionField *pInputExecOrderAction, int nRequestID){return 0;}

    ///ѯ��¼������
    virtual int ReqForQuoteInsert(CThostFtdcInputForQuoteField *pInputForQuote, int nRequestID){return 0;}

    ///����¼������
    virtual int ReqQuoteInsert(CThostFtdcInputQuoteField *pInputQuote, int nRequestID){return 0;}

    ///���۲�������
    virtual int ReqQuoteAction(CThostFtdcInputQuoteActionField *pInputQuoteAction, int nRequestID){return 0;}

    ///����������������
    virtual int ReqBatchOrderAction(CThostFtdcInputBatchOrderActionField *pInputBatchOrderAction, int nRequestID){return 0;}

    ///�������¼������
    virtual int ReqCombActionInsert(CThostFtdcInputCombActionField *pInputCombAction, int nRequestID){return 0;}

    ///�����ѯ����
    virtual int ReqQryOrder(CThostFtdcQryOrderField *pQryOrder, int nRequestID){return 0;}

    ///�����ѯ�ɽ�
    virtual int ReqQryTrade(CThostFtdcQryTradeField *pQryTrade, int nRequestID){return 0;}

    ///�����ѯͶ���ֲ߳�
    virtual int ReqQryInvestorPosition(CThostFtdcQryInvestorPositionField *pQryInvestorPosition, int nRequestID){return 0;}

    ///�����ѯ�ʽ��˻�
    virtual int ReqQryTradingAccount(CThostFtdcQryTradingAccountField *pQryTradingAccount, int nRequestID){return 0;}

    ///�����ѯͶ����
    virtual int ReqQryInvestor(CThostFtdcQryInvestorField *pQryInvestor, int nRequestID){return 0;}

    ///�����ѯ���ױ���
    virtual int ReqQryTradingCode(CThostFtdcQryTradingCodeField *pQryTradingCode, int nRequestID){return 0;}

    ///�����ѯ��Լ��֤����
    virtual int ReqQryInstrumentMarginRate(CThostFtdcQryInstrumentMarginRateField *pQryInstrumentMarginRate, int nRequestID){return 0;}

    ///�����ѯ��Լ��������
    virtual int ReqQryInstrumentCommissionRate(CThostFtdcQryInstrumentCommissionRateField *pQryInstrumentCommissionRate, int nRequestID){return 0;}

    ///�����ѯ������
    virtual int ReqQryExchange(CThostFtdcQryExchangeField *pQryExchange, int nRequestID){return 0;}

    ///�����ѯ��Ʒ
    virtual int ReqQryProduct(CThostFtdcQryProductField *pQryProduct, int nRequestID){return 0;}

    ///�����ѯ��Լ
    virtual int ReqQryInstrument(CThostFtdcQryInstrumentField *pQryInstrument, int nRequestID){return 0;}

    ///�����ѯ����
    virtual int ReqQryDepthMarketData(CThostFtdcQryDepthMarketDataField *pQryDepthMarketData, int nRequestID){return 0;}

    ///�����ѯͶ���߽�����
    virtual int ReqQrySettlementInfo(CThostFtdcQrySettlementInfoField *pQrySettlementInfo, int nRequestID){return 0;}

    ///�����ѯת������
    virtual int ReqQryTransferBank(CThostFtdcQryTransferBankField *pQryTransferBank, int nRequestID){return 0;}

    ///�����ѯͶ���ֲ߳���ϸ
    virtual int ReqQryInvestorPositionDetail(CThostFtdcQryInvestorPositionDetailField *pQryInvestorPositionDetail, int nRequestID){return 0;}

    ///�����ѯ�ͻ�֪ͨ
    virtual int ReqQryNotice(CThostFtdcQryNoticeField *pQryNotice, int nRequestID){return 0;}

    ///�����ѯ������Ϣȷ��
    virtual int ReqQrySettlementInfoConfirm(CThostFtdcQrySettlementInfoConfirmField *pQrySettlementInfoConfirm, int nRequestID){return 0;}

    ///�����ѯͶ���ֲ߳���ϸ
    virtual int ReqQryInvestorPositionCombineDetail(CThostFtdcQryInvestorPositionCombineDetailField *pQryInvestorPositionCombineDetail, int nRequestID){return 0;}

    ///�����ѯ��֤����ϵͳ���͹�˾�ʽ��˻���Կ
    virtual int ReqQryCFMMCTradingAccountKey(CThostFtdcQryCFMMCTradingAccountKeyField *pQryCFMMCTradingAccountKey, int nRequestID){return 0;}

    ///�����ѯ�ֵ��۵���Ϣ
    virtual int ReqQryEWarrantOffset(CThostFtdcQryEWarrantOffsetField *pQryEWarrantOffset, int nRequestID){return 0;}

    ///�����ѯͶ����Ʒ��/��Ʒ�ֱ�֤��
    virtual int ReqQryInvestorProductGroupMargin(CThostFtdcQryInvestorProductGroupMarginField *pQryInvestorProductGroupMargin, int nRequestID){return 0;}

    ///�����ѯ��������֤����
    virtual int ReqQryExchangeMarginRate(CThostFtdcQryExchangeMarginRateField *pQryExchangeMarginRate, int nRequestID){return 0;}

    ///�����ѯ������������֤����
    virtual int ReqQryExchangeMarginRateAdjust(CThostFtdcQryExchangeMarginRateAdjustField *pQryExchangeMarginRateAdjust, int nRequestID){return 0;}

    ///�����ѯ����
    virtual int ReqQryExchangeRate(CThostFtdcQryExchangeRateField *pQryExchangeRate, int nRequestID){return 0;}

    ///�����ѯ�����������Ա����Ȩ��
    virtual int ReqQrySecAgentACIDMap(CThostFtdcQrySecAgentACIDMapField *pQrySecAgentACIDMap, int nRequestID){return 0;}

    ///�����ѯ��Ʒ���ۻ���
    virtual int ReqQryProductExchRate(CThostFtdcQryProductExchRateField *pQryProductExchRate, int nRequestID){return 0;}

    ///�����ѯ��Ʒ��
    virtual int ReqQryProductGroup(CThostFtdcQryProductGroupField *pQryProductGroup, int nRequestID){return 0;}

    ///�����ѯ�����̺�Լ��������
    virtual int ReqQryMMInstrumentCommissionRate(CThostFtdcQryMMInstrumentCommissionRateField *pQryMMInstrumentCommissionRate, int nRequestID){return 0;}

    ///�����ѯ��������Ȩ��Լ������
    virtual int ReqQryMMOptionInstrCommRate(CThostFtdcQryMMOptionInstrCommRateField *pQryMMOptionInstrCommRate, int nRequestID){return 0;}

    ///�����ѯ����������
    virtual int ReqQryInstrumentOrderCommRate(CThostFtdcQryInstrumentOrderCommRateField *pQryInstrumentOrderCommRate, int nRequestID){return 0;}

    ///�����ѯ��Ȩ���׳ɱ�
    virtual int ReqQryOptionInstrTradeCost(CThostFtdcQryOptionInstrTradeCostField *pQryOptionInstrTradeCost, int nRequestID){return 0;}

    ///�����ѯ��Ȩ��Լ������
    virtual int ReqQryOptionInstrCommRate(CThostFtdcQryOptionInstrCommRateField *pQryOptionInstrCommRate, int nRequestID){return 0;}

    ///�����ѯִ������
    virtual int ReqQryExecOrder(CThostFtdcQryExecOrderField *pQryExecOrder, int nRequestID){return 0;}

    ///�����ѯѯ��
    virtual int ReqQryForQuote(CThostFtdcQryForQuoteField *pQryForQuote, int nRequestID){return 0;}

    ///�����ѯ����
    virtual int ReqQryQuote(CThostFtdcQryQuoteField *pQryQuote, int nRequestID){return 0;}

    ///�����ѯ��Ϻ�Լ��ȫϵ��
    virtual int ReqQryCombInstrumentGuard(CThostFtdcQryCombInstrumentGuardField *pQryCombInstrumentGuard, int nRequestID){return 0;}

    ///�����ѯ�������
    virtual int ReqQryCombAction(CThostFtdcQryCombActionField *pQryCombAction, int nRequestID){return 0;}

    ///�����ѯת����ˮ
    virtual int ReqQryTransferSerial(CThostFtdcQryTransferSerialField *pQryTransferSerial, int nRequestID){return 0;}

    ///�����ѯ����ǩԼ��ϵ
    virtual int ReqQryAccountregister(CThostFtdcQryAccountregisterField *pQryAccountregister, int nRequestID){return 0;}

    ///�����ѯǩԼ����
    virtual int ReqQryContractBank(CThostFtdcQryContractBankField *pQryContractBank, int nRequestID){return 0;}

    ///�����ѯԤ��
    virtual int ReqQryParkedOrder(CThostFtdcQryParkedOrderField *pQryParkedOrder, int nRequestID){return 0;}

    ///�����ѯԤ�񳷵�
    virtual int ReqQryParkedOrderAction(CThostFtdcQryParkedOrderActionField *pQryParkedOrderAction, int nRequestID){return 0;}

    ///�����ѯ����֪ͨ
    virtual int ReqQryTradingNotice(CThostFtdcQryTradingNoticeField *pQryTradingNotice, int nRequestID){return 0;}

    ///�����ѯ���͹�˾���ײ���
    virtual int ReqQryBrokerTradingParams(CThostFtdcQryBrokerTradingParamsField *pQryBrokerTradingParams, int nRequestID){return 0;}

    ///�����ѯ���͹�˾�����㷨
    virtual int ReqQryBrokerTradingAlgos(CThostFtdcQryBrokerTradingAlgosField *pQryBrokerTradingAlgos, int nRequestID){return 0;}

    ///�����ѯ��������û�����
    virtual int ReqQueryCFMMCTradingAccountToken(CThostFtdcQueryCFMMCTradingAccountTokenField *pQueryCFMMCTradingAccountToken, int nRequestID){return 0;}

    ///�ڻ����������ʽ�ת�ڻ�����
    virtual int ReqFromBankToFutureByFuture(CThostFtdcReqTransferField *pReqTransfer, int nRequestID){return 0;}

    ///�ڻ������ڻ��ʽ�ת��������
    virtual int ReqFromFutureToBankByFuture(CThostFtdcReqTransferField *pReqTransfer, int nRequestID){return 0;}

    ///�ڻ������ѯ�����������
    virtual int ReqQueryBankAccountMoneyByFuture(CThostFtdcReqQueryAccountField *pReqQueryAccount, int nRequestID){return 0;}

private:
    // ��ʼ��
    void initParam();
    // time controller
    CTimeController *m_ptrTimeController;
    // market maker dispatcher
    CMMDispatcher *m_ptrDispatcher;
    // list of CTFMarketMaker
    QList<CTFMarketMaker*> m_lstTFMMPtr;

    // ��Ҫ��������Ĵ���list
    QList<QString> m_lstSecuCode;
    // ��Ҫ���͵�����������ݣ�������һ������ݣ�ÿ����أ�
    QMap<QString,QVector<CThostFtdcDepthMarketDataField>*> m_mpDepthMktData;
    // ��ʼ�������������
    // �����Ѷ��������Լ�����tick������m_ptrDepthMktData
    // ���鲥�ŵ�λ����������Ϊ0�����鲥��ʱ������Ϊ������������Сʱ��
    void initDepthMktData(QDate tmTradingday);
    // ��Լ���鲥�ŵ�λ������
    QMap<QString,int> m_mpReplayIndex;
    // ��ǰ���������Ӧ��ʱ��
    TThostFtdcTimeType m_szMktReplayTime;
    // tick�������ڵ��ļ���·��
    QString m_strTickDataDir;
    // ���غ�Լָ�����ڵ�tick����
    void loadMktData(QDate tmTradingDay);
    // �����������,�������鲥����Ϸ���false�����򷵻�true
    bool onDistributeMktData();

    // ������
    QMap<int,CThostFtdcInputOrderField*> *m_ptrOrderPool;
    // ��ʼ��������
    void initOrderPool();
    // begin and end date of backtest
    QDate m_tmBegDate;
    QDate m_tmEndDate;
    // �ز�����Ľ������б�
    QVector<QDate> m_vTradingDay;
    // ���ؿ�ʼ�������ڵĽ������б�
    // �ļ���Ϊ��TradingDay.csv��
    void loadTradingDays();
    // ��������ϣ����������أ���ȡ�������飬��϶�����
    void onMatchOrder();
    // ������ʱ��ϣ����ڱ����ύʱ�Ĵ��
    // ��Ϲ���
    // ����ʱ�����ί�м۴��ڵ�����һ�ۣ���ɽ����ɽ���Ϊί�мۡ���һ�ۡ����¼�����ȡ�м䣻���ܳɽ�����ȴ����ŵ�������ܳɽ�
    // ����ʱ�����ί�м�С�ڵ�����һ�ۣ���ɽ����ɽ���Ϊί�мۡ���һ�ۡ����¼�����ȡ�м䣻���ܳɽ�����ȴ����ŵ�������ܳɽ�
    bool MatchOrder_OnInsertOrder(CThostFtdcInputOrderField *ptrOrder);
    // ����������ϣ����ڶ��ύʱδ�ɽ��ı������д��
    // ��Ϲ���
    // ����ʱ�����ί�м۴�����һ�ۣ���ɽ����ɽ���Ϊί�м�
    // ����ʱ�����ί�м�С����һ�ۣ���ɽ����ɽ���Ϊί�м�
    bool MatchOrder_OnTraverseOrder(CThostFtdcInputOrderField *ptrOrder);
    // ���ͳɽ��ر�
    void sendRtnTrade(CThostFtdcInputOrderField *ptrOrder,double fTradePrice);

    // accumulated P&L
    double m_fPnL;
    // calculate P&L��Ŀǰֻ֧�ֵ���ƽ�֡����ֲֹ�ҹ�Ĳ���P&L����
    void calcPnL(const CThostFtdcTradeField *ptrTrade);
    // pointer to file that save P&L data,�ļ���Ϊ"PnL.txt"
    QFile *m_ptrPnLFile;
};

} // namespace HESS

#endif // CTFMMBACKTEST_H
