#include "timebase.h"
#include <QDateTime>

TimeBase::TimeBase()
{

}

TimeBase::~TimeBase()
{

}

qint64 TimeBase::getTimestamps()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    qint64 epochTime = dateTime.toMSecsSinceEpoch();

    return epochTime;
}

int TimeBase::getTimestamps_ms()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    int ms = dateTime.time().msec();

    return ms;
}

QString TimeBase::getTimestamps_Qstring()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    QString timestamp = dateTime.toString("yyyy-MM-dd hh:mm:ss.zzz");

    return timestamp;
}
