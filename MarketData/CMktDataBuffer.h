#ifndef CMKTDATABUFFER_H
#define CMKTDATABUFFER_H

#include <QStringList>
#include <QString>
#include <QMap>
#include <QList>
#include <QObject>
#include <QReadWriteLock>
#include <QTime>
#include "Utility/DataStructDef.h"
//#include "MarketData/API/CTP/ThostFtdcMdApi.h"
#include "CTP/inc/ThostFtdcMdApi.h"

namespace HESS
{

// ���ڹ������黺��������(singltonģʽ)
class CMktDataBuffer
{
public:
    // ���캯��
    CMktDataBuffer(){}

    // ��������
    virtual ~CMktDataBuffer(){}

    // ���黺������ʼ��
    virtual void init(QStringList lstInstrCode) = 0;

    // Ϊָ���Ľ��ڹ��߿������黺����
    virtual void addBuffer(QString strInstrCode) = 0;

    // ȡ��ָ�����ڹ��ߵ����黺����
    virtual void deleteBuffer(QString strInstrCode) = 0;

    // ��ӽ��ڹ��ߴ��������뻺����

protected:
    // ���������<HESSCode,CodeTableField>

    // read write lock
    mutable QReadWriteLock m_lock;

};

// �ڻ�&��Ȩ���黺������
class CDerivativeMktDataBuffer : public QObject,public CMktDataBuffer
{
    Q_OBJECT
public:
    // ȡ��CDerivativeMktDataBuffer�ľ�̬ʵ��ָ��
    static CDerivativeMktDataBuffer* getMktDataBufferInstPtr();

    // ��������
    virtual ~CDerivativeMktDataBuffer();

    // ���黺������ʼ��
    virtual void init(QStringList lstInstrCode);

    // Ϊָ��������Ʒ�������黺����
    virtual void addBuffer(QString strInstrCode);

    // ȡ��ָ������Ʒ�����黺����
    virtual void deleteBuffer(QString strInstrCode);

    // ȡ�������������
    const DerivDepthMktDataField& getDepthMktData(QString strInstrCode) const;

    // ȡ��simple��������
    SimpleDerivMktDataField getSimpleMktData(QString strInstrCode) const;

public slots:
    // ���������������
    void setDepthMktData(const DerivDepthMktDataField &tMktData);
    void setDepthMktData(const QList<DerivDepthMktDataField> &lstMktData);

    void setDepthMktData(const CThostFtdcDepthMarketDataField &tMktData);

signals:
    // ����������
    void mktDataUpdated(QString _strSecuCode);

private:
    // ����Ʒ���黺����
    QMap<QString,DerivDepthMktDataField*> m_mpMktDataBuffer;

    // Ψһ��CDerivativeMktDataBuffer��̬ʵ��ָ��
    static CDerivativeMktDataBuffer *m_ptrMktDataBufferInst;

    // ���캯��
    CDerivativeMktDataBuffer();

    // ��������Ƕ�࣬�������ڴ��ͷ�
    class CGarbo
    {
    public:
        ~CGarbo()
        {
            if(CDerivativeMktDataBuffer::m_ptrMktDataBufferInst)
                delete m_ptrMktDataBufferInst;
        }
    };

    static CGarbo m_Garbo;  // ����һ����̬��Ա���������ʱ��ϵͳ���Զ�����������������
};

}

#endif // CMKTDATABUFFER_H
