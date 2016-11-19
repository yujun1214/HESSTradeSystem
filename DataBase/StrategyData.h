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
    // 取得StrategyData的静态实例指针
    static StrategyData* getStrategyDataPtr();

    // 析构函数
    virtual ~StrategyData()
    {
        if(m_ptrStrategyData != nullptr)
        {
            delete m_ptrStrategyData;
            m_ptrStrategyData = nullptr;
        }
    }

    // 保存策略持仓权重表
    bool saveStrategyHoldingWeightData(QVector<TStrategyHoldingWeight> *_ptrVStrategyHoldingWeight) const;

private:
    // 构造函数
    StrategyData(QString DBName,QString HostName,QString UserName,QString Password):DBInterface(DBName,HostName,UserName,Password){}

    // 唯一的StregyData的静态实例指针
    static StrategyData* m_ptrStrategyData;

}; // class StrategyData

} // namespace HESS

#endif // STRATEGYDATA_H
