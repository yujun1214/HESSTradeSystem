#ifndef CTIMECONTROLLER_H
#define CTIMECONTROLLER_H

#include <QObject>
#include <QTime>
#include <QReadWriteLock>

namespace HESS
{

// ���ظ�����������ǰʱ���Ӧ�Ľ���״̬�Ļ���
class CTimeController : public QObject
{
    Q_OBJECT
public:
    CTimeController():m_CurrTime(QTime(6,0,0,0)){}
    ~CTimeController(){}

    enum TimeStatus
    {
        // δ����
        PREOPENING = 0,
        // ���̻�����
        OPENINGCUSHION = 1,
        // ��������
        NORMALTRADING = 2,
        // С������
        INTERSETIONCLOSING = 3,
        // �������
        NOONCLOSING = 4,
        // ���̻�����
        PRECLOSINGCUSHION = 5,
        // ����ƽ����
        PRECLOSINGLIQUIDATION = 6,
        // ������
        CLOSED = 7
    };

    virtual TimeStatus CurrTimeStatus() const = 0;

public slots:
    // ���õ�ǰʱ��
    void onSetCurrTime(QTime _currtime);

protected:
    // ��ǰ����ʱ��(for backtest)
    QTime m_CurrTime;
    // read write lock
    mutable QReadWriteLock m_lock;

};

// �н�������ʱ��״̬��
class CCFFEXTimeController : public CTimeController
{
public:
    CCFFEXTimeController(){}
    ~CCFFEXTimeController(){}

    virtual TimeStatus CurrTimeStatus() const;
};

// ҹ�̽��׵�ʱ��״̬��
class CNightTradingTimeController : public CTimeController
{
public:
    CNightTradingTimeController(){}
    ~CNightTradingTimeController(){}

    virtual TimeStatus CurrTimeStatus() const;
};

// ���ڻز��ʱ��״̬��
class CBackTestTimeController : public CTimeController
{
public:
    CBackTestTimeController(){}
    ~CBackTestTimeController(){}

    virtual TimeStatus CurrTimeStatus() const;
};

} // namespace HESS

#endif // CTIMECONTROLLER_H
