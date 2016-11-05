#include "CTimeController.h"
#include <QTime>
#include <QReadLocker>
#include <QWriteLocker>

namespace HESS
{

void CTimeController::onSetCurrTime(QTime _currtime)
{
    QWriteLocker locker(&m_lock);
    m_CurrTime = _currtime;
}

CTimeController::TimeStatus CCFFEXTimeController::CurrTimeStatus() const
{
    QTime tmCurrTime = QTime::currentTime();
    if(tmCurrTime <= QTime::fromString("09:15:00","hh:mm:ss"))
    {
        return CTimeController::PREOPENING;
    }
    else if(tmCurrTime <= QTime::fromString("09:16:00","hh:mm:ss"))
    {
        return CTimeController::OPENINGCUSHION;
    }
    else if(tmCurrTime <= QTime::fromString("11:30:00","hh:mm:ss"))
    {
        return CTimeController::NORMALTRADING;
    }
    else if(tmCurrTime <= QTime::fromString("13:00:00","hh:mm:ss"))
    {
        return CTimeController::NOONCLOSING;
    }
    else if(tmCurrTime <= QTime::fromString("15:13:00","hh:mm:ss"))
    {
        return CTimeController::NORMALTRADING;
    }
    else if(tmCurrTime <= QTime::fromString("15:13:30","hh:mm:ss"))
    {
        return CTimeController::PRECLOSINGCUSHION;
    }
    else if(tmCurrTime <= QTime::fromString("15:13:50","hh:mm:ss"))
    {
        return CTimeController::PRECLOSINGLIQUIDATION;
    }
    else
    {
        return CTimeController::CLOSED;
    }
}

CTimeController::TimeStatus CNightTradingTimeController::CurrTimeStatus() const
{
    QTime tmCurrTime = QTime::currentTime();
    if(tmCurrTime <= QTime::fromString("21:00:00","hh:mm:ss"))
    {
        return CTimeController::PREOPENING;
    }
    else if(tmCurrTime <= QTime::fromString("21:01:00","hh:mm:ss"))
    {
        return CTimeController::OPENINGCUSHION;
    }
    else if(tmCurrTime <= QTime::fromString("23:28:00","hh:mm:ss"))
    {
        return CTimeController::NORMALTRADING;
    }
    else if(tmCurrTime <= QTime::fromString("23:28:30","hh:mm:ss"))
    {
        return CTimeController::PRECLOSINGCUSHION;
    }
    else if(tmCurrTime <= QTime::fromString("23:29:00","hh:mm:ss"))
    {
        return CTimeController::PRECLOSINGLIQUIDATION;
    }
    else
    {
        return CTimeController::CLOSED;
    }
}

CTimeController::TimeStatus CBackTestTimeController::CurrTimeStatus() const
{
    QReadLocker locker(&m_lock);
//    if(m_CurrTime <= QTime::fromString("09:15:00","hh:mm:ss"))
    if(m_CurrTime <= QTime(9,15,0,0))
    {
        return CTimeController::PREOPENING;
    }
//    else if(m_CurrTime <= QTime::fromString("09:16:00","hh:mm:ss"))
    else if(m_CurrTime <= QTime(9,16,0,0))
    {
        return CTimeController::OPENINGCUSHION;
    }
//    else if(m_CurrTime <= QTime::fromString("11:30:00","hh:mm:ss"))
    else if(m_CurrTime <= QTime(11,30,0,0))
    {
        return CTimeController::NORMALTRADING;
    }
//    else if(m_CurrTime <= QTime::fromString("13:00:00","hh:mm:ss"))
    else if(m_CurrTime <= QTime(13,0,0,0))
    {
        return CTimeController::NOONCLOSING;
    }
//    else if(m_CurrTime <= QTime::fromString("15:13:00","hh:mm:ss"))
    else if(m_CurrTime <= QTime(15,13,0,0))
    {
        return CTimeController::NORMALTRADING;
    }
//    else if(m_CurrTime <= QTime::fromString("15:13:30","hh:mm:ss"))
    else if(m_CurrTime <= QTime(15,13,30,0))
    {
        return CTimeController::PRECLOSINGCUSHION;
    }
//    else if(m_CurrTime <= QTime::fromString("15:13:50","hh:mm:ss"))
    else if(m_CurrTime <= QTime(15,13,50,0))
    {
        return CTimeController::PRECLOSINGLIQUIDATION;
    }
    else
    {
        return CTimeController::CLOSED;
    }
}

}
