#ifndef CONNECTTOSERVERTHREAD_H
#define CONNECTTOSERVERTHREAD_H
#include <QObject>
#include <QThread>
#include "protocol/protocol_cjson.h"

class ConnectToServerThread : public QThread
{
    Q_OBJECT
public:
     explicit ConnectToServerThread(QObject *parent = 0);
    ~ConnectToServerThread();
     virtual void run() ;
     void stop(int server_index);

public slots:
     void slot_connectToServer(int);
private:
    PROTOCOL_CJSON* protocol_cjson;
};

#endif // CONNECTTOSERVERTHREAD_H
