#include "serverbase.h"
#include <QUuid>
#include <QByteArray>
#include <QDir>
#include <QSettings>
#include <QDebug>
#include <QFileInfo>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QHostInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDateTime>
#include <QEventLoop>
#include <QMessageBox>
#include "sql/sqlbase.h"

extern dbConnect_info_t m_dbconnect_info;       //数据库信息
QList<client_all_info_t*> clientList;           //管理客户端的列表
QList<client_net_info_t*>TcpSocketList;         //管理客户端socket ip port
server_info_t server_info;                      //管理服务端信息
taskThread_Result_info_t taskResult_info;       //管理客户端和服务端通信交互完成后的结果处理
cap_record_info_t cap_record_info;              //抓拍记录 用于数据库                    //登录用户修改了密码后需要重新登录系统

ServerBase::ServerBase()
{
    memset(&server_info,0,sizeof(server_info_t));
}

ServerBase::~ServerBase()
{
}

void ServerBase::server_init()
{
    sqlBase sqlBase_obj;
    QString fileName = QDir::currentPath();
    fileName += "/config/system.ini";
    qDebug() << "fileName:" << fileName << Qt::endl;
    QFileInfo fileinfo(fileName);
    if(fileinfo.isFile()) {
        qDebug()<< "server configure file is exsisted";

        //改变可变的服务端配置  ip地址 外网地址 端口号
        set_server_info_FileExsisted(&server_info,fileName);

        //更新数据库服务器信息表
        int ret = sqlBase_obj.update_to_server_info(m_dbconnect_info.dbconn,&server_info);
        if(ret > 0) {
            QMessageBox::information(0,"提示", "server_info数据库更新成功!");
        }
        else {
            QMessageBox::information(0,"提示", "server_info数据库更新失败!");
            return;
        }

    }
    else {
        set_server_info(&server_info,fileName);
        //先清空以前的数据
        QString sql = "DELETE  FROM server_info;";
        int ret = sqlBase_obj.exec_sql(m_dbconnect_info.dbconn,sql);
        if(ret > 0) {
            QMessageBox::information(0,"提示", "server_info数据库清空成功!");
        }
        else {
            QMessageBox::information(0,"提示", "server_info数据库 清空失败!");
            return;
        }
        //保存到数据库服务器信息表中
        ret = sqlBase_obj.insert_to_server_info(m_dbconnect_info.dbconn,&server_info);
        if(ret > 0) {
            QMessageBox::information(0,"提示", "server_info数据库插入成功!");
        }
        else {
            QMessageBox::information(0,"提示", "server_info数据库插入失败!");
            return;
        }


    }

}

void ServerBase::set_server_info(server_info_t *server_info,QString path)
{
    QUuid id = QUuid::createUuid();
    QString strId = id.toString();
    strId.replace("{","");
    strId.replace("}","");
    QByteArray ba = strId.toLatin1();
    strcpy(server_info->server_uuid,ba.data());

    QString ip = getIPPath();
    ba = ip.toLatin1();
    strcpy(server_info->server_ip,ba.data());

    QString mac = getHostMacAddress();
    ba = mac.toLatin1();
    strcpy(server_info->server_mac,ba.data());

    QString outip = getOutIPPath();
    ba = outip.toLatin1();
    strcpy(server_info->server_outip,ba.data());

    //server_info->server_online = ONLINE_STATUS_ON;//服务器启动之后表示在线
    server_info->server_port = 0;//初始值先设置为0 后面在主页面的时候进行更新
    strcpy(server_info->server_name,SERVERNAME);

    QString version = getVersion();
    ba = version.toLatin1();
    strcpy(server_info->server_version,ba.data());


//    qDebug() << "server_uuid::" <<  server_info->server_uuid << Qt::endl;
//    qDebug() << "server_name::" <<  server_info->server_name << Qt::endl;
//    qDebug() << "server_mac::" <<  server_info->server_mac << Qt::endl;
//    qDebug() << "server_ip::" <<  server_info->server_ip << Qt::endl;
//    qDebug() << "server_outip::" <<  server_info->server_outip << Qt::endl;
//    qDebug() << "server_version::" <<  server_info->server_version << Qt::endl;



    QSettings scfg(path, QSettings::IniFormat);
    scfg.setIniCodec("UTF8");

    scfg.beginGroup("ServerInfo");
    scfg.setValue("server_uuid",server_info->server_uuid);
    scfg.setValue("server_name",server_info->server_name);
    scfg.setValue("server_mac",server_info->server_mac);
    scfg.setValue("server_ip",server_info->server_ip);
    scfg.setValue("server_outip",server_info->server_outip);
    scfg.setValue("server_port",server_info->server_port);
    scfg.setValue("server_version",server_info->server_version);

    scfg.endGroup();

}

