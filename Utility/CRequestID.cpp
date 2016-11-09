#include "CRequestID.h"
#include <QMutexLocker>

namespace HESS
{

CRequestID* CRequestID::_ptrCRequestID = nullptr;

CRequestID::~CRequestID()
{
    if(_ptrCRequestID != nullptr)
    {
        delete _ptrCRequestID;
        _ptrCRequestID = nullptr;
    }
}

CRequestID* CRequestID::getCRequestIDPtr()
{
    if(_ptrCRequestID == nullptr)
    {
        _ptrCRequestID = new CRequestID();
    }
    return _ptrCRequestID;
}

int CRequestID::getValidRequestID()
{
    QMutexLocker locker(&m_IDMutex);
    return ++m_nRequestID;
}

void CRequestID::initRequestID(int _initID)
{
    QMutexLocker locker(&m_IDMutex);
    m_nRequestID = _initID;
}

/*
int CRequestID::m_nRequestID = 0;
QMutex CRequestID::m_IDMutex = QMutex();

int CRequestID::getValidRequestID()
{
    QMutexLocker locker(&m_IDMutex);
    return ++m_nRequestID;
}
*/
}
