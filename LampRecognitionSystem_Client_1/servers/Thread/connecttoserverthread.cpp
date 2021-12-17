#include "connecttoserverthread.h"
#include "base/include/thread_base.h"
#include <QDebug>

#ifdef __cplusplus
extern "C" {
#include "servers/serial/ec20_serial.h"
#include "servers/camera/v4l2_camera.h"
}
#endif




static int _index;
extern taskthread_info_t taskthreadInfo[MAXSERVERNUMBER];//任务线程信息  一个任务线程代表操作一路socket与服务器交互
extern int fd_ec20;
extern client_all_info_t client_all_info;

ConnectToServerThread::ConnectToServerThread(QObject *parent) : QThread(parent)
{
    protocol_cjson = new PROTOCOL_CJSON();
}

ConnectToServerThread::~ConnectToServerThread()
{
    delete protocol_cjson;
}

void ConnectToServerThread::run()
{
    QMutex mutex;
    while(taskthreadInfo[_index].connecttoserverThread_runable) {
       qDebug() << " start data ConnectToServer thread   " << "thread_id::" << this->currentThreadId() << endl;
       mutex.lock();
       char recv_buff[1000] = {0};
       char send_buff[1000] = {0};
       int timeout_ms = 20000;
       int ret;
       taskthreadInfo[_index].connecttoserverThreadID = this->currentThreadId();

       if(!strcmp((char*)taskthreadInfo[_index].at_qiopen_info.service_type,SERVICE_TYPE_TCP)) {
          memset(recv_buff,0,sizeof(recv_buff));
          sprintf(send_buff,"AT+QIOPEN=%d,%d,\"%s\",\"%s\",%d,0,%d\r\n",taskthreadInfo[_index].at_qiopen_info.context_id,taskthreadInfo[_index].at_qiopen_info.connectID, \
                 taskthreadInfo[_index].at_qiopen_info.service_type,taskthreadInfo[_index].at_qiopen_info.server_ip,taskthreadInfo[_index].at_qiopen_info.server_port,\
                  taskthreadInfo[_index].at_qiopen_info.access_mode);

          AT_Data_Send_Recv(fd_ec20,send_buff,strlen(send_buff),recv_buff,sizeof(recv_buff),timeout_ms);

          if(strlen(recv_buff) == strlen(AT_COMMAND_RESPOND_OK)) {
              memset(recv_buff,0,sizeof(recv_buff));
              if(serial_data_available(fd_ec20,timeout_ms)) {//have a data
                  ret= serial_receive(fd_ec20,recv_buff,sizeof(recv_buff));
                  if(ret < 0) {
                      printf("recv failed\n");
                      taskthreadInfo[_index].connect_status = CONNECT_STATUS_FAILED;
                  }
                  else {
                      printf("recv again recv_buff = %s len = %ld\n",recv_buff,strlen(recv_buff));
                      if(!strcmp(recv_buff,AT_QIOPEN_OK)) {
                          printf("recf qiopen ok\n");
                           taskthreadInfo[_index].connect_status = CONNECT_STATUS_SUCCESS;
                      }
                      else {
                          printf("recf qiopen nok\n");
                           taskthreadInfo[_index].connect_status = CONNECT_STATUS_FAILED;
                      }
                  }
              }
          }
          else {
             taskthreadInfo[_index].connect_status = CONNECT_STATUS_FAILED;
             printf("recf buffer is not compare\n");
          }


          if(taskthreadInfo[_index].connect_status == CONNECT_STATUS_SUCCESS) {
              client_all_info.client_info.client_online = ONLINE_STATUS_ON;

              //构造json类型的客户端信息
              protocol_cjson->create_clientinfo_json(&client_all_info);
              taskthreadInfo[_index].taskthread_waitconditon.wakeAll();//连接成功唤醒线程
          }

       mutex.unlock();
    }//end tcp if
   }//end while

   qDebug() << "end ConnectToServerthread" << endl;
}

void ConnectToServerThread::stop(int server_index)
{
    qDebug() << " ConnectToServerThread 线程关闭函数 " << endl;
    taskthreadInfo[server_index].connecttoserverThread_runable = false;
    taskthreadInfo[server_index].taskthread_waitconditon.wakeAll();
}

void ConnectToServerThread::slot_connectToServer(int index)
{
    _index = index;
}
