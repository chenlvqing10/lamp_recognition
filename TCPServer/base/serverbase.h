#ifndef SERVERBASE_H
#define SERVERBASE_H
#include <QString>
#include "base/eventbase.h"

//服务端基础类 用于进行服务端基本操作
class ServerBase
{
public:
    ServerBase();
    ~ServerBase();
public:
    void server_init();
private:
    void set_server_info(server_info_t* server_info,QString path);
    void set_server_info_FileExsisted(server_info_t *server_info,QString path);
private:
    QString getIPPath();
    QString getHostMacAddress();
    QString getOutIPPath();
    QString getVersion();
    QString GetHtml(QString url);
    QString GetNetIP(QString webCode);
};

#endif // SERVERBASE_H
