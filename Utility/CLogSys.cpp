#include "CLogSys.h"
#include <QFile>
#include <QIODevice>
#include <QTextStream>
#include <QDateTime>
#include <iostream>
#include <QTextStream>

namespace HESS{

CLogSys* CLogSys::_LogSysInstancePtr = NULL;

CLogSys::CLogSys()
{
//#if defined(Q_OS_MAC)
//    _logFileFullPath = "/users/davidyujun/share/QuantStrategy/log.txt";
//#elif defined(Q_OS_WIN32)
//    _logFileFullPath = "D:/share/QuantStrategy/log.txt";
//#endif
}

CLogSys* CLogSys::getLogSysInstance()
{
    if(_LogSysInstancePtr == NULL)
    {
        _LogSysInstancePtr = new CLogSys();
    }
    return _LogSysInstancePtr;
}

QString CLogSys::LogTypeString(LOGTYPE type) const
{
    QString strLogType;
    switch(type)
    {
    case INFO:
        strLogType = "Info";
        break;
    case WARNING:
        strLogType = "Warning";
        break;
    case ERR:
        strLogType = "Error";
        break;
    default:
        strLogType = "";
        break;
    }
    return strLogType;
}

void CLogSys::init() const
{
    QFile logFile("log.txt");
    if(!logFile.open(QIODevice::Append))
        return;
    QTextStream out(&logFile);
    out << "[" << QDateTime::currentDateTime().toString("yyyy.MM.dd:HH:mm:ss") << "] Initializing...\n";
    out.flush();
    logFile.close();
}

bool CLogSys::setLog(LOGTYPE type, QString where, QString what) const
{
//    QFile logFile(_logFileFullPath);
    QFile logFile("log.txt");
    if(!logFile.open(QIODevice::Append))
    {
//        QMessageBox::critical(NULL,"Writing Log File","Could not open file "+logFile.fileName());
        return false;
    }
    QString strLogType = LogTypeString(type);
    QTextStream out(&logFile);
    out << "[" << QDateTime::currentDateTime().toString("yyyy.MM.dd:HH:mm:ss") << "][" << strLogType << "][" << where << "]" << what << "\r\n";
    out.flush();
    logFile.close();
    return true;
}

void CLogSys::toConsole(QString _loginfo) const
{
    if(_loginfo.length() > 0)
    {
        std::cout << _loginfo.toStdString() << std::endl;
    }
}

void CLogSys::toFile(QFile *_ptrFile, QString _loginfo) const
{
    if(_loginfo.length() <= 0)
        return;
    if(_ptrFile == nullptr)
    {
        toConsole("Error,Did not specified.");
        return;
    }
    if(!_ptrFile->isOpen())
    {
        if(!_ptrFile->open(QIODevice::Append | QIODevice::Text))
        {
            toConsole("Error, Could not open file" + _ptrFile->fileName());
            return;
        }
    }
    QTextStream out(_ptrFile);
    out << _loginfo << "\r\n";
    out.flush();
}

} // end of namespace HESS

