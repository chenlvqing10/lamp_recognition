#include "taskthread.h"
#include <QDebug>


static int _index;
extern taskthread_info_t taskthreadInfo[MAXSERVERNUMBER];//任务线程信息  一个任务线程代表操作一路socket与服务器交互

TaskThread::TaskThread(QObject *parent) : QThread(parent)
{
}

TaskThread::~TaskThread()
{
    
    
}

void TaskThread::run()
{   
    qDebug()<<tr("主线程id:")<<QThread::currentThreadId();
    taskthreadInfo[_index].taskThreadID = this->currentThreadId();//任务线程ID

    //根据线程ID获取服务器索引
    int server_index = -99;
    for(int i=0;i<MAXSERVERNUMBER;i++)
    {
        if(this->currentThreadId() == taskthreadInfo[i].taskThreadID) {
            server_index = i;
            break;
        }
    }

    //创建新的线程对象
    obj_ConnectToServerThread = new ConnectToServerThread();
    obj_VideoCaptureThread    = new VideoCaptureThread();
    obj_CommunicationThread   = new CommunicationThread();

    //使能任务线程和连接线程
    taskthreadInfo[server_index].taskThread_runable = true;
    taskthreadInfo[server_index].connecttoserverThread_runable = true;

    //信号槽 参数传递
    connect(this,SIGNAL(sig_connectToServer(int)),obj_ConnectToServerThread,SLOT(slot_connectToServer(int)));

   while(taskthreadInfo[server_index].taskThread_runable) {
       qDebug()<< "start main task thread for thread id【" << this->currentThreadId() << "】" << endl;

      taskthreadInfo[server_index].taskthread_mutex.lock();//锁住
      //启动连接服务器线程去连接服务器 如果没有连接成功 线程则阻塞在这里
      emit sig_connectToServer(server_index);
      obj_ConnectToServerThread->start();
      taskthreadInfo[server_index].taskthread_waitconditon.wait(&taskthreadInfo[server_index].taskthread_mutex);

      //退出线程 解锁
      if(taskthreadInfo[server_index].taskThread_runable == false){
          taskthreadInfo[server_index].taskthread_mutex.unlock();
        break;
      }
      taskthreadInfo[server_index].connecttoserverThread_runable = false;//连接成功后退出线程
      taskthreadInfo[server_index].taskthread_mutex.unlock();

      sleep(100);


      //连接成功之后执行接下来的线程
      taskthreadInfo[server_index].videocaptureThread_runable = true;
      taskthreadInfo[server_index].communicationThread_runable = true;
      connect(this,SIGNAL(sig_videoCapture(int)),obj_VideoCaptureThread,SLOT(slot_videoCapture(int)));
      connect(this,SIGNAL(sig_communication(int)),obj_CommunicationThread,SLOT(slot_communication(int)));

      while (1) {
          taskthreadInfo[server_index].taskthread_mutex.lock();
          emit sig_videoCapture(server_index);
          obj_VideoCaptureThread->start();


         // emit sig_communication(server_index);
          //obj_CommunicationThread->start();

          if(!taskthreadInfo[server_index].taskThread_runable) {
               taskthreadInfo[server_index].taskthread_mutex.unlock();
               break;
          }
          taskthreadInfo[server_index].taskthread_mutex.unlock();
      }//end while(1)
   }//end while(taskthreadInfo[server_index].taskThread_runable)
    
  qDebug() << "  end ConnectThread thread " << endl;
}

void TaskThread::stop(int server_index)
{
    taskthreadInfo[server_index].taskThread_runable = false;

    if(taskthreadInfo[server_index].connecttoserverThread_runable)
        obj_ConnectToServerThread->stop(server_index);
    if(taskthreadInfo[server_index].videocaptureThread_runable)
        obj_VideoCaptureThread->stop(server_index);
    if(taskthreadInfo[server_index].communicationThread_runable)
        obj_CommunicationThread->stop(server_index);
}

void TaskThread::slot_TaskThread(taskthread_info_t* taskthread_info)
{
   _index = taskthread_info->at_qiopen_info.connectID;
}
