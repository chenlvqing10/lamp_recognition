#ifndef MYCLIENT_H
#define MYCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QDebug>
#include <QThreadPool>
#include "mytask.h"
#include <QTimer>
#include "sql/sqlbase.h"
#include "base/serverbase.h"

#ifdef __cplusplus
extern "C" {
#include "base/cJSON.h"
}
#endif

class myClient : public QObject
{
    Q_OBJECT
public:
    explicit myClient(QObject *parent = 0);
    ~myClient();
    void SetSocket(int Descriptor);
signals:

public slots:
    //void slot_NewConnected();
    void slot_disconnected();
    void slot_RecieveData();
    void slot_Result(taskThread_Result_info_t* taskresult_info);
    void slot_sendHeart();
private:
    QTcpSocket *tcpSocket;
    QTimer* hearttimer;
};

#endif // MYCLIENT_H
