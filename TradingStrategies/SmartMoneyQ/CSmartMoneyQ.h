#ifndef CSMARTMONEYQ_H
#define CSMARTMONEYQ_H

#include <QDate>
#include <QMap>
#include <QString>
#include <QVector>
#include <map>
#include "Utility/DataStructDef.h"

namespace HESS
{

// 聪明钱Q指标计算类
class CSmartMoneyQ
{
public:
    CSmartMoneyQ(int _nCalcDays) : m_StrategyCode("SmartMoneyQ"),m_nCalcDays(_nCalcDays),m_ptrKMktData(nullptr),m_ptrSmartQ(nullptr)
    {
        if(m_ptrKMktData == nullptr)
            m_ptrKMktData = new QMap<QString,QVector<TSecuKMktData>*>();
        if(m_ptrSmartQ == nullptr)
            m_ptrSmartQ = new std::map<double,QString>();
    }
    ~CSmartMoneyQ(){}

    // 计算SmartQ指标并保存
    void DoCalc(QDate tmCalcDate);

private:
    // 策略代码
    QString m_StrategyCode;
    // SmartQ指标计算的时间长度（天）
    int m_nCalcDays;
    // 导入的1分钟K线数据
    QMap<QString,QVector<TSecuKMktData>*>* m_ptrKMktData;

    // 计算指定证券的SmartQ指标
//    bool CalcSmartQ(QString _strCode,double &fSmartQ) const;
    void CalcSmartQ();

    // 导入K线数据
    void LoadMktKData(QDate tmCalcDate);

    // 筛选聪明钱组合
    void FilterSmartQPort(QVector<QString>* _ptrVSmartQPort) const;

    // 保存SmartMoneyQ Portfolio
    void SaveSmartQPort(QDate tmCalcDate,QVector<QString>* _ptrVSmartQPort) const;

    // 保存SmartMoneyQ指标值<SmartQ,code>
    std::map<double,QString>* m_ptrSmartQ;
};

} // namespace HESS

#endif // CSMARTMONEYQ_H
