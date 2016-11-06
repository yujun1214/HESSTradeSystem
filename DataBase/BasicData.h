#ifndef BASICDATA_H
#define BASICDATA_H

#include "DataBase/DBInterface.h"
#include "Utility/DataStructDef.h"
#include <QString>
#include <QDate>
#include <QVector>

namespace HESS
{

class BasicData:public DBInterface
{

public:
    // BasicData(QString DBName):DBInterface(DBName){}

    // 取得BasicData的静态实例指针
    static BasicData* getBasicDataPtr();

    // 析构函数
    virtual ~BasicData()
    {
        if(_BasicDataInstancePtr != nullptr)
        {
            delete _BasicDataInstancePtr;
            _BasicDataInstancePtr = nullptr;
        }
    }

    // 取得证券K线行情数据(指定截止日期和天数，含截止日期)
    bool getSecuKMktData(QString SecuCode,QDate tmEndDate,int nDays,QVector<TSecuKMktData> &vKMktData);

    // 取得当天有行情的个股（A股）代码
    bool getTradingSecuCode(QDate tmDate,QVector<QString> &vSecuCode);

private:
    BasicData(QString DBName,QString HostName,QString userName,QString password):DBInterface(DBName,HostName,userName,password){}

    static BasicData* _BasicDataInstancePtr;    // 唯一的BasicData的静态实例指针

};

} // namespace HESS

#endif // BASICDATA_H
