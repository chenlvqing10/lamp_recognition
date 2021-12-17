#include "ui_lamp_mainwindow.h"
#include "ui_ui_lamp_mainwindow.h"
#include <QMessageBox>
#include <QtDebug>
#include <QString>
#include <QByteArray>
#include <QDir>
#include <QSettings>
#include <QUuid>
#include <QDateTime>


#ifdef __cplusplus
extern "C" {
#include "servers/camera/v4l2_camera.h"
#include "base/include/delay.h"
#include "servers/serial/ec20_serial.h"
}
#endif


extern client_all_info client_all_info;//客户端信息
extern taskthread_info_t taskthreadInfo[MAXSERVERNUMBER];
extern int fd_ec20;
extern int cam_FdList[MAXVIDEONUM];

ui_lamp_mainWindow::ui_lamp_mainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ui_lamp_mainWindow)
{
    ui->setupUi(this);

    //配置客户端信息
    QString fileName = QDir::currentPath();
    fileName += "/config/client.ini";
    qDebug() << "fileName:" << fileName << endl;
    QFileInfo fileinfo(fileName);

    //如果已经配置了客户端西信息 则从客户端配置文件中读取相关的信息
    if(fileinfo.isFile())
        get_client_info(fileName);
     else//第一次启动客户端的时候自动生成客户端配置信息
        set_client_info(fileName);
    client_all_info.client_info.client_online = ONLINE_STATUS_OFF;//在线状态初始化
    

    //初始化串口
    int ret =serial_init(EC20_SERIAL_NAME,BAUD_RATE_115200,DATA_BITS_8,PARITY_NONE,STOP_BIT_1,FLOW_CONTROL_NONE);
    if(ret < 0) {
        QMessageBox::information(0,"提示", "4G模块串口初始化失败！");
        return;
    }
    else
        QMessageBox::information(0,"提示", "4G模块串口初始化成功！");

    qDebug() << "fd_ec20::" << fd_ec20 << endl;//串口文件描述符
/*
    //初始化4G模块 激活移动网络 等待连接
    ret = AT_Command_int(fd_ec20);
    if(ret == AT_COMMAND_ERROR) {
         QMessageBox::information(0,"提示", "4G模块AT指令错误！");
         return;
    }
    if(ret == AT_NO_SIM) {
         QMessageBox::information(0,"提示", "4G模块未检出SIM卡！");
         return;
    }
    if(ret == AT_NO_NETWORK) {
         QMessageBox::information(0,"提示", "4G模块未检出网络注册信息！");
         return;
    }
    if(ret == AT_SET_CONTEXT_ERROR || ret == AT_ACTIVE_CONTEXT_ERROR) {
         QMessageBox::information(0,"提示", "4G模块激活移动场景失败！");
         return;
    }
    qDebug() << "4G模块移动场景激活成功" << endl;
*/

    //

    //初始化摄像头设备 获取连接客户端的所有摄像头设备信息 获取指示灯设备信息 等待获取图像/视频信息
    ret = v4l2_init_camera();
    if(ret < 0)
        printf("init all v4l2 camera failed\n");
     else
        printf("init all v4l2 camera successed\n");

    //申请内核缓冲帧空间 便映射到用户空间 放入视频采集队列准备开启视频采集
    for(int i=0;i<(int)client_all_info.camera_num;i++) {
        ret = v4l2_camera_reqbuff(cam_FdList[i],i,5);//5
        if(ret < 0) {
            printf("request buffer error\n");
        }
   }
   qDebug() << "摄像头初始化成功" << endl;

}


ui_lamp_mainWindow::~ui_lamp_mainWindow()
{
    delete ui;
    serial_close(fd_ec20);
}


void ui_lamp_mainWindow::on_ui_lamp_connect_btn1_clicked()
{ 
    if(ui->ui_lamp_connect_btn1->text() == "连接") {
        //创建任务线程对象
        taskThread1  = new TaskThread();
        connect(this,SIGNAL(sig_TaskThread(taskthread_info_t*)),taskThread1,SLOT(slot_TaskThread()));

        //构造参数结构体
        QByteArray bi;
        bi = ui->ui_lamp_server_ip1->text().toLatin1();
        strcpy((char*)taskthreadInfo[connectID_0].at_qiopen_info.server_ip,bi.data());
        taskthreadInfo[connectID_0].at_qiopen_info.server_port = ui->ui_lamp_server_port1->text().toShort();
        strcpy((char*)taskthreadInfo[connectID_0].at_qiopen_info.service_type,SERVICE_TYPE_TCP);
        taskthreadInfo[connectID_0].at_qiopen_info.connectID = connectID_0;
        taskthreadInfo[connectID_0].at_qiopen_info.context_id = CONTEXTID;
        taskthreadInfo[connectID_0]. at_qiopen_info.access_mode = BUFFER_ACCESS_MODE;

        //得到房间信息和变电站信息
        get_room_info();

        //发送信号 传递参数
        emit sig_TaskThread(&taskthreadInfo[connectID_0]);

        //启动线程
        taskThread1->start();

        ui->ui_lamp_connect_btn1->setText("断开");
    }
    else {
        taskThread1->stop(0);
        delayms(1000);//延时等待线程结束 控制调用顺序
        delete  taskThread1;
        ui->ui_lamp_connect_btn1->setText("连接");
    }
}


