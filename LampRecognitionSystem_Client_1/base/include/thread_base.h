#ifndef THREAD_BASE_H
#define THREAD_BASE_H
#include <QWaitCondition>
#include <QMutex>

#ifdef __cplusplus
extern "C" {
#include "servers/serial/ec20_serial.h"
}
#endif


#define CONNECTTOSERVER_FAILED   false
#define CONNECTTOSERVER_SUCCESS  true

typedef struct taskthread_info{
    QMutex          taskthread_mutex;                   //任务线程互斥锁
    QWaitCondition  taskthread_waitconditon;            //任务线程条件变量
    at_qiopen_t     at_qiopen_info;                     //TCP/UDP协议一路socket信息
    bool            taskThread_runable;                 //任务线程运行状态
    bool            connecttoserverThread_runable;      //连接服务器线程运行状态
    bool            communicationThread_runable;        //数据交互线程运行状态
    bool            videocaptureThread_runable;         //视频捕获线程运行状态
    void*           taskThreadID;                       //taskThread线程ID
    void*           connecttoserverThreadID;            //子线程线程ID(连接到服务器)
    void*           communicationThreadID;              //子线程线程ID(数据交互)
    void*           videocaptureThreadID;               //子线程线程ID(视频数据交互)
    bool            connect_status;
}taskthread_info_t ;


#define MAXSERVERNUMBER 12

#endif // THREAD_BASE_H
