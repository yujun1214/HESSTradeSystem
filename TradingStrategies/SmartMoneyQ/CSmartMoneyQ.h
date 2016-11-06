#ifndef CSMARTMONEYQ_H
#define CSMARTMONEYQ_H

#include <QDate>
#include <QMap>
#include <QString>
#include <QVector>
#include "Utility/DataStructDef.h"

namespace HESS
{

// ����ǮQָ�������
class CSmartMoneyQ
{
public:
    CSmartMoneyQ() : m_ptrKMktData(nullptr)
    {
        if(m_ptrKMktData == nullptr)
            m_ptrKMktData = new QMap<QString,QVector<TSecuKMktData>*>();
    }
    ~CSmartMoneyQ(){}

    // ����SmartQָ�겢����
    void DoCalc();

private:
    // ��������
    QDate m_tmDate;
    // SmartQָ������ʱ�䳤�ȣ��죩
    int m_nCalcDays;
    // �����1����K������
    QMap<QString,QVector<TSecuKMktData>*>* m_ptrKMktData;

    // ����ָ��֤ȯ��SmartQָ��
    bool CalcSmartQ(QString _strCode,double &fSmartQ) const;
    // ����K������
    void LoadMktKData();
};

} // namespace HESS

#endif // CSMARTMONEYQ_H