void ServerBase::set_server_info_FileExsisted(server_info_t *server_info, QString path)
{
    QByteArray ba;
    QString ip = getIPPath();
    ba = ip.toLatin1();
    strcpy(server_info->server_ip,ba.data());

    QString mac = getHostMacAddress();
    ba = mac.toLatin1();
    strcpy(server_info->server_mac,ba.data());

    QString outip = getOutIPPath();
    ba = outip.toLatin1();
    strcpy(server_info->server_outip,ba.data());

    QSettings scfg(path, QSettings::IniFormat);
    scfg.setIniCodec("UTF8");

    scfg.beginGroup("ServerInfo");
    scfg.setValue("server_mac",server_info->server_mac);
    scfg.setValue("server_ip",server_info->server_ip);
    scfg.setValue("server_outip",server_info->server_outip);
    ba = scfg.value("server_uuid").toString().toLatin1();
    strcpy(server_info->server_uuid,ba.data());
    ba = scfg.value("server_name").toString().toLatin1();
    strcpy(server_info->server_name,ba.data());
    server_info->server_port = scfg.value("server_port").toInt();
    ba = scfg.value("server_version").toString().toLatin1();
    strcpy(server_info->server_version,ba.data());

    scfg.endGroup();
}


QString ServerBase::getIPPath()
{
    QString hostAddr = "";
    QList<QNetworkInterface> nets = QNetworkInterface::allInterfaces();// 获取所有网络接口列表
    int nCnt = nets.count();

    //排除虚拟机地址
    for(int i = 0; i < nCnt; i ++) {
        if (  nets[i].hardwareAddress() == "00:50:56:C0:00:01" ) {
            nets.removeAt( i );
            break;
        }
    }
    nCnt = nets.count();
    for(int i = 0; i < nCnt; i ++) {
        if (  nets[i].hardwareAddress() == "00:50:56:C0:00:08" ) {
            nets.removeAt( i );
            break;
        }
    }

    foreach(QNetworkInterface interface,nets) {
        //排除不在活动的IP
        if( interface.flags().testFlag(QNetworkInterface::IsUp) && interface.flags().testFlag(QNetworkInterface::IsRunning ) ){
            QList<QNetworkAddressEntry> entryList = interface.addressEntries();
            foreach(QNetworkAddressEntry entry,entryList) {
                QHostAddress hostAddress = entry.ip();
                if ( hostAddr.isEmpty() ) {
                    //排除本地地址
                    if ( hostAddress != QHostAddress::LocalHost && hostAddress.toIPv4Address() ) {
                        quint32 nIPV4 = hostAddress.toIPv4Address();

                        //本地链路地址
                        quint32 nMinRange = QHostAddress("169.254.1.0").toIPv4Address();
                        quint32 nMaxRange = QHostAddress("169.254.254.255").toIPv4Address();
                        //排除链路地址
                        if ( ( nIPV4 >= nMinRange ) && ( nIPV4 <= nMaxRange ) )
                            continue;
                        //qDebug() << hostAddress;
                        hostAddr = hostAddress.toString();
                        return hostAddr;
                    }
                }

            }
        }
    }
     return "null";
}

QString ServerBase::getHostMacAddress()
{
    qDebug() << "getHostMacAddress";
    QList<QNetworkInterface> nets = QNetworkInterface::allInterfaces();// 获取所有网络接口列表
    int nCnt = nets.count();

    QString strMacAddr = "";
    for(int i = 0; i < nCnt; i ++)
    {
        //00:50:56:C0:00:01 00:50:56:C0:00:08虚拟机地址
        // 如果此网络接口被激活并且正在运行并且不是回环地址，则就是我们需要找的Mac地址
        if(nets[i].flags().testFlag(QNetworkInterface::IsUp) && nets[i].flags().testFlag(QNetworkInterface::IsRunning)
                && !nets[i].flags().testFlag(QNetworkInterface::IsLoopBack) &&  nets[i].hardwareAddress() != "00:50:56:C0:00:01" && nets[i].hardwareAddress() != "00:50:56:C0:00:08" )
        {
            strMacAddr = nets[i].hardwareAddress();
            break;
        }
    }
    return strMacAddr;
}

QString ServerBase::getOutIPPath()
{
      return GetNetIP(GetHtml("https://www.taobao.com/help/getip.php"));
}

QString ServerBase::getVersion()
{
    QString currentTime = QDateTime::currentDateTime().toString("yyyy_MMdd_hhmmss");
    //qDebug() << "currentTime" <<currentTime << Qt::endl;
    QString version = QString("%1%2").arg("v_server").arg(currentTime);

    return version;
}

//外网的获取方法，通过爬网页来获取外网IP
QString ServerBase::GetHtml(QString url)//网页源代码
{
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    QNetworkReply *reply = manager->get(QNetworkRequest(QUrl(url)));
    QByteArray responseData;
    QEventLoop eventLoop;
    QObject::connect(manager, SIGNAL(finished(QNetworkReply *)), &eventLoop, SLOT(quit()));
    eventLoop.exec();
    responseData = reply->readAll();
    //qDebug() << "responseData::" << QString(responseData);
    return QString(responseData);
}

QString ServerBase::GetNetIP(QString webCode)
{
    if(webCode == "") {
        return "0.0.0.0";
    }
    QString web = webCode.replace(" ", "");
    qDebug() << "web::" << web;
    web = web.replace("\r", "");
    web = web.replace("\n", "");
    web = web.replace("\"", "");
    web = web.replace("(", "");
    web = web.replace(")","");
    web = web.replace("{", "");
    web = web.replace("}", "");
    web = web.replace("ipCallback","");
    web = web.replace("ip","");
    web = web.replace(":", "");

    if(web.size() > 100 || web.size() < 6) {
        return "0.0.0.0";
    }

    return web;
}
