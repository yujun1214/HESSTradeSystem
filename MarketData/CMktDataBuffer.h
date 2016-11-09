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

// 金融工具行情缓冲区基类(singlton模式)
class CMktDataBuffer
{
public:
    // 构造函数
    CMktDataBuffer(){}

    // 析构函数
    virtual ~CMktDataBuffer(){}

    // 行情缓冲区初始化
    virtual void init(QStringList lstInstrCode) = 0;

    // 为指定的金融工具开辟行情缓冲区
    virtual void addBuffer(QString strInstrCode) = 0;

    // 取消指定金融工具的行情缓冲区
    virtual void deleteBuffer(QString strInstrCode) = 0;

    // 添加金融工具代码至代码缓冲区

protected:
    // 代码表缓冲区<HESSCode,CodeTableField>

    // read write lock
    mutable QReadWriteLock m_lock;

};

// 期货&期权行情缓冲区类
class CDerivativeMktDataBuffer : public QObject,public CMktDataBuffer
{
    Q_OBJECT
public:
    // 取得CDerivativeMktDataBuffer的静态实例指针
    static CDerivativeMktDataBuffer* getMktDataBufferInstPtr();

    // 析构函数
    virtual ~CDerivativeMktDataBuffer();

    // 行情缓冲区初始化
    virtual void init(QStringList lstInstrCode);

    // 为指定的衍生品开辟行情缓冲区
    virtual void addBuffer(QString strInstrCode);

    // 取消指定衍生品的行情缓冲区
    virtual void deleteBuffer(QString strInstrCode);

    // 取得深度行情数据
    const DerivDepthMktDataField& getDepthMktData(QString strInstrCode) const;

    // 取得simple行情数据
    SimpleDerivMktDataField getSimpleMktData(QString strInstrCode) const;

public slots:
    // 更新深度行情数据
    void setDepthMktData(const DerivDepthMktDataField &tMktData);
    void setDepthMktData(const QList<DerivDepthMktDataField> &lstMktData);

    void setDepthMktData(const CThostFtdcDepthMarketDataField &tMktData);

signals:
    // 行情更新完毕
    void mktDataUpdated(QString _strSecuCode);

private:
    // 衍生品行情缓冲区
    QMap<QString,DerivDepthMktDataField*> m_mpMktDataBuffer;

    // 唯一的CDerivativeMktDataBuffer静态实例指针
    static CDerivativeMktDataBuffer *m_ptrMktDataBufferInst;

    // 构造函数
    CDerivativeMktDataBuffer();

    // 单例的内嵌类，处理单例内存释放
    class CGarbo
    {
    public:
        ~CGarbo()
        {
            if(CDerivativeMktDataBuffer::m_ptrMktDataBufferInst)
                delete m_ptrMktDataBufferInst;
        }
    };

    static CGarbo m_Garbo;  // 定义一个静态成员，程序结束时，系统会自动调用它的析构函数
};

}

#endif // CMKTDATABUFFER_H
