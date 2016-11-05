#include "CConfig.h"
#include "CLogSys.h"
#include <stdio.h>
#include <utility>

namespace HESS{
bool CConfig::parsekv(char *szSrc, char *szKey, char *szValue) const
{
    char *sharp;  // ����
    sharp = strchr(szSrc,'#');

    // ���Դ�ַ�����һ���ַ�Ϊ#,��ô����Ϊע����
    if(sharp != NULL && sharp == szSrc)
        return false;

    char *p,*q;
    p = strchr(szSrc,'=');  // p�ҵ��Ⱥ�
    q = strchr(szSrc,'\n'); // q�ҵ����з�

    if(p != NULL && q != NULL)
    {// ����еȺ����л��з�
        *q = '\0';  // �����з�����Ϊ�ַ���������
        strncpy(szKey,szSrc,p - szSrc); // ���Ⱥ�ǰ�����ݿ�����szKey��
        strcpy(szValue,p+1);    // ���Ⱥź�����ݿ�����szValue��
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
    char szbuf[100] = "";    // �����ַ���
    char szKey[100] = "";    // ���ñ�����
    char szValue[100] = "";  // ���ñ���ֵ

    // �������ļ�
    fd = fopen(m_szConfigFileFullName,"r");

    if(fd ==  NULL)
    {
        CLogSys::getLogSysInstance()->setLog(CLogSys::ERR,"CConfig::getConfig(const char*,char*)",
                                             QString::fromStdString("�����ļ���ʧ��,"+std::string(m_szConfigFileFullName)));
        return false;
    }

    // ���ζ�ȡ�ļ���ÿһ��
    while(fgets(szbuf,100,fd))
    {
        // ��ȡ��ֵ��
        if(parsekv(szbuf,szKey,szValue))
        {
            // �����ȡ�ļ�����szKey��ͬ,��ô���ظü���Ӧ��ֵ
            if(strcmp(szKey,_szKey) == 0)
            {
                strcpy(_szValue,szValue);
                return true;
            }
            // ��ն�ȡ������key
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
    char szbuf[100] = "";    // �����ַ���
    char szKey[100] = "";    // ���ñ�����
    char szValue[100] = "";  // ���ñ���ֵ

    // �������ļ�
    fd = fopen(m_szConfigFileFullName,"r");

    if(fd ==  NULL)
    {
        CLogSys::getLogSysInstance()->setLog(CLogSys::ERR,"CConfig::getConfig(const char*,char*)",
                                             QString::fromStdString("�����ļ���ʧ��,"+std::string(m_szConfigFileFullName)));
        return false;
    }

    // ��ռ�ֵmap
    pmpConfig->clear();
    // ���ζ�ȡ�ļ���ÿһ��
    while(fgets(szbuf,100,fd))
    {
        // ��ȡ��ֵ��
        if(parsekv(szbuf,szKey,szValue))
        {
            // �����ֵmap
            std::string strKey(szKey);
            std::string strValue(szValue);
            pmpConfig->insert(std::make_pair(strKey,strValue));

            // ��ն�ȡ������key
            memset(szKey,0,strlen(szKey));
        }
    }
    fclose(fd);
    return true;
}

}

