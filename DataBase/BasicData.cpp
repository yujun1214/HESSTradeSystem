#include "BasicData.h"
#include <qvariant.h>

namespace HESS
{

// 定义静态实例指针
BasicData* BasicData::_BasicDataInstancePtr = nullptr;

// 取得BasicData的静态实例指针
BasicData* BasicData::getBasicDataPtr()
{
    if (_BasicDataInstancePtr == nullptr)
    {
        _BasicDataInstancePtr = new BasicData("BasicData","192.168.101.77","root","yujun791025");
    }
    return _BasicDataInstancePtr;
}

bool BasicData::getSecuKMktData(KDATATYPE _ktype, QString SecuCode, QDate tmEndDate, int nDays, QVector<TSecuKMktData> &vKMktData)
{
    strSql = "select * from Secu1MQuotationData";
}

bool BasicData::getTradingSecuCode(QDate tmDate, QVector<QString> &vSecuCode)
{
    strSql = QString("select distinct SecuCode from SecuQuotationData where tmDate = '%1' and Volume >0").arg(tmDate.toString("yyyyMMdd"));
    if(!ExecuteQuery("Get trading secu's codes."))
        return false;
    vSecuCode.clear();
    while(query.next())
    {
        vSecuCode.push_back(query.value("SecuCode").toString());
    }
    return true;
}

} // namespace HESS
