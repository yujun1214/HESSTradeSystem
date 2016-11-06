#ifndef CSMARTMONEYQ_H
#define CSMARTMONEYQ_H

#include <QDate>
#include <QMap>
#include <QString>
#include <QVector>
#include "Utility/DataStructDef.h"

namespace HESS
{

// 聪明钱Q指标计算类
class CSmartMoneyQ
{
public:
    CSmartMoneyQ() : m_ptrKMktData(nullptr)
    {
        if(m_ptrKMktData == nullptr)
            m_ptrKMktData = new QMap<QString,QVector<TSecuKMktData>*>();
    }
    ~CSmartMoneyQ(){}

    // 计算SmartQ指标并保存
    void DoCalc();

private:
    // 计算日期
    QDate m_tmDate;
    // SmartQ指标计算的时间长度（天）
    int m_nCalcDays;
    // 导入的1分钟K线数据
    QMap<QString,QVector<TSecuKMktData>*>* m_ptrKMktData;

    // 计算指定证券的SmartQ指标
    bool CalcSmartQ(QString _strCode,double &fSmartQ) const;
    // 导入K线数据
    void LoadMktKData();
};

} // namespace HESS

#endif // CSMARTMONEYQ_H
