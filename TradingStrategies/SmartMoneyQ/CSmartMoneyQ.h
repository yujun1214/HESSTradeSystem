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

// ����ǮQָ�������
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

    // ����SmartQָ�겢����
    void DoCalc(QDate tmCalcDate);

private:
    // ���Դ���
    QString m_StrategyCode;
    // SmartQָ������ʱ�䳤�ȣ��죩
    int m_nCalcDays;
    // �����1����K������
    QMap<QString,QVector<TSecuKMktData>*>* m_ptrKMktData;

    // ����ָ��֤ȯ��SmartQָ��
//    bool CalcSmartQ(QString _strCode,double &fSmartQ) const;
    void CalcSmartQ();

    // ����K������
    void LoadMktKData(QDate tmCalcDate);

    // ɸѡ����Ǯ���
    void FilterSmartQPort(QVector<QString>* _ptrVSmartQPort) const;

    // ����SmartMoneyQ Portfolio
    void SaveSmartQPort(QDate tmCalcDate,QVector<QString>* _ptrVSmartQPort) const;

    // ����SmartMoneyQָ��ֵ<SmartQ,code>
    std::map<double,QString>* m_ptrSmartQ;
};

} // namespace HESS

#endif // CSMARTMONEYQ_H
