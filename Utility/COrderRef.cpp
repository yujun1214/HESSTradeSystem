#include "COrderRef.h"
#include <QMutexLocker>

namespace HESS
{

COrderRef* COrderRef::_ptrCOrderRef = nullptr;

//COrderRef::COrderRef(char _orderref[])
//{
//    strcpy(m_szOrderRef,_orderref);
//}

COrderRef::~COrderRef()
{
    if(_ptrCOrderRef != nullptr)
    {
        delete _ptrCOrderRef;
        _ptrCOrderRef = nullptr;
    }
}

COrderRef* COrderRef::getCOrderRefPtr()
{
    if(_ptrCOrderRef == nullptr)
    {
        _ptrCOrderRef = new COrderRef();
    }
    return _ptrCOrderRef;
}

int COrderRef::getValidOrderRef()
{
    QMutexLocker locker(&m_RefMutex);
    return ++m_nOrderRef;
}

void COrderRef::initOrderRef(int _initRef)
{
    QMutexLocker locker(&m_RefMutex);
    m_nOrderRef = _initRef;
}

}
