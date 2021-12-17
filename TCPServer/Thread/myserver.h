#ifndef MYSERVER_H
#define MYSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QAbstractSocket>
#include "myclient.h"


class myServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit myServer(QObject *parent = 0);
public slots:
    void slot_StartServer(int port);
    void slot_StopServer();

protected:
    void incomingConnection(qintptr handle); //新连接处理函数
};

#endif // MYSERVER_H
