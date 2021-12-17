#ifndef TIMEBASE_H
#define TIMEBASE_H

#include <QObject>

class TimeBase
{
public:
    TimeBase();
    ~TimeBase();
public:
    qint64  getTimestamps();
    int     getTimestamps_ms();
    QString getTimestamps_Qstring();
};

#endif // TIMEBASE_H
