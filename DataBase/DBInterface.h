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

    // 数据库类型
    enum DBTYPE {QMYSQL = 0,QODBC = 1};
protected:
    // 执行查询
    bool ExecuteQuery(QString strDescription);

protected:
//    QString         _DBName;  // 数据库名称
//    QString         _HostName;  // 主机名称
//    QString         _UserName;  // 用户名
//    QString         _Password;  // 密码
    QSqlDatabase    db;     // 数据库对象
    QSqlQuery       query;  // 数据库查询
    QString         strSql; // sql语句

    DBTYPE          _DatabaseType;  // 数据库类型
};

} // namespace HESS

#endif // DBINTERFACE_H
