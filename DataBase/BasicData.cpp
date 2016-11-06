#include "BasicData.h"

namespace HESS
{

// 定义静态实例指针
BasicData* BasicData::_BasicDataInstancePtr = nullptr;

// 取得BasicData的静态实例指针
BasicData* BasicData::getBasicDataPtr()
{
    if (_BasicDataInstancePtr == nullptr)
    {
        _BasicDataInstancePtr = new BasicData("BasicData","192.168.101.77","root","yujun791025");
    }
    return _BasicDataInstancePtr;
}

} // namespace HESS
