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

// 定义CDerivativeMktDataBuffer的静态实例
CDerivativeMktDataBuffer* CDerivativeMktDataBuffer::m_ptrMktDataBufferInst = nullptr;

// 构造函数
CDerivativeMktDataBuffer::CDerivativeMktDataBuffer()
{
    m_mpMktDataBuffer = QMap<QString,DerivDepthMktDataField*>();
}

// 取得CDerivativeMktDataBuffer的静态实例指针
CDerivativeMktDataBuffer* CDerivativeMktDataBuffer::getMktDataBufferInstPtr()
{
//    QWriteLocker locker(&m_lock);
    if(m_ptrMktDataBufferInst == nullptr)
    {
        m_ptrMktDataBufferInst = new CDerivativeMktDataBuffer();
    }
    return m_ptrMktDataBufferInst;
}

// 析构函数
CDerivativeMktDataBuffer::~CDerivativeMktDataBuffer()
{
    // 释放行情缓冲区
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

// 行情缓冲区初始化
void CDerivativeMktDataBuffer::init(QStringList lstInstrCode)
{
    QWriteLocker locker(&m_lock);

    // 如果行情缓冲区不为空，先释放缓冲区
    if(!m_mpMktDataBuffer.empty())
    {
        for(QMap<QString,DerivDepthMktDataField*>::iterator it = m_mpMktDataBuffer.begin();it != m_mpMktDataBuffer.end();++it)
        {
            delete it.value();
            it.value() = nullptr;
        }
        m_mpMktDataBuffer.clear();
    }

    // 初始化行情缓冲区
    for(QStringList::const_iterator it = lstInstrCode.begin();it != lstInstrCode.end();++it)
    {
        if(!m_mpMktDataBuffer.contains(*it))
            addBuffer(*it);
    }

    // 针对backtest，还应将m_nObserverNumHandledMktData变量设置为0
    m_nObserverNumHandledMktData = 0;
}

// 为指定的衍生品开辟行情缓冲区
void CDerivativeMktDataBuffer::addBuffer(QString strInstrCode)
{
//    QWriteLocker locker(&m_lock);

    // 如果该衍生品已在缓冲区中存在，那么无需新开缓冲区
    if(m_mpMktDataBuffer.contains(strInstrCode))
        return;
    DerivDepthMktDataField* pNewMktDataBuffer = new DerivDepthMktDataField;
    m_mpMktDataBuffer.insert(strInstrCode,pNewMktDataBuffer);
}

// 取消指定衍生品的行情缓冲区
void CDerivativeMktDataBuffer::deleteBuffer(QString strInstrCode)
{
//    QWriteLocker locker(&m_lock);

    if(m_mpMktDataBuffer.contains(strInstrCode))
    {
        delete m_mpMktDataBuffer[strInstrCode];
        m_mpMktDataBuffer.remove(strInstrCode);
    }
}

// 更新深度行情数据
void CDerivativeMktDataBuffer::setDepthMktData(const DerivDepthMktDataField &tMktData)
{
    QWriteLocker locker(&m_lock);

    // 如果该衍生品不在行情缓冲区内，那么先开辟行情缓冲区
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
    // 如果该衍生品不在行情缓冲区内，那么先开辟该衍生品的行情缓冲区
    if(!m_mpMktDataBuffer.contains(strInstrumentID))
        addBuffer(strInstrumentID);

    CopyDerivativeDepthMktDataField(*m_mpMktDataBuffer[strInstrumentID],tMktData);

//    std::cout << "Mkt data updated, code=" << tMktData.InstrumentID << ", last price=" << tMktData.LastPrice << std::endl;
    emit mktDataUpdated(strInstrumentID);

    QTime tmUpdateTime = QTime::fromString(QString(tMktData.UpdateTime),"hh:mm:ss");
    emit sigMktDataUpdateTime(tmUpdateTime);
}

// 取得深度行情数据
const DerivDepthMktDataField& CDerivativeMktDataBuffer::getDepthMktData(QString strInstrCode) const
{
    QReadLocker locker(&m_lock);

    return *m_mpMktDataBuffer[strInstrCode];
}

// 返回simple行情数据
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

// 对行情观察者处理完深度行情后的响应(for backtest)
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
