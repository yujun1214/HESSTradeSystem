#include "CSmartMoneyQ.h"
#include "DataBase/BasicData.h"
#include "Utility/CLogSys.h"

namespace HESS
{

void CSmartMoneyQ::LoadMktKData()
{
    // 取得当天有行情的个股代码
    QVector<QString> vSecuCode;
    if(!BasicData::getBasicDataPtr()->getTradingSecuCode(m_tmDate,vSecuCode))
    {
        CLogSys::getLogSysInstance()->toConsole("[Error] Could not get trading secu codes from BasicData.");
        return;
    }
    // 遍历个股代码，导入1分钟行情数据
    m_ptrKMktData->clear(); // 情况行情数据
    QVector<TSecuKMktData> vMktData;
    for(QVector<QString>::const_iterator itcode = vSecuCode.begin();itcode != vSecuCode.end();++itcode)
    {
        vMktData.clear();
        if(!BasicData::getBasicDataPtr()->getSecuKMktData(*itcode,m_tmDate,m_nCalcDays,vMktData))
        {
            CLogSys::getLogSysInstance()->toConsole(QString("Could not get 1m quotation data of %1.").arg(*itcode));
            continue;
        }
        QVector<TSecuKMktData>* ptrSingleKMktData = new QVector<TSecuKMktData>();
        for(QVector<TSecuKMktData>::iterator itdata = vMktData.begin();itdata != vMktData.end();++itdata)
        {

        }
    }
}

} // namespace HESS
