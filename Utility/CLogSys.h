#ifndef CLOGSYS_H
#define CLOGSYS_H

#include <QObject>
#include <QString>
#include <QFile>

namespace HESS{

class CLogSys : public QObject
{
    Q_OBJECT
public:
//    LogSys(){}

    static CLogSys* getLogSysInstance();

    enum LOGTYPE    // 日志类型
    {
        INFO,       // 信息
        WARNING,    // 警告
        ERR         // 错误
    };

    // 初始化
    void init() const;

    // 写日志
    // type:日志类型
    // where:记录在程序中产生日志的位置
    // what:日志内容
    bool setLog(LOGTYPE type,QString where,QString what) const;

public slots:
    // 输出日志到console
    void toConsole(QString _loginfo) const;
    // 输出日期到文件
    void toFile(QFile *_ptrFile,QString _loginfo) const;

private:
//    QString _logtype;   // 日志类型(Info,Err)
//    QString _where;     // 日志位置
//    QString _what;      // 日志内容

    QString _logFileFullPath;   // 日志文件路径(fullname)

    CLogSys();

    static CLogSys* _LogSysInstancePtr;  // 唯一静态LogSys指针

    // 取得日志类型文本
    QString LogTypeString(LOGTYPE type) const;
};

} // end of namespace HESS

#endif // LOGSYS_H
