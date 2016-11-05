#include <QReadLocker>
#include <QWriteLocker>
#include "CMktDataBuffer.h"
#include <iostream>

namespace HESS
{

// add mkt data observer(for backtest)
void CMktDataBuffer::addObserver()
{
    QWriteLocker locker(&m_lock);
    ++m_nObserverNum;
}

// ����CDerivativeMktDataBuffer�ľ�̬ʵ��
CDerivativeMktDataBuffer* CDerivativeMktDataBuffer::m_ptrMktDataBufferInst = nullptr;

// ���캯��
CDerivativeMktDataBuffer::CDerivativeMktDataBuffer()
{
    m_mpMktDataBuffer = QMap<QString,DerivDepthMktDataField*>();
}

// ȡ��CDerivativeMktDataBuffer�ľ�̬ʵ��ָ��
CDerivativeMktDataBuffer* CDerivativeMktDataBuffer::getMktDataBufferInstPtr()
{
//    QWriteLocker locker(&m_lock);
    if(m_ptrMktDataBufferInst == nullptr)
    {
        m_ptrMktDataBufferInst = new CDerivativeMktDataBuffer();
    }
    return m_ptrMktDataBufferInst;
}

// ��������
CDerivativeMktDataBuffer::~CDerivativeMktDataBuffer()
{
    // �ͷ����黺����
    for(QMap<QString,DerivDepthMktDataField*>::iterator it = m_mpMktDataBuffer.begin();it != m_mpMktDataBuffer.end();++it)
    {
        delete it.value();
        it.value() = nullptr;
    }

    if(m_ptrMktDataBufferInst != nullptr)
    {
        delete m_ptrMktDataBufferInst;
        m_ptrMktDataBufferInst = nullptr;
    }
}

// ���黺������ʼ��
void CDerivativeMktDataBuffer::init(QStringList lstInstrCode)
{
    QWriteLocker locker(&m_lock);

    // ������黺������Ϊ�գ����ͷŻ�����
    if(!m_mpMktDataBuffer.empty())
    {
        for(QMap<QString,DerivDepthMktDataField*>::iterator it = m_mpMktDataBuffer.begin();it != m_mpMktDataBuffer.end();++it)
        {
            delete it.value();
            it.value() = nullptr;
        }
        m_mpMktDataBuffer.clear();
    }

    // ��ʼ�����黺����
    for(QStringList::const_iterator it = lstInstrCode.begin();it != lstInstrCode.end();++it)
    {
        if(!m_mpMktDataBuffer.contains(*it))
            addBuffer(*it);
    }

    // ���backtest����Ӧ��m_nObserverNumHandledMktData��������Ϊ0
    m_nObserverNumHandledMktData = 0;
}

// Ϊָ��������Ʒ�������黺����
void CDerivativeMktDataBuffer::addBuffer(QString strInstrCode)
{
//    QWriteLocker locker(&m_lock);

    // ���������Ʒ���ڻ������д��ڣ���ô�����¿�������
    if(m_mpMktDataBuffer.contains(strInstrCode))
        return;
    DerivDepthMktDataField* pNewMktDataBuffer = new DerivDepthMktDataField;
    m_mpMktDataBuffer.insert(strInstrCode,pNewMktDataBuffer);
}

// ȡ��ָ������Ʒ�����黺����
void CDerivativeMktDataBuffer::deleteBuffer(QString strInstrCode)
{
//    QWriteLocker locker(&m_lock);

    if(m_mpMktDataBuffer.contains(strInstrCode))
    {
        delete m_mpMktDataBuffer[strInstrCode];
        m_mpMktDataBuffer.remove(strInstrCode);
    }
}

// ���������������
void CDerivativeMktDataBuffer::setDepthMktData(const DerivDepthMktDataField &tMktData)
{
    QWriteLocker locker(&m_lock);

    // ���������Ʒ�������黺�����ڣ���ô�ȿ������黺����
    if(!m_mpMktDataBuffer.contains(tMktData.InstrumentID))
        addBuffer(tMktData.InstrumentID);
    CopyDerivativeDepthMktDataField(*m_mpMktDataBuffer[tMktData.InstrumentID],tMktData);

    emit mktDataUpdated(tMktData.InstrumentID);
}

void CDerivativeMktDataBuffer::setDepthMktData(const QList<DerivDepthMktDataField> &lstMktData)
{
    QWriteLocker locker(&m_lock);

    for(QList<DerivDepthMktDataField>::const_iterator it = lstMktData.begin();it != lstMktData.end();++it)
    {
        setDepthMktData(*it);
    }
}

void CDerivativeMktDataBuffer::setDepthMktData(const CThostFtdcDepthMarketDataField &tMktData)
{
    QWriteLocker locker(&m_lock);

    QString strInstrumentID = QString(tMktData.InstrumentID);
    // ���������Ʒ�������黺�����ڣ���ô�ȿ��ٸ�����Ʒ�����黺����
    if(!m_mpMktDataBuffer.contains(strInstrumentID))
        addBuffer(strInstrumentID);

    CopyDerivativeDepthMktDataField(*m_mpMktDataBuffer[strInstrumentID],tMktData);

//    std::cout << "Mkt data updated, code=" << tMktData.InstrumentID << ", last price=" << tMktData.LastPrice << std::endl;
    emit mktDataUpdated(strInstrumentID);

    QTime tmUpdateTime = QTime::fromString(QString(tMktData.UpdateTime),"hh:mm:ss");
    emit sigMktDataUpdateTime(tmUpdateTime);
}

// ȡ�������������
const DerivDepthMktDataField& CDerivativeMktDataBuffer::getDepthMktData(QString strInstrCode) const
{
    QReadLocker locker(&m_lock);

    return *m_mpMktDataBuffer[strInstrCode];
}

// ����simple��������
SimpleDerivMktDataField CDerivativeMktDataBuffer::getSimpleMktData(QString strInstrCode) const
{
    QReadLocker locker(&m_lock);

    const DerivDepthMktDataField* ptrDepthMktData = m_mpMktDataBuffer[strInstrCode];
    SimpleDerivMktDataField tMktData;
    tMktData.InstrumentID = ptrDepthMktData->InstrumentID;
    tMktData.Last = ptrDepthMktData->Last;
    tMktData.Volume = ptrDepthMktData->Volume;
    tMktData.Turnover = ptrDepthMktData->Turnover;
    tMktData.AskPrice = ptrDepthMktData->AskPrice[0];
    tMktData.AskVol = ptrDepthMktData->AskVol[0];
    tMktData.BidPrice = ptrDepthMktData->BidPrice[0];
    tMktData.BidVol = ptrDepthMktData->BidVol[0];
    return tMktData;
}

// ������۲��ߴ����������������Ӧ(for backtest)
void CDerivativeMktDataBuffer::ObserverHandleMktDataDone()
{
    QWriteLocker locker(&m_lock);
    if(++m_nObserverNumHandledMktData >= m_nObserverNum)
    {
        emit sigReadyToReceiveMktData();
        m_nObserverNumHandledMktData = 0;
    }
}

}
