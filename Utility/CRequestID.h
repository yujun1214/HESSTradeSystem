#ifndef CREQUESTID_H
#define CREQUESTID_H

#include <QMutex>

namespace HESS
{

/* CTP UserTraderApi��requestid�İ�װ�࣬��Ӧ���߳� */
class CRequestID
{
public:
    static CRequestID* getCRequestIDPtr();

    ~CRequestID();

    int getValidRequestID();

    void initRequestID(int _initID);

private:
    CRequestID(int _requestid = 0):m_nRequestID(_requestid){}

    static CRequestID* _ptrCRequestID;

    int m_nRequestID;
    // ������
    QMutex m_IDMutex;

};  // CRequestID

/*
class CRequestID
{
public:
    ~CRequestID(){}

    static int getValidRequestID();

private:
    CRequestID(){}

    static int m_nRequestID;

    static QMutex m_IDMutex;
};
*/
}

#endif // CREQUESTID_H
