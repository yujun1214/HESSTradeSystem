#ifndef DBINTERFACE_H
#define DBINTERFACE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>

namespace HESS
{

class DBInterface
{
public:
    DBInterface(QString DBName,QString HostName,QString userName,QString password);
//    ~DBInterface();

    // ���ݿ�����
    enum DBTYPE {QMYSQL = 0,QODBC = 1};
protected:
    // ִ�в�ѯ
    bool ExecuteQuery(QString strDescription);

protected:
//    QString         _DBName;  // ���ݿ�����
//    QString         _HostName;  // ��������
//    QString         _UserName;  // �û���
//    QString         _Password;  // ����
    QSqlDatabase    db;     // ���ݿ����
    QSqlQuery       query;  // ���ݿ��ѯ
    QString         strSql; // sql���

    DBTYPE          _DatabaseType;  // ���ݿ�����
};

} // namespace HESS

#endif // DBINTERFACE_H
