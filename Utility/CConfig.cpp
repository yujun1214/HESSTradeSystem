#include "CConfig.h"
#include "CLogSys.h"
#include <stdio.h>
#include <utility>

namespace HESS{
bool CConfig::parsekv(char *szSrc, char *szKey, char *szValue) const
{
    char *sharp;  // 井号
    sharp = strchr(szSrc,'#');

    // 如果源字符串第一个字符为#,那么该行为注释行
    if(sharp != NULL && sharp == szSrc)
        return false;

    char *p,*q;
    p = strchr(szSrc,'=');  // p找到等号
    q = strchr(szSrc,'\n'); // q找到换行符

    if(p != NULL && q != NULL)
    {// 如果有等号且有换行符
        *q = '\0';  // 将换行符设置为字符串结束符
        strncpy(szKey,szSrc,p - szSrc); // 将等号前的内容拷贝到szKey中
        strcpy(szValue,p+1);    // 将等号后的内容拷贝到szValue中
        return true;
    }
    else
    {
        return false;
    }
}

bool CConfig::getConfig(const char *_szKey, char *_szValue) const
{
//    int i;
    FILE *fd;
    char szbuf[100] = "";    // 缓冲字符串
    char szKey[100] = "";    // 配置变量名
    char szValue[100] = "";  // 配置变量值

    // 打开配置文件
    fd = fopen(m_szConfigFileFullName,"r");

    if(fd ==  NULL)
    {
        CLogSys::getLogSysInstance()->setLog(CLogSys::ERR,"CConfig::getConfig(const char*,char*)",
                                             QString::fromStdString("配置文件打开失败,"+std::string(m_szConfigFileFullName)));
        return false;
    }

    // 依次读取文件的每一行
    while(fgets(szbuf,100,fd))
    {
        // 读取键值对
        if(parsekv(szbuf,szKey,szValue))
        {
            // 如果读取的键名与szKey相同,那么返回该键对应的值
            if(strcmp(szKey,_szKey) == 0)
            {
                strcpy(_szValue,szValue);
                return true;
            }
            // 清空读取出来的key
            memset(szKey,0,strlen(szKey));
        }
    }
    fclose(fd);
    return false;
}

bool CConfig::getConfig(std::map<std::string,std::string> *pmpConfig) const
{
//    int i;
    FILE *fd;
    char szbuf[100] = "";    // 缓冲字符串
    char szKey[100] = "";    // 配置变量名
    char szValue[100] = "";  // 配置变量值

    // 打开配置文件
    fd = fopen(m_szConfigFileFullName,"r");

    if(fd ==  NULL)
    {
        CLogSys::getLogSysInstance()->setLog(CLogSys::ERR,"CConfig::getConfig(const char*,char*)",
                                             QString::fromStdString("配置文件打开失败,"+std::string(m_szConfigFileFullName)));
        return false;
    }

    // 清空键值map
    pmpConfig->clear();
    // 依次读取文件的每一行
    while(fgets(szbuf,100,fd))
    {
        // 读取键值对
        if(parsekv(szbuf,szKey,szValue))
        {
            // 插入键值map
            std::string strKey(szKey);
            std::string strValue(szValue);
            pmpConfig->insert(std::make_pair(strKey,strValue));

            // 清空读取出来的key
            memset(szKey,0,strlen(szKey));
        }
    }
    fclose(fd);
    return true;
}

}

