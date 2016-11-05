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
    CMktDataBuffer():m_nObserverNumHandledMktData(0){}

    // ��������
    virtual ~CMktDataBuffer(){}

    // ���黺������ʼ��
    virtual void init(QStringList lstInstrCode) = 0;

    // Ϊָ���Ľ��ڹ��߿������黺����
    virtual void addBuffer(QString strInstrCode) = 0;

    // ȡ��ָ�����ڹ��ߵ����黺����
    virtual void deleteBuffer(QString strInstrCode) = 0;

    // ��ӽ��ڹ��ߴ��������뻺����

    // add mkt data observer(for backtest)
    void addObserver();

protected:
    // ���������<HESSCode,CodeTableField>

    // read write lock
    mutable QReadWriteLock m_lock;

    // mkt data observer number(for backtest)
    int m_nObserverNum;
    // num of observer whick has handled the mkt data
    int m_nObserverNumHandledMktData;

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

    // slots for backtest
    // ������۲��ߴ����������������Ӧ
    void ObserverHandleMktDataDone();

signals:
    // ����������
    void mktDataUpdated(QString _strSecuCode);

    // ����������µ�ʱ��(for backtest)
    void sigMktDataUpdateTime(QTime _updatetime);

    // signals for backtest
    // signal of ready to receive mkt data
    void sigReadyToReceiveMktData();

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
