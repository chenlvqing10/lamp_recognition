#include "ui_image_capture.h"
#include "ui_ui_image_capture.h"
#include <QDebug>
#include "base/eventbase.h"
#include "base/serverbase.h"
#include <QMessageBox>
#include <QTcpSocket>
#include "sql/sqlbase.h"
#include <QSqlError>
#ifdef __cplusplus
extern "C" {
#include "base/cJSON.h"
}
#endif

extern dbConnect_info_t m_dbconnect_info; //数据库信息
extern QList<client_net_info_t*>TcpSocketList;//管理客户端socket ip port
extern QList<client_all_info_t*> clientList;//管理客户端的列表
extern server_info_t server_info;//服务端
extern taskThread_Result_info_t taskResult_info;//管理服务端处理客户端消息的业务逻辑结果

ui_image_capture::ui_image_capture(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ui_image_capture)
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
    ui->setupUi(this);
    qDebug() << "客户端数量::" << clientList.size() <<Qt::endl;
    init_widget(clientList.size());
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(slot_reshow_widget()));
    timer->start(200);
}

ui_image_capture::~ui_image_capture()
{
    delete ui;
    delete timer;
    timer = NULL;
}



void ui_image_capture::init_widget(int client_num)
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
    if(clientList.size() > 0) {
        for(int i=0;i<client_num;i++) {//客户端索引
            qDebug() <<"client_all_info["<< i << "].client_info.client_name:" << QString((char*)clientList.at(i)->client_info.client_name) << Qt::endl;
            //增加客户端
            ui->ui_imgcap_clientname_cmbx->addItem(QString((char*)clientList.at(i)->client_info.client_name));
            //增加相机
            ui->ui_imgcap_cameraname_cmbx->clear();
            for(int j=0;j<clientList.at(i)->camera_num;j++) {//摄像机索引
                ui->ui_imgcap_cameraname_cmbx->addItem(QString((char*)clientList.at(0)->camera_device_info[j].camera_device_name));
          }
        }
    }
}

int ui_image_capture::get_clinet_index()
{
    int client_index = -99;
     for(int i=0;i<clientList.size();i++) {//客户端索引
         if(ui->ui_imgcap_clientname_cmbx->currentText() == QString((char*)clientList.at(i)->client_info.client_name))
             client_index = i;
     }
     return client_index;
}

int ui_image_capture::get_camera_index()
{
    int camera_index = 0;
    for(int i=0;i<clientList.size();i++) {//客户端索引
        for(int j=0;j<clientList.at(i)->camera_num;j++) {//相机索引
            if(ui->ui_imgcap_cameraname_cmbx->currentText() == QString((char*)clientList.at(i)->camera_device_info[j].camera_device_name))
                camera_index = j;
        }
    }
    return camera_index;
}

void ui_image_capture::on_ui_imgcap_clientname_cmbx_currentIndexChanged(int index)
{
    ui->ui_imgcap_clientname_cmbx->setCurrentIndex(index);
}

void ui_image_capture::on_ui_imgcap_cameraname_cmbx_currentIndexChanged(int index)
{
    ui->ui_imgcap_cameraname_cmbx->setCurrentIndex(index);
}

void ui_image_capture::on_ui_imgcap_clientname_cmbx_currentTextChanged(const QString &arg1)
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
    if(clientList.size() >= 2 && ui->ui_imgcap_clientname_cmbx->count()==clientList.size()) { //有两个客户端及以上
        for(int i=0;i<clientList.size();i++) {//客户端索引
            if(arg1 == QString((char*)clientList.at(i)->client_info.client_name)) {
                ui->ui_imgcap_cameraname_cmbx->clear();
                for(int j=0;j<clientList.at(i)->camera_num;j++) {//相机索引
                    ui->ui_imgcap_cameraname_cmbx->addItem(QString((char*)clientList.at(i)->camera_device_info[j].camera_device_name));
                }//end camera index
            }//end if client name
        }//end client index
    }
}
void ui_image_capture::on_ui_imacap_log_btn_clicked()
{
    imgcap_log_obj = new ui_imgcap_log();
    imgcap_log_obj->show();
}

//抓拍图片
void ui_image_capture::on_ui_imgcap_cap_btn_clicked()
{
    //判断是否为空
    if(ui->ui_imgcap_cameraname_cmbx->currentText() == "" || ui->ui_imgcap_clientname_cmbx->currentText() == "") {
        QMessageBox::information(this,"提示", "没有有效的客户端和相机信息!");
        return;
    }
    //得到客户端，相机索引
    int client_index = get_clinet_index();
    int camera_index = get_camera_index();

    //组织成json字符串  发送给对应的客户端
    cJSON* root = cJSON_CreateObject();
    cJSON_AddItemToObject(root,"cmd",cJSON_CreateString(CMD_SERVER_IMG_CAP));
    cJSON_AddItemToObject(root,"server_uuid",cJSON_CreateString(server_info.server_uuid));
    cJSON_AddItemToObject(root,"client_uuid",cJSON_CreateString((char*)clientList.at(client_index)->client_info.client_uuid));
    cJSON_AddItemToObject(root,"camera_name",cJSON_CreateString((char*)clientList.at(client_index)->camera_device_info[camera_index].camera_device_name));

    char* json_imgcap_info = cJSON_Print(root);
    qDebug() << "send system data::" << QString(json_imgcap_info) << Qt::endl;
    TcpSocketList.at(client_index)->TcpClientSocket->write(json_imgcap_info);
}

void ui_image_capture::slot_reshow_widget()
{
    if(taskResult_info.client_cancel_flag == true) {
        qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
        this->close();
    }

    if(taskResult_info.client_show_imgcap == true) {
        taskResult_info.client_show_imgcap= false;
        QPixmap map(taskResult_info.img_cap_path);
        ui->ui_imgcap_show_lab->setPixmap(map);
        ui->ui_imgcap_show_lab->setScaledContents(true);
        ui->ui_imgcap_show_lab->resize(ui->ui_imgcap_show_lab->width(),ui->ui_imgcap_show_lab->height());

    }
}
