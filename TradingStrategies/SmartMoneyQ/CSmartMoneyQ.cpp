#include "CSmartMoneyQ.h"
#include "DataBase/BasicData.h"
#include "Utility/CLogSys.h"

namespace HESS
{

void CSmartMoneyQ::LoadMktKData()
{
    // ȡ�õ���������ĸ��ɴ���
    QVector<QString> vSecuCode;
    if(!BasicData::getBasicDataPtr()->getTradingSecuCode(m_tmDate,vSecuCode))
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
