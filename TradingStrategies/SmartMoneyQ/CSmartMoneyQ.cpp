#include "CSmartMoneyQ.h"
#include "DataBase/BasicData.h"
#include "DataBase/StrategyData.h"
#include "Utility/CLogSys.h"
#include <map>
#include <math.h>

namespace HESS
{

void CSmartMoneyQ::LoadMktKData(QDate tmCalcDate)
{
    // ȡ�õ���������ĸ��ɴ���
    QVector<QString> vSecuCode;
    if(!BasicData::getBasicDataPtr()->getTradingSecuCode(tmCalcDate,vSecuCode))
    {
        CLogSys::getLogSysInstance()->toConsole("[Error] Could not get trading secu codes from BasicData.");
        return;
    }
    // �������ɴ��룬����1������������
    m_ptrKMktData->clear(); // �����������
    QVector<TSecuKMktData> vMktData;
    for(QVector<QString>::const_iterator itcode = vSecuCode.begin();itcode != vSecuCode.end();++itcode)
    {
        vMktData.clear();
        if(!BasicData::getBasicDataPtr()->getSecuKMktData(KDATA_1M,*itcode,tmCalcDate,m_nCalcDays,vMktData))
        {
            CLogSys::getLogSysInstance()->toConsole(QString("Could not get 1m quotation data of %1.").arg(*itcode));
            continue;
        }
        QVector<TSecuKMktData>* ptrSingleKMktData = new QVector<TSecuKMktData>();
        for(QVector<TSecuKMktData>::iterator itdata = vMktData.begin();itdata != vMktData.end();++itdata)
        {

        }
    }
} // LoadMktKData

void CSmartMoneyQ::CalcSmartQ()
{
    // ptrmpSmartMoneyS�洢ÿ�����ɵ�SmartMoneySָ��ֵ:map<SmartMoneyS,pair<vol,amount>>
    // QMap<double,QPair<double,double> > *ptrmpSmartMoneyS = new QMap<double,QPair<double,double> >();
    std::map<double,std::pair<double,double>,std::greater<double> > *ptrmpSmartMoneyS = new std::map<double,std::pair<double,double>,std::greater<double> >();

    // �������ɴ��룬����ÿ������ÿ���ӵ�smartmoneysָ�꣬ͬʱ��¼ÿ���ӵĳɽ����ͳɽ����
    for(QMap<QString,QVector<TSecuKMktData>*>::const_iterator itcode = m_ptrKMktData->begin();itcode != m_ptrKMktData->end();++itcode)
    {// �������ɴ���
        ptrmpSmartMoneyS->clear();
        QVector<TSecuKMktData> *ptrvMktData = itcode.value();
        double fTotalVol = 0.0;     // ��¼�ܽ�����
        double fTotalAmount = 0.0;  // ��¼�ܽ��׶�
        for(int i = 0;i < ptrvMktData->size();++i)
        {// �������ɵ�1�������飬����ÿ���ӵ�SmartMoneySֵ
            double fSmartMoneyS = fabs(ptrvMktData->at(i).Close/ptrvMktData->at(i).PrevClose - 1.0) / pow(ptrvMktData->at(i).Vol,0.5);
            ptrmpSmartMoneyS->insert(std::make_pair(fSmartMoneyS,std::make_pair(ptrvMktData->at(i).Vol,ptrvMktData->at(i).Amount)));

            fTotalVol += ptrvMktData->at(i).Vol;
            fTotalAmount += ptrvMktData->at(i).Amount;
        }
        // ��SmartMoneySָ��ֵ�Ӵ�С���ҳ��ɽ����ۼ�ռ��ǰ20%��SmartMoneySָ��
        double fSmartMoneyVol = 0.0;    // ��¼����Ǯ��Ӧ���ۼƳɽ���
        double fSmartMoneyAmount = 0.0; // ��¼����Ǯ��Ӧ���ۼƳɽ���
        for(std::map<double,std::pair<double,double> >::const_iterator itSMS = ptrmpSmartMoneyS->begin();itSMS != ptrmpSmartMoneyS->end();++itSMS)
        {
            if(fSmartMoneyVol > fTotalVol * 0.2)
                break;
            fSmartMoneyVol += itSMS->second.first;
            fSmartMoneyAmount += itSMS->second.second;
        }
        // ������ɵĴ���Ǯ��������Q
        double fVWap_Smart = fSmartMoneyAmount / fSmartMoneyVol;
        double fVWap_All = fTotalAmount / fTotalVol;
        double fSmartMoneyQ = fVWap_Smart / fVWap_All;
        // ����SmartQָ��
        m_ptrSmartQ->insert(std::make_pair(fSmartMoneyQ,*itcode));
    }

}

void CSmartMoneyQ::FilterSmartQPort(QVector<QString> *_ptrVSmartQPort) const
{
    int nSecuNum = (int)(m_ptrSmartQ->size() * 0.2);
    std::map<double,QString>::const_iterator it = m_ptrSmartQ->begin();

    if(!_ptrVSmartQPort->empty())
        _ptrVSmartQPort->clear();
    for(int i = 0;i < nSecuNum;++i,++it)
    {
        _ptrVSmartQPort->push_back(it->second);
    }
}

void CSmartMoneyQ::SaveSmartQPort(QDate tmCalcDate, QVector<QString>* _ptrVSmartQPort) const
{
    if(_ptrVSmartQPort->empty())
        return;
    double fWeight = 1.0 / (double)_ptrVSmartQPort->size();
    QVector<TStrategyHoldingWeight> vHoldingWeight;
    for(QVector<QString>::const_iterator it = _ptrVSmartQPort->begin();it != _ptrVSmartQPort->end();++it)
    {
        TStrategyHoldingWeight HoldingWeight;
        HoldingWeight.tmDate = tmCalcDate;
        HoldingWeight.StrategyCode = m_StrategyCode;
        HoldingWeight.SecuCode = *it;
        HoldingWeight.fWeight = fWeight;
        vHoldingWeight.push_back(HoldingWeight);
    }
    StrategyData::getStrategyDataPtr()->saveStrategyHoldingWeightData(&vHoldingWeight);
}

void CSmartMoneyQ::DoCalc(QDate tmCalcDate)
{
    // ����K������
    LoadMktKData(tmCalcDate);

    // ����SmartQָ��
    CalcSmartQ();

    // ɸѡSmartQ Portfolio
    QVector<QString> *ptrVSmartQPort = new QVector<QString>();
    FilterSmartQPort(ptrVSmartQPort);

    // ����SmartQ Portfolio
    SaveSmartQPort(tmCalcDate,ptrVSmartQPort);
}

} // namespace HESS
