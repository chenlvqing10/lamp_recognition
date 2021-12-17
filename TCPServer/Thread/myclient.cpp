#include "myclient.h"
#include <QList>
#include <QHostAddress>

#include <QMessageBox>
#include "base/eventbase.h"
#include <QDateTime>
#include <QPixmap>
#include <QDir>

//用于一次性读取客户端发送过来的大数据信息
static QString data_hex;
static bool read_flag = false;
static int  data_len = 0;
static int client_index = 0;

extern dbConnect_info_t m_dbconnect_info;
extern taskThread_Result_info_t taskResult_info;    //服务端处理客户端消息的业务逻辑结果
extern QList<client_net_info_t*>TcpSocketList;      //管理客户端socket ip port
extern QList<client_all_info_t*> clientList;        //管理客户端的列表
extern server_info_t server_info;                   //服务端信息

myClient::myClient(QObject *parent) :
    QObject(parent)
{
    //设置线程池最大线程数量 处理器数量
    QThreadPool::globalInstance()->setMaxThreadCount(8);
}

myClient::~myClient()
{

    tcpSocket = NULL;
    delete tcpSocket;

    hearttimer = NULL;
    delete hearttimer;
}

void myClient::SetSocket(int Descriptor)
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
    //qDebug() << "tcpsocket size:::" << TcpSocketList.size();
    tcpSocket = new QTcpSocket(this);//新的套接字
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(slot_RecieveData()));//信号槽函数
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(slot_disconnected()));

    tcpSocket->setSocketDescriptor(Descriptor);//设置socket句柄 加入到客户端列表中


    hearttimer = new QTimer();//启动定时器 发送心跳包
    connect(hearttimer, SIGNAL(timeout()), this, SLOT(slot_sendHeart()));
    hearttimer->start(30000);//30秒

    QString client_ip = tcpSocket->peerAddress ().toString();//获取客户端的IP地址和端口号 将该客户端的在线状态设置为1
    client_ip.replace(":","");
    client_ip.replace("f","");
    int client_port = tcpSocket->peerPort();
    qDebug() << "ip:" << client_ip << "port::" << client_port << Qt::endl;

    client_net_info_t* client_net_info = new client_net_info_t;//赋值
    client_net_info->TcpClientSocket = tcpSocket;
    client_net_info->client_ip = client_ip;
    client_net_info->client_port = client_port;
    TcpSocketList.append(client_net_info);
    //free(client_net_info);//释放内存
    qDebug() << "tcpsocket size:::" << TcpSocketList.size();
    qDebug() << "tcpsocket ip:::"  << TcpSocketList.at(0)->client_ip;
    qDebug() << "tcpsocket port:::"  << TcpSocketList.at(0)->client_port;
}

void myClient::slot_disconnected()
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
    int client_index = 0;
    QString client_uuid = "";

    QTcpSocket *readSocket = qobject_cast<QTcpSocket *>(sender());//得到客户端socket
    qDebug() << "tcpsocket size:::" << TcpSocketList.size();

    if(TcpSocketList.size() > 0) {
         //判断是哪一个客户端  通过ip
         for(int i=0;i<TcpSocketList.size();i++) {
             QString ip = readSocket->peerAddress().toString();
             ip.replace(":","");
             ip.replace("f","");
             if(ip == TcpSocketList.at(i)->client_ip) {
                 client_index = i;
              }
         }
         qDebug() << "客户端索引:" << client_index << Qt::endl;
         //删除TcpSocke
         TcpSocketList.removeAt(client_index);
   }
     //删除客户端信息
     if(clientList.size() > 0) {
        client_uuid = QString((char*)clientList.at(client_index)->client_info.client_uuid)  ;
         //删除客户端列表信息
         clientList.removeAt(client_index);
         //重置客户端界面信息
         taskResult_info.client_cancel_flag = true;

         //更新数据库中客户端在线状态
         sqlBase sqlbase;
         int ret = sqlbase.update_to_client_info(m_dbconnect_info.dbconn,0,client_uuid);
         if(ret > 0) {
             qDebug()<<"client_online 数据库更新成功!"<<Qt::endl;
             //QMessageBox::information(0,"提示", "client_online 数据库更新成功!");
         }
         else {
               qDebug()<<"client_online 数据库更新失败"<<Qt::endl;
             //QMessageBox::information(0,"提示", "client_online 数据库更新失败!");
             return;
        }
     }

      taskResult_info.client_cancel_flag = true;
}


