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

    enum LOGTYPE    // ��־����
    {
        INFO,       // ��Ϣ
        WARNING,    // ����
        ERR         // ����
    };

    // ��ʼ��
    void init() const;

    // д��־
    // type:��־����
    // where:��¼�ڳ����в�����־��λ��
    // what:��־����
    bool setLog(LOGTYPE type,QString where,QString what) const;

public slots:
    // �����־��console
    void toConsole(QString _loginfo) const;
    // ������ڵ��ļ�
    void toFile(QFile *_ptrFile,QString _loginfo) const;

private:
//    QString _logtype;   // ��־����(Info,Err)
//    QString _where;     // ��־λ��
//    QString _what;      // ��־����

    QString _logFileFullPath;   // ��־�ļ�·��(fullname)

    CLogSys();

    static CLogSys* _LogSysInstancePtr;  // Ψһ��̬LogSysָ��

    // ȡ����־�����ı�
    QString LogTypeString(LOGTYPE type) const;
};

} // end of namespace HESS

#endif // LOGSYS_H
