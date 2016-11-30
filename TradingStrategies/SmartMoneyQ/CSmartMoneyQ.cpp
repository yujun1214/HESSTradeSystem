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
    // 取得当天有行情的个股代码
    QVector<QString> vSecuCode;
    if(!BasicData::getBasicDataPtr()->getTradingSecuCode(tmCalcDate,vSecuCode))
    {
        CLogSys::getLogSysInstance()->toConsole("[Error] Could not get trading secu codes from BasicData.");
        return;
    }
    // 遍历个股代码，导入1分钟行情数据
    m_ptrKMktData->clear(); // 清空行情数据
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
    // ptrmpSmartMoneyS存储每个个股的SmartMoneyS指标值:map<SmartMoneyS,pair<vol,amount>>
    // QMap<double,QPair<double,double> > *ptrmpSmartMoneyS = new QMap<double,QPair<double,double> >();
    std::map<double,std::pair<double,double>,std::greater<double> > *ptrmpSmartMoneyS = new std::map<double,std::pair<double,double>,std::greater<double> >();

    // 遍历个股代码，计算每个个股每分钟的smartmoneys指标，同时记录每分钟的成交量和成交金额
    for(QMap<QString,QVector<TSecuKMktData>*>::const_iterator itcode = m_ptrKMktData->begin();itcode != m_ptrKMktData->end();++itcode)
    {// 遍历个股代码
        ptrmpSmartMoneyS->clear();
        QVector<TSecuKMktData> *ptrvMktData = itcode.value();
        double fTotalVol = 0.0;     // 记录总交易量
        double fTotalAmount = 0.0;  // 记录总交易额
        for(int i = 0;i < ptrvMktData->size();++i)
        {// 遍历个股的1分钟行情，计算每分钟的SmartMoneyS值
            double fSmartMoneyS = fabs(ptrvMktData->at(i).Close/ptrvMktData->at(i).PrevClose - 1.0) / pow(ptrvMktData->at(i).Vol,0.5);
            ptrmpSmartMoneyS->insert(std::make_pair(fSmartMoneyS,std::make_pair(ptrvMktData->at(i).Vol,ptrvMktData->at(i).Amount)));

            fTotalVol += ptrvMktData->at(i).Vol;
            fTotalAmount += ptrvMktData->at(i).Amount;
        }
        // 按SmartMoneyS指标值从大到小，找出成交量累计占比前20%的SmartMoneyS指标
        double fSmartMoneyVol = 0.0;    // 记录聪明钱对应的累计成交量
        double fSmartMoneyAmount = 0.0; // 记录聪明钱对应的累计成交额
        for(std::map<double,std::pair<double,double> >::const_iterator itSMS = ptrmpSmartMoneyS->begin();itSMS != ptrmpSmartMoneyS->end();++itSMS)
        {
            if(fSmartMoneyVol > fTotalVol * 0.2)
                break;
            fSmartMoneyVol += itSMS->second.first;
            fSmartMoneyAmount += itSMS->second.second;
        }
        // 计算个股的聪明钱情绪因子Q
        double fVWap_Smart = fSmartMoneyAmount / fSmartMoneyVol;
        double fVWap_All = fTotalAmount / fTotalVol;
        double fSmartMoneyQ = fVWap_Smart / fVWap_All;
        // 保存SmartQ指标
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
    // 导入K线数据
    LoadMktKData(tmCalcDate);

    // 计算SmartQ指标
    CalcSmartQ();

    // 筛选SmartQ Portfolio
    QVector<QString> *ptrVSmartQPort = new QVector<QString>();
    FilterSmartQPort(ptrVSmartQPort);

    // 保存SmartQ Portfolio
    SaveSmartQPort(tmCalcDate,ptrVSmartQPort);
}

} // namespace HESS