void myClient::slot_RecieveData()
{
   qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
   QTcpSocket *readSocket = qobject_cast<QTcpSocket *>(sender());//得到客户端socket
   bool recieve_end_flag = false;//数据接收结束的标志

   if(read_flag == false) {//开始读取
        qDebug() << "tcpsocket size:::" << TcpSocketList.size();
         //判断是哪一个客户端
         for(int i=0;i<TcpSocketList.size();i++) {
             QString ip = readSocket->peerAddress().toString();
             ip.replace(":","");
             ip.replace("f","");
             if(ip == TcpSocketList.at(i)->client_ip) {
                 client_index = i;
              }
         }
         qDebug() << "客户端索引:" << client_index << Qt::endl;
    }

    qDebug() << "有多少数据可读:"<< readSocket->bytesAvailable() <<Qt::endl;

    if(read_flag == false) {//得到数据大小
        QByteArray ba = readSocket->readAll();
        //解析数据大小  根据协议
        QByteArray ba_copy = ba;
        QString str_len;
        int len;
        int count = 0;
        for(int i=0;i<ba_copy.size();i++) {
            if(ba_copy.at(i) == ':') {
                count = ++i;
                break;
            }
        }
       for(int i = count;i<ba_copy.size();i++){

            if(ba_copy.at(i) == ',')
                break;
            else
                str_len.append(ba_copy.at(i));
        }
        str_len.replace("\t","");
        qDebug() << "str len is :" << str_len ;
        len = str_len.toInt();//总的数据大小
        data_len = len - QString(ba).size();//剩下要读取的数据大小 = 总的数据大小 - 本次读取的数据大小
        qDebug() << "从客户端发送过来的数据总共是" << len << "  本次接收的数据是 " << QString(ba).size() << "  还剩下的数据是" << data_len<<Qt::endl;
        ba_copy.clear();

        if(len <= QString(ba).size() ) {//数据量比较小的情况下  一次接受完全
            //qDebug() << "数据接收结束 小数据" << Qt::endl;
            //qDebug() << "client send data:" <<  ba <<Qt::endl;
            //全局 解析json字符串  传给任务线程去处理
            taskResult_info.current_client = client_index;
            taskResult_info.string_RecivedData = QString(ba);
            recieve_end_flag = true;
        }
        else if(data_len < 10) {
            qDebug() << "客户端数据长度字段错误，请检查客户端json数据" ;
            TcpSocketList.clear();
            return;
        }
        else if(len > QString(ba).size() &&  len != 0){//接着收下一次的数据
            read_flag = true;
            data_hex.append(ba);
        }
        else if(len == 0) {//接受错误
             //QMessageBox::information(this,"提示", "读取错误！");
             qDebug() << "读取错误" << Qt::endl;
             qDebug() << "ba" << QString(ba) << "ba size:" <<ba.length() <<Qt::endl;
             read_flag = false;
             data_hex.clear();
             return;
        }
        else
        {
             qDebug() << "其它情况" << Qt::endl;
             read_flag = false;
             data_hex.clear();
             return;
        }
     }

    if(read_flag == true) {//数据量比较大的情况下
        if(readSocket->bytesAvailable() >= data_len) {//一次性读取剩下的数据 59626 = 2920 + 56706
            QByteArray  baba =  readSocket->read(data_len);
            qDebug() << "baba length:" <<  QString(baba).length() <<Qt::endl;
            data_hex.append(baba);
            QString str(data_hex);
            //qDebug() << "最后几位的字符串:" << str.mid(str.length()-strlen("\"end\":\t1\n}"),strlen("\"end\":\t1\n}"));
            if(str.mid(str.length()-strlen("\"end\":\t1\n}"),strlen("\"end\":\t1\n}")) == "\"end\":\t1\n}" ) {
                read_flag = false;
                qDebug() << "数据接收结束" << Qt::endl;
               // qDebug() << "client send data:" <<  QString(data_hex) <<Qt::endl;
                recieve_end_flag = true;
            }
            else {
                //无效的接收
                qDebug() << "无效数据" <<Qt::endl;
                read_flag = false;
                data_hex.clear();
                return;
            }

            //全局 解析json字符串  传给任务线程去处理
            if(recieve_end_flag == true) {
                taskResult_info.current_client = client_index;
                taskResult_info.string_RecivedData = data_hex;
                data_hex.clear();
            }
        }
   }
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
    qDebug() << "the size of data_hex is: " << taskResult_info.string_RecivedData.length();

    //线程池处理  任务逻辑类
    if(recieve_end_flag == true) { //接收结束
        myTask* mytask = new myTask();
        mytask->setAutoDelete(true);
        connect(mytask, SIGNAL(Result(taskThread_Result_info_t*)), this, SLOT(slot_Result(taskThread_Result_info_t*)),Qt::QueuedConnection);
        QThreadPool::globalInstance()->start(mytask);
    }

}

void myClient::slot_Result(taskThread_Result_info_t *taskresult_info)
{
     qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;

     //与主界面进行交互
     if(taskresult_info->cmd == CMD_CLIENT_CONN) {
        taskresult_info->client_add_flag = true;
        qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
     }

     if(taskresult_info->cmd == CMD_CLIENT_GET_VIDEO)
        taskresult_info->client_show_flag = true;

     if(taskresult_info->cmd == CMD_CLIENT_SERVER_IMG_CAP_ACK)
         taskresult_info->client_show_imgcap = true;


}

void myClient::slot_sendHeart()
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;

    for(int i=0;i<TcpSocketList.size();i++) {
        cJSON* root = cJSON_CreateObject();
        cJSON_AddItemToObject(root,"cmd",cJSON_CreateString(CMD_SERVER_HEART));
        cJSON_AddItemToObject(root,"server_uuid",cJSON_CreateString(server_info.server_uuid));
        QByteArray ba = TcpSocketList.at(i)->client_ip.toLatin1();
        cJSON_AddItemToObject(root,"client_ip",cJSON_CreateString(ba.data()));
        char* json_heart_info = cJSON_Print(root);
        qDebug() << "send system data::" << QString(json_heart_info) << Qt::endl;
        TcpSocketList.at(i)->TcpClientSocket->write(json_heart_info);//循环发送给多个客户端
        cJSON_free(root);
        free(json_heart_info);
    }

}

