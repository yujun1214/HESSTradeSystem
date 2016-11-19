#ifndef STRATEGYDATA_H
#define STRATEGYDATA_H

#include "DataBase/DBInterface.h"
#include "Utility/DataStructDef.h"
#include <QString>
#include <QVector>

namespace HESS
{

class StrategyData:public DBInterface
{
public:
    // ȡ��StrategyData�ľ�̬ʵ��ָ��
    static StrategyData* getStrategyDataPtr();

    // ��������
    virtual ~StrategyData()
    {
        if(m_ptrStrategyData != nullptr)
        {
            delete m_ptrStrategyData;
            m_ptrStrategyData = nullptr;
        }
    }

    // ������Գֲ�Ȩ�ر�
    bool saveStrategyHoldingWeightData(QVector<TStrategyHoldingWeight> *_ptrVStrategyHoldingWeight) const;

private:
    // ���캯��
    StrategyData(QString DBName,QString HostName,QString UserName,QString Password):DBInterface(DBName,HostName,UserName,Password){}

    // Ψһ��StregyData�ľ�̬ʵ��ָ��
    static StrategyData* m_ptrStrategyData;

}; // class StrategyData

} // namespace HESS

#endif // STRATEGYDATA_H
