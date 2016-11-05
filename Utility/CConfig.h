#ifndef CCONFIG_H
#define CCONFIG_H

#include <string>
#include <string.h>
#include <map>

namespace HESS
{

class CConfig
{
public:
    CConfig(const char* szConfigFileFullName)
    {
        strncpy(m_szConfigFileFullName,szConfigFileFullName,sizeof(m_szConfigFileFullName)-1);
    }

    ~CConfig(){}

    /**
     * @brief ��ȡָ������ֵ
     * @param[in] _szKey ָ���ļ���
     * @param[out] _szValue �����������Ӧ��ֵ
     * @return true:��ȡ�ɹ�,false:��ȡʧ��
     * @detail
     * @see getConfig
     */
    bool getConfig(const char* _szKey,char* _szValue) const;

    /**
     * @brief ��ȡ�����ļ������е���������
     * @param[out] �����ļ��ļ�ֵmap
     * @return true:��ȡ�ɹ�,false:��ȡʧ��
     * @detail
     * @see getConfig
     */
    bool getConfig(std::map<std::string,std::string> *pmpConfig) const;

protected:

private:
    char m_szConfigFileFullName[256];

    /**
     * @brief �����ַ������ԵȺŷֿ��ļ�ֵ��
     * @param[int] szStr Դ�ַ���
     * @param[out] szKey ��
     * @param[out] szValue ֵ
     * @return true:�����ɹ�,false:����ʧ��
     * @detail
     * @see parsekv
     */
    bool parsekv(char *szSrc,char *szKey,char *szValue) const;
};

}

#endif // CCONFIG_H
