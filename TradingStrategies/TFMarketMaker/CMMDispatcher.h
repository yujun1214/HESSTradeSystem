#ifndef CMMDISPATCHER_H
#define CMMDISPATCHER_H

#include <QObject>
#include <QList>
#include <QMutex>
#include <QMutexLocker>
#include <QVector>
#include <QString>
#include <QMap>

namespace HESS
{

// ������ί�м۸�ĵ�����
class CMMDispatcher : public QObject
{
    Q_OBJECT

public:
    // contructor
    CMMDispatcher(QString _code):m_strSecuCode(_code),m_nIndex(0)
    {
        m_lstOrderInterval.clear();
        m_mpBuyOrderPrice.clear();
        m_mpSellCloseOrderPrice.clear();
        m_mpSellOrderPrice.clear();
        m_mpBuyCloseOrderPrice.clear();
    }
    // destructor
    ~CMMDispatcher(){}

    void setPriceTick(int _nTick)
    {
        QMutexLocker locker(&m_lock);
        m_nPriceTick = _nTick;
    }

    void setPricePrecision(int _nPrecision)
    {
        QMutexLocker locker(&m_lock);
        m_nPricePrecision = _nPrecision;
    }

    void addOrderInterval(int _nInterval);

    // �����ȡ���õ����뿪��ί�м۸�
//    bool acquireBuyOrderPrice(double& _fOrderPrice);
    bool acquireBuyOrderPrice(long& _nIndex,double& _fOrderPrice);
    // �����ÿ��õ�����ƽ��ί�м۸�
//    bool acquireSellCloseOrderPrice(double& _fClosePrice);
    bool acquireSellCloseOrderPrice(long& _nIndex,double& _fClosePrice);

    // �����ȡ���õ���������ί�м۸�
//    bool acquireSellOrderPrice(double& _fOrderPrice);
    bool acquireSellOrderPrice(long& _nIndex,double& _fOrderPrice);
    // �����ȡ���õ�����ƽ��ί�м۸�
//    bool acquireBuyCloseOrderPrice(double& _fClosePrice);
    bool acquireBuyCloseOrderPrice(long& _nIndex,double& _fClosePrice);

    // ָ��index��Ӧ�Ŀ���ί�м۸��Ƿ񳬳���Intervals��Χ
    // _longshort = 0:Ϊ���뿪��,_longshort = 1:Ϊ����ί�п���
    // ��������return true, ����return false
    bool exceedIntervalRange(int _longshort,long _index) const;

public slots:
    // delete the buy order price tied with the given index
    void deleteBuyOrderPrice(long _nIndex);

    // delete the sell close order price tied with the given index
    void deleteSellCloseOrderPrice(long _nIndex);

    // delete the sell order price tied with the given index
    void deleteSellOrderPrice(long _nIndex);

    // delete the buy close order price tied with the given index
    void deleteBuyCloseOrderPrice(long _nIndex);

private:
    // ���е��ȶ���Ĵ���
    QString m_strSecuCode;
    // �洢ί�м۸������һ��һ�۶��ٸ�tick��list,����������
    QList<int> m_lstOrderInterval;
    // һ��price tickhֵ
    int m_nPriceTick;
    // ����ľ��ȣ�������ʵ����������������ת��
    int m_nPricePrecision;
    // index
    long m_nIndex;
    // ���뿪��ί�м۸�����
//    QVector<int> m_vBuyOrderPrice;
    QMap<long,double> m_mpBuyOrderPrice;
    // ����ƽ��ί�м۸�����
//    QVector<int> m_vSellCloseOrderPrice;
    QMap<long,double> m_mpSellCloseOrderPrice;
    // ��������ί�м۸�����
//    QVector<int> m_vSellOrderPrice;
    QMap<long,double> m_mpSellOrderPrice;
    // ����ƽ��ί�м۸�����
//    QVector<int> m_vBuyCloseOrderPrice;
    QMap<long,double> m_mpBuyCloseOrderPrice;
    // ������
    mutable QMutex m_lock;

    // ������һ��һ�ۺ�ί�м۸񣬷������ߵľ��루��λ��tick��
    int calcInterval(int _nBidAskPrice,int _nOrderPrice) const;
};

} // HESS

#endif // CMMDISPATCHER_H
