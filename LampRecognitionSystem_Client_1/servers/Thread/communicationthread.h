#ifndef COMMUNICATIONTHREAD_H
#define COMMUNICATIONTHREAD_H

#include <QObject>
#include <QThread>

class CommunicationThread : public QThread
{
    Q_OBJECT
public:
    explicit CommunicationThread(QObject *parent = 0);
    virtual void run();
    void stop(int server_index);
public slots:
    void slot_communication(int);
};

#endif // COMMUNICATIONTHREAD_H
