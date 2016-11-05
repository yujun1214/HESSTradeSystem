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
     * @brief 读取指定键的值
     * @param[in] _szKey 指定的键名
     * @param[out] _szValue 与给定键名对应的值
     * @return true:读取成功,false:读取失败
     * @detail
     * @see getConfig
     */
    bool getConfig(const char* _szKey,char* _szValue) const;

    /**
     * @brief 读取配置文件中所有的配置数据
     * @param[out] 配置文件的键值map
     * @return true:读取成功,false:读取失败
     * @detail
     * @see getConfig
     */
    bool getConfig(std::map<std::string,std::string> *pmpConfig) const;

protected:

private:
    char m_szConfigFileFullName[256];

    /**
     * @brief 解析字符串中以等号分开的键值対
     * @param[int] szStr 源字符串
     * @param[out] szKey 键
     * @param[out] szValue 值
     * @return true:解析成功,false:解析失败
     * @detail
     * @see parsekv
     */
    bool parsekv(char *szSrc,char *szKey,char *szValue) const;
};

}

#endif // CCONFIG_H