void ui_lamp_mainWindow::on_ui_lamp_connect_btn2_clicked()
{
    qDebug() << "thread 2" << endl;
}


void ui_lamp_mainWindow::on_ui_lamp_connect_btn3_clicked()
{

}


void ui_lamp_mainWindow::on_ui_lamp_connect_btn4_clicked()
{

}


void ui_lamp_mainWindow::on_ui_lamp_connect_btn5_clicked()
{

}


void ui_lamp_mainWindow::on_ui_lamp_connect_btn6_clicked()
{

}


void ui_lamp_mainWindow::on_ui_lamp_connect_btn7_clicked()
{

}


void ui_lamp_mainWindow::on_ui_lamp_connect_btn8_clicked()
{

}


void ui_lamp_mainWindow::on_ui_lamp_connect_btn9_clicked()
{

}


void ui_lamp_mainWindow::on_ui_lamp_connect_btn10_clicked()
{

}


void ui_lamp_mainWindow::on_ui_lamp_connect_btn11_clicked()
{

}


void ui_lamp_mainWindow::on_ui_lamp_connect_btn12_clicked()
{

}

void ui_lamp_mainWindow::set_client_info(QString path)
{
        QUuid id = QUuid::createUuid();
        QString strId = id.toString();
        strId.replace("{","");
        QByteArray ba = strId.toLatin1();
        strcpy((char*)client_all_info.client_info.client_uuid,ba.data());

        strcpy((char*)client_all_info.client_info.client_name,CLIENTNAME);
        client_all_info.client_info.client_online = ONLINE_STATUS_OFF;

        QString version = getVersion();
        ba = version.toLatin1();
        strcpy((char*)client_all_info.client_info.client_version,ba.data());

        QSettings scfg(path, QSettings::IniFormat);
        scfg.setIniCodec("UTF8");

        scfg.beginGroup("ClientInfo");
        scfg.setValue("client_uuid",QString((char*)client_all_info.client_info.client_uuid));
        scfg.setValue("client_name",QString((char*)client_all_info.client_info.client_name));
        scfg.setValue("client_version",QString((char*)client_all_info.client_info.client_version));

        scfg.endGroup();

}

void ui_lamp_mainWindow::get_client_info(QString path)
{
    //从配置文件中读取客户端的固定信息
    qDebug()<< "client configure file is exsisted" << endl;
    QSettings scfg(path, QSettings::IniFormat);
    scfg.setIniCodec("UTF8");
    scfg.beginGroup("ClientInfo");
    QString uuid = scfg.value("client_uuid").toString();
    QString name = scfg.value("client_name").toString();
    QString version = scfg.value("client_version").toString();
    QByteArray ba = uuid.toLatin1();
    strcpy((char*)client_all_info.client_info.client_uuid,ba.data());
    ba = name.toLatin1();
    strcpy((char*)client_all_info.client_info.client_name,ba.data());
    ba = version.toLatin1();
    strcpy((char*)client_all_info.client_info.client_version,ba.data());
    scfg.endGroup();
}

void ui_lamp_mainWindow::get_room_info()
{
    QByteArray bi = ui->ui_lamp_room_name_edt->text().toLatin1();
    strcpy((char*)client_all_info.room_info.room_name,bi.data());
    qDebug() << "room name" <<client_all_info.room_info.room_name <<endl;
    bi = ui->ui_lamp_transfm_substation_name_edt->text().toLatin1();
    strcpy((char*)client_all_info.room_info.substation_name,bi.data());
    qDebug() << "substation name" <<client_all_info.room_info.substation_name <<endl;
}

QString ui_lamp_mainWindow::getVersion()
{
    QString currentTime = QDateTime::currentDateTime().toString("yyyy_MMdd_hhmmss");
    //qDebug() << "currentTime" <<currentTime << Qt::endl;
    QString version = QString("%1%2").arg("v_client").arg(currentTime);

    return version;
}
