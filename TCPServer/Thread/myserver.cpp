#include "myserver.h"
#include <QtDebug>
#include <QHostAddress>
#include "base/serverbase.h"

bool listen_flag = false;
extern taskThread_Result_info_t taskResult_info;    //服务端处理客户端消息的业务逻辑结果
extern QList<client_net_info_t*>TcpSocketList;      //管理客户端socket ip port
extern QList<client_all_info_t*> clientList;        //管理客户端的列表

myServer::myServer(QObject *parent) :
    QTcpServer(parent)
{

}

//监听
void myServer::slot_StartServer(int port)
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;

    if(listen(QHostAddress::Any,port))
        listen_flag = true;
    else
        listen_flag = false;
}

void myServer::slot_StopServer()
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;

    //停止服务端服务
    this->close();

    //断开与客户端的连接
   for(int i=TcpSocketList.size() - 1;i>=0;i--)
        TcpSocketList.at(i)->TcpClientSocket->disconnectFromHost();

    //删除与客户端交互的所有信息
    TcpSocketList.clear();
    clientList.clear();
    //memset(&taskResult_info,0,sizeof(taskThread_Result_info_t));
}

//处理新连接
void myServer::incomingConnection(qintptr handle)
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
    qDebug() << "new tcp client connetc for incomingConnection function" << Qt::endl;
    myClient* myclient = new myClient(this);
    myclient->SetSocket(handle);
}





