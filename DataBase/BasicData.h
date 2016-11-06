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

    // ȡ��BasicData�ľ�̬ʵ��ָ��
    static BasicData* getBasicDataPtr();

    // ��������
    virtual ~BasicData()
    {
        if(_BasicDataInstancePtr != nullptr)
        {
            delete _BasicDataInstancePtr;
            _BasicDataInstancePtr = nullptr;
        }
    }

    // ȡ��֤ȯK����������(ָ����ֹ���ں�����������ֹ����)
    bool getSecuKMktData(QString SecuCode,QDate tmEndDate,int nDays,QVector<TSecuKMktData> &vKMktData);

    // ȡ�õ���������ĸ��ɣ�A�ɣ�����
    bool getTradingSecuCode(QDate tmDate,QVector<QString> &vSecuCode);

private:
    BasicData(QString DBName,QString HostName,QString userName,QString password):DBInterface(DBName,HostName,userName,password){}

    static BasicData* _BasicDataInstancePtr;    // Ψһ��BasicData�ľ�̬ʵ��ָ��

};

} // namespace HESS

#endif // BASICDATA_H
