#include "BasicData.h"

namespace HESS
{

// ���徲̬ʵ��ָ��
BasicData* BasicData::_BasicDataInstancePtr = nullptr;

// ȡ��BasicData�ľ�̬ʵ��ָ��
BasicData* BasicData::getBasicDataPtr()
{
    if (_BasicDataInstancePtr == nullptr)
    {
        _BasicDataInstancePtr = new BasicData("BasicData","192.168.101.77","root","yujun791025");
    }
    return _BasicDataInstancePtr;
}

} // namespace HESS
