#include "communicationthread.h"
#include "base/include/thread_base.h"
#include <QDebug>

static int _index;
extern taskthread_info_t taskthreadInfo[MAXSERVERNUMBER];

CommunicationThread::CommunicationThread(QObject *parent) : QThread(parent)
{

}

void CommunicationThread::run()
{
    QMutex mutex;
    while(taskthreadInfo[_index].communicationThread_runable) {
       mutex.lock();
       taskthreadInfo[_index].communicationThreadID = this->currentThreadId();
       printf("\n-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");
       qDebug() << " start data Communication thread   " << "thread_id::" << this->currentThreadId() << endl;


        printf("\n-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------\n");

       mutex.unlock();
   }

   qDebug() << "end data Communication thread" << endl;
}

void CommunicationThread::stop(int server_index)
{
    taskthreadInfo[server_index].communicationThread_runable = false;
}

void CommunicationThread::slot_communication(int server_index)
{
    _index = server_index;
}
