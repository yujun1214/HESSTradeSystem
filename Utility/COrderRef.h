#ifndef CORDERREF_H
#define CORDERREF_H

#include <QMutex>

namespace HESS
{

class COrderRef
{
public:
    static COrderRef* getCOrderRefPtr();

    ~COrderRef();

//    void initOrderRef(char _orderref[]);

//    char* getValidOrderRef();

    int getValidOrderRef();

    void initOrderRef(int _initRef);

private:
//    COrderRef(char _orderref[] = "000000000000");
    COrderRef(int _orderref = 0):m_nOrderRef(_orderref){}

    static COrderRef* _ptrCOrderRef;

//    char m_szOrderRef[13];
    int m_nOrderRef;

    QMutex m_RefMutex;
};

}

#endif // CORDERREF_H
