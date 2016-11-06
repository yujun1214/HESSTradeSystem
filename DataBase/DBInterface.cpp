#include "dbinterface.h"
#include <qsqlerror.h>
#include <QDebug>

namespace HESS
{

//QSqlDatabase DBInterface::db = QSqlDatabase();

DBInterface::DBInterface(QString DBName, QString HostName, QString userName, QString password)
{
    db = QSqlDatabase::addDatabase("QMYSQL",DBName);
    if(db.isValid())
    {
        db.setHostName("localhost");
        db.setDatabaseName(DBName);
        //db.setUserName("root");
        //db.setPassword("yujun791025");
        db.setUserName(userName);
        db.setPassword(password);
        try
        {
            if(!db.open())
            {
                //QMessageBox::critical(0,QObject::tr("Database Error"),db.lastError().text());
                return;
            }
            query = QSqlQuery(db);
        }
        catch(QString exception)
        {
            //QMessageBox::critical(0,Cstr("数据库连接失败!"),exception);
        }
        _DatabaseType = QMYSQL;
    }
    else
    {
        db.close();
        db = QSqlDatabase::addDatabase("QODBC",DBName);
        db.setHostName(HostName);
        db.setDatabaseName(DBName);
        //db.setUserName("sa");
        //db.setPassword("sa");
        db.setUserName(userName);
        db.setPassword(password);
        try
        {
            if(!db.open())
            {
                return;
            }
            query = QSqlQuery(db);
        }
        catch(QString exception)
        {

        }
        _DatabaseType = QODBC;
    }
}

//DBInterface::~DBInterface()
//{
//    if(db.isOpen())
//        db.close();
//}

bool DBInterface::ExecuteQuery(QString strDescription)
{
    query.clear();
    //qDebug() << "strSql:" << strSql << "\n";
    query.exec(strSql);
    if(!query.isActive())
    {
        QMessageBox::critical(0,strDescription,query.lastError().text());
        return false;
    }
    return true;
}

} // namespace HESS
