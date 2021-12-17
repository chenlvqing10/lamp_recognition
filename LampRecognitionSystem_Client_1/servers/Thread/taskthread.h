#ifndef TASKTHREAD_H
#define TASKTHREAD_H

#include <QObject>
#include <QThread>
#include "base/include/thread_base.h"
#include "servers/Thread/connecttoserverthread.h"
#include "servers/Thread/communicationthread.h"
#include "servers/Thread/videocapturethread.h"


class TaskThread : public QThread
{
    Q_OBJECT
public:
    explicit TaskThread(QObject *parent = 0);
    ~TaskThread();
    virtual void run();
    void stop(int server_index);
public slots:
    void slot_TaskThread(taskthread_info_t*);

private:
    ConnectToServerThread* obj_ConnectToServerThread;
    CommunicationThread*   obj_CommunicationThread;
    VideoCaptureThread*    obj_VideoCaptureThread;

signals:
    void sig_connectToServer(int);
    void sig_videoCapture(int);
    void sig_communication(int);
};

#endif // TASKTHREAD_H
