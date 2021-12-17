#include "ui_system_setting.h"
#include "ui_ui_system_setting.h"
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
extern dbConnect_info_t m_dbconnect_info;
extern QString username_login;
extern QList<client_net_info_t*>TcpSocketList;//管理客户端socket ip port
extern QList<client_all_info_t*> clientList;//管理客户端的列表
extern server_info_t server_info;
extern taskThread_Result_info_t taskResult_info;//管理服务端处理客户端消息的业务逻辑结果

ui_system_setting::ui_system_setting(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ui_system_setting)
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
    ui->setupUi(this);

    qDebug() << "客户端数量::" << clientList.size() <<Qt::endl;
    init_widget(clientList.size());
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(slot_reinit_widget()));
    timer->start(200);

    //密码框限制条件
    QRegExp regx("[a-zA-Z0-9]+$");
    QValidator *validator = new QRegExpValidator(regx, ui->ui_newpassed_edt );
    ui->ui_newpassed_edt->setValidator( validator );
    QValidator *validator1 = new QRegExpValidator(regx, ui->ui_confirm_newpassed_edt );
    ui->ui_confirm_newpassed_edt->setValidator( validator1 );
}

ui_system_setting::~ui_system_setting()
{
    delete ui;
    delete timer;
    timer = NULL;
}

void ui_system_setting::init_widget(int client_num)
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;

    //窗体设置
    ui->ui_system_setting_main_tabwidget->insertTab(0,ui->ui_system_setting_camera_tab,tr("相机参数设置111"));
    ui->ui_system_setting_main_tabwidget->insertTab(1,ui->ui_system_setting_user_tab,tr("用户管理"));
    ui->ui_system_setting_main_tabwidget->insertTab(2,ui->ui_system_setting_server_tab,tr("服务器信息"));
    ui->ui_system_setting_main_tabwidget->insertTab(3,ui->ui_system_setting_upgrade_tab,tr("软件升级"));
    ui->ui_system_setting_main_tabwidget->setCurrentIndex(0);

    //亮度初始化
    ui->ui_brightness_v_sld->setMinimum(-64);
    ui->ui_brightness_v_sld->setMaximum(64);

    //对比度
    ui->ui_contrast_v_sld->setMinimum(0);
    ui->ui_contrast_v_sld->setMaximum(100);

    //饱和度
    ui->ui_saturation_v_sld->setMinimum(0);
    ui->ui_saturation_v_sld->setMaximum(100);

    //锐度(清晰度)
    ui->ui_sharpress_v_sld->setMinimum(0);
    ui->ui_sharpress_v_sld->setMaximum(100);

    //白平衡
    ui->ui_whitebalance_v_sld->setMinimum(2800);
    ui->ui_whitebalance_v_sld->setMaximum(6500);

    //曝光
    ui->ui_exposure_v_sld->setMinimum(300);
    ui->ui_exposure_v_sld->setMaximum(9900);

    //帧率
    ui->ui_fps_v_sld->setMinimum(0);
    ui->ui_fps_v_sld->setSingleStep(5);
    ui->ui_fps_v_sld->setMaximum(30);

    //分辨率
    ui->ui_resolution_v_cmbx->addItem("1920 * 1080");
    ui->ui_resolution_v_cmbx->addItem("1080 * 720");
    ui->ui_resolution_v_cmbx->addItem("800  * 600");
    ui->ui_resolution_v_cmbx->addItem("640  * 480");


    if(clientList.size() == 0) {//没有客户端接过来
        ui->ui_brightness_v_lab->setText(QString::number(BRIGHTNESS_INIT));
        ui->ui_brightness_v_sld->setValue(BRIGHTNESS_INIT);
        ui->ui_contrast_v_lab->setText(QString::number(CONTRAST_INIT));
        ui->ui_contrast_v_sld->setValue(CONTRAST_INIT);
        ui->ui_saturation_v_lab->setText(QString::number(SATURATION_INIT));
        ui->ui_saturation_v_sld->setValue(SATURATION_INIT);
        ui->ui_sharpress_v_lab->setText(QString::number(SHARPRESS_INIT));
        ui->ui_sharpress_v_sld->setValue(SHARPRESS_INIT);
        ui->ui_whitebalance_v_lab->setText(QString::number(WHITEBALANCE_INIT));
        ui->ui_whitebalance_v_sld->setValue(WHITEBALANCE_INIT);
        ui->ui_exposure_v_lab->setText(QString::number(EXPOSURE_INIT));
        ui->ui_exposure_v_sld->setValue(EXPOSURE_INIT);
        ui->ui_fps_v_lab->setText(QString::number(FPS_INIT));
        ui->ui_fps_v_sld->setValue(FPS_INIT);
        ui->ui_resolution_v_cmbx->setCurrentText(RESOLUTION_INIT);
    }
    else {
        ui->ui_system_setting_clientname_cmbx->clear();//先clear
        ui->ui_system_setting_cameraname_cmbx->clear();
        //增加客户端列表和摄像机列表

        for(int i=0;i<client_num;i++) {//客户端索引
            qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
            qDebug() <<"client_all_info["<< i << "].client_info.client_name:" << QString((char*)clientList.at(i)->client_info.client_name) << Qt::endl;
            //增加客户端
            ui->ui_system_setting_clientname_cmbx->addItem(QString((char*)clientList.at(i)->client_info.client_name));
            //qDebug() <<"client_all_info["<< i << "].client_info.camera_num:" << clientList.at(i)->camera_num << Qt::endl;
            //增加相机
            ui->ui_system_setting_cameraname_cmbx->clear();
            for(int j=0;j<clientList.at(i)->camera_num;j++) {//摄像机索引
                ui->ui_system_setting_cameraname_cmbx->addItem(QString((char*)clientList.at(0)->camera_device_info[j].camera_device_name));
                qDebug() <<"clientList.at(client_index)->camera_device_info[count].camera_device_name:" << QString((char*)clientList.at(i)->camera_device_info[j].camera_device_name) << Qt::endl;
          }
        }
        set_camera_params(0,0);
    }



    //设置服务端信息
    ui->ui_system_setting_server_uuid_lab->setText(server_info.server_uuid);
    ui->ui_system_setting_server_name_lab->setText(server_info.server_name);
    ui->ui_system_setting_server_mac_lab->setText(server_info.server_mac);
    ui->ui_system_setting_server_local_ip_lab->setText(server_info.server_ip);
    ui->ui_system_setting_server_foreign_ip_lab->setText(server_info.server_outip);
    ui->ui_system_setting_server_version_lab->setText(server_info.server_version);
}

void ui_system_setting::set_camera_params(int client_index, int camera_index)
{
    ui->ui_brightness_v_lab->setText(QString::number(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_brightness));
    ui->ui_brightness_v_sld->setValue(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_brightness);
    ui->ui_contrast_v_lab->setText(QString::number(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_contrast));
    ui->ui_contrast_v_sld->setValue(clientList.at(client_index)->camera_device_info[client_index].camera_paras_info.camera_ctrl_paras.val_contrast);
    ui->ui_saturation_v_lab->setText(QString::number(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_saturation));
    ui->ui_saturation_v_sld->setValue(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_saturation);
    ui->ui_sharpress_v_lab->setText(QString::number(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_sharpness));
    ui->ui_sharpress_v_sld->setValue(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_sharpness);
    ui->ui_whitebalance_v_lab->setText(QString::number(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_white_balance_temp));
    ui->ui_whitebalance_v_sld->setValue(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_white_balance_temp);
    ui->ui_exposure_v_lab->setText(QString::number(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_exposure));
    ui->ui_exposure_v_sld->setValue(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_exposure);
    ui->ui_fps_v_lab->setText(QString::number(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.fps));
    ui->ui_fps_v_sld->setValue(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.fps);

    if(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.width == 1920)
        ui->ui_resolution_v_cmbx->setCurrentIndex(0);
    else if(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.width == 1080)
         ui->ui_resolution_v_cmbx->setCurrentIndex(1);
    else if(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.width == 800)
         ui->ui_resolution_v_cmbx->setCurrentIndex(2);
    else if(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.width == 640)
        ui->ui_resolution_v_cmbx->setCurrentIndex(3);
}

int ui_system_setting::get_clinet_index()
{
    int client_index = -99;
     for(int i=0;i<clientList.size();i++) {//客户端索引
         if(ui->ui_system_setting_clientname_cmbx->currentText() == QString((char*)clientList.at(i)->client_info.client_name))
             client_index = i;
     }
     return client_index;
}

int ui_system_setting::get_camera_index()
{
    int camera_index = 0;
    for(int i=0;i<clientList.size();i++) {//客户端索引
        for(int j=0;j<clientList.at(i)->camera_num;j++) {//相机索引
            if(ui->ui_system_setting_cameraname_cmbx->currentText() == QString((char*)clientList.at(i)->camera_device_info[j].camera_device_name))
                camera_index = j;
        }
    }
    return camera_index;
}



void ui_system_setting::on_ui_brightness_v_sld_valueChanged(int value)
{
    ui->ui_brightness_v_lab->setText(QString::number(value));
}

void ui_system_setting::on_ui_contrast_v_sld_valueChanged(int value)
{
    ui->ui_contrast_v_lab->setText(QString::number(value));
}

void ui_system_setting::on_ui_saturation_v_sld_valueChanged(int value)
{
    ui->ui_saturation_v_lab->setText(QString::number(value));
}

void ui_system_setting::on_ui_sharpress_v_sld_valueChanged(int value)
{
    ui->ui_sharpress_v_lab->setText(QString::number(value));
}

void ui_system_setting::on_ui_whitebalance_v_sld_valueChanged(int value)
{
    ui->ui_whitebalance_v_lab->setText(QString::number(value));
}

void ui_system_setting::on_ui_exposure_v_sld_valueChanged(int value)
{
    ui->ui_exposure_v_lab->setText(QString::number(value));
}

void ui_system_setting::on_ui_fps_v_sld_valueChanged(int value)
{
     ui->ui_fps_v_lab->setText(QString::number(value));
}

void ui_system_setting::on_ui_resolution_v_cmbx_currentIndexChanged(int index)
{
    qDebug() << "index:" << index <<Qt::endl;
}

void ui_system_setting::on_ui_system_setting_camera_setting_btn_clicked()
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
    if(ui->ui_system_setting_clientname_cmbx->currentText().isEmpty() ||
        ui->ui_system_setting_cameraname_cmbx->currentText().isEmpty()) {
         QMessageBox::information(this,"提示", "客户端和摄像机名不能为空！");
         return;
    }


     //得到客户端，相机索引
     int client_index = get_clinet_index();
     int camera_index = get_camera_index();

    //更新参数数值
     QString value = ui->ui_resolution_v_cmbx->currentText();
     QString width,height;
     int i = value.indexOf(" * ");
     width = value.mid(0,i);
     height = value.mid(i+2,value.size()-i-2);

    qDebug() << "width::" << width << "height::" << height << Qt::endl;

    clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.width = width.toInt();
    clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.height = height.toInt();
    clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.fps = ui->ui_fps_v_sld->value();
    clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_white_balance_temp = ui->ui_whitebalance_v_sld->value();
    clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_brightness = ui->ui_brightness_v_sld->value();
    clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_contrast = ui->ui_contrast_v_sld->value();
    clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_saturation = ui->ui_saturation_v_sld->value();
    clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_sharpness = ui->ui_sharpress_v_sld->value();
    clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_exposure = ui->ui_exposure_v_sld->value();

    //更新数据库
     QSqlQuery query(m_dbconnect_info.dbconn);
     query.exec("BEGIN");
     query.prepare("UPDATE camera_info \
                   set camera_info.width = ?,camera_info.height = ?,camera_info.fps =?,\
                   camera_info.white_balance_temp = ?,camera_info.brightness = ?,camera_info.contrast = ?,\
                   camera_info.saturation = ?,camera_info.sharpness = ?,camera_info.exposure = ?\
                   WHERE camera_info.client_uuid = ? and camera_info.camera_device_name = ?;");

     query.addBindValue(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.width);
     query.addBindValue(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.height);
     query.addBindValue(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.fps);
     query.addBindValue(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_white_balance_temp);
     query.addBindValue(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_brightness);
     query.addBindValue(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_contrast);
     query.addBindValue(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_saturation);
     query.addBindValue(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_sharpness);
     query.addBindValue(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_exposure);
     query.addBindValue(QString((char*)clientList.at(client_index)->client_info.client_uuid));
     query.addBindValue(QString((char*)clientList.at(client_index)->camera_device_info[camera_index].camera_device_name));

     if(query.exec()) {
         query.exec("COMMIT");
         QMessageBox::information(this,"提示", "相机参数数据库更新成功!");
     }
     else {
         qDebug()<< query.lastError() <<Qt::endl;
         QMessageBox::information(this,"提示", "相机参数数据库更新失败!");
         return;
     }

    //组织成json字符串  发送给对应的客户端
    cJSON* root = cJSON_CreateObject();

    cJSON_AddItemToObject(root,"cmd",cJSON_CreateString(CMD_SERVER_PARAS_CHG));
    cJSON_AddItemToObject(root,"server_uuid",cJSON_CreateString(server_info.server_uuid));
    cJSON_AddItemToObject(root,"client_uuid",cJSON_CreateString((char*)clientList.at(client_index)->client_info.client_uuid));
    cJSON_AddItemToObject(root,"camera_name",cJSON_CreateString((char*)clientList.at(client_index)->camera_device_info[camera_index].camera_device_name));
    cJSON_AddItemToObject(root,"width",cJSON_CreateNumber(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.width));
    cJSON_AddItemToObject(root,"height",cJSON_CreateNumber(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.height));
    cJSON_AddItemToObject(root,"fps",cJSON_CreateNumber(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.fps));
    cJSON_AddItemToObject(root,"white_balance_temp",cJSON_CreateNumber(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_white_balance_temp));
    cJSON_AddItemToObject(root,"brightness",cJSON_CreateNumber(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_brightness));
    cJSON_AddItemToObject(root,"contrast",cJSON_CreateNumber(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_contrast));
    cJSON_AddItemToObject(root,"saturation",cJSON_CreateNumber(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_saturation));
    cJSON_AddItemToObject(root,"sharpness",cJSON_CreateNumber(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_sharpness));
    cJSON_AddItemToObject(root,"exposure",cJSON_CreateNumber(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_exposure));
    cJSON_AddItemToObject(root,"video_coding_fmt",cJSON_CreateNumber(clientList.at(client_index)->camera_device_info[camera_index].camera_paras_info.video_coding_fmt));

    char* json_paras_info = cJSON_Print(root);
    qDebug() << "send system data::" << QString(json_paras_info) << Qt::endl;
    TcpSocketList.at(client_index)->TcpClientSocket->write(json_paras_info);
}


void ui_system_setting::on_ui_system_setting_passwdchg_btn_clicked()
{
    //检查是否为空
    if(ui->ui_username_edt->text().isEmpty()) {
        QMessageBox::information(this,"提示", "用户名不能为空！");
         ui->ui_username_edt->setFocus();
        return;
    }

    //检查用户名是否正确
    qDebug() << "登录的用户名:" << username_login;
    if(ui->ui_username_edt->text() != username_login) {
        QMessageBox::information(this,"提示", "用户名与登录用户名不符，请重新输入！");
        ui->ui_username_edt->setText("");
        ui->ui_username_edt->setFocus();
        return;
    }

    //检查密码是否为空
    if(ui->ui_newpassed_edt->text().isEmpty()) {
        QMessageBox::information(this,"提示", "新密码不能为空！");
        ui->ui_newpassed_edt->setFocus();
        ui->ui_newpassed_edt->setText("");
        return;
    }
    else {
        if(ui->ui_confirm_newpassed_edt->text().isEmpty()) {
            QMessageBox::information(this,"提示", "新的确认密码密码不能为空！");
            ui->ui_confirm_newpassed_edt->setFocus();
            ui->ui_confirm_newpassed_edt->setText("");
            return;
        }
        else {
            //检查新密码和确认密码是否相同
            if(ui->ui_newpassed_edt->text() != ui->ui_confirm_newpassed_edt->text()) {
                QMessageBox::information(this,"提示", "新的确认密码与新密码不一致，请重新输入！");
                ui->ui_confirm_newpassed_edt->setFocus();
                ui->ui_confirm_newpassed_edt->setText("");
                return;
            }
            else {
                //更新数据库内用户名的新密码
                QSqlQuery query(m_dbconnect_info.dbconn);
                query.exec("BEGIN");
                query.prepare("UPDATE user_info\
                              SET user_info.user_pwd = ?\
                              WHERE user_info.user_name = ?;");
                query.addBindValue(ui->ui_confirm_newpassed_edt->text().toUInt());
                query.addBindValue(username_login);
                if(query.exec()) {
                    query.exec("COMMIT");
                    QMessageBox::information(this,"提示", "用户密码更新成功!");
                    //删除保存密码的配置
                    taskResult_info.newpwd_flag = true;
                    this->close();
                }
                else {
                    qDebug()<< query.lastError() <<Qt::endl;
                    QMessageBox::information(this,"提示", "用户密码更新失败!");
                    return;
                }


                //输出提示信息
                ui->ui_username_edt->setText("");
                ui->ui_newpassed_edt->setText("");
                ui->ui_confirm_newpassed_edt->setText("");
            }
        }
    }
}

void ui_system_setting::on_ui_system_setting_clientname_cmbx_currentIndexChanged(int index)//表示客户端索引
{
    ui->ui_system_setting_clientname_cmbx->setCurrentIndex(index);
}

void ui_system_setting::on_ui_system_setting_cameraname_cmbx_currentIndexChanged(int index)
{
    ui->ui_system_setting_cameraname_cmbx->setCurrentIndex(index);
}

void ui_system_setting::on_ui_system_setting_clientname_cmbx_currentTextChanged(const QString &arg1)
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
    if(clientList.size() >= 2 && ui->ui_system_setting_clientname_cmbx->count()==clientList.size()) { //有两个客户端及以上
        for(int i=0;i<clientList.size();i++) {//客户端索引
            if(arg1 == QString((char*)clientList.at(i)->client_info.client_name)) {
                ui->ui_system_setting_cameraname_cmbx->clear();
                for(int j=0;j<clientList.at(i)->camera_num;j++) {//相机索引
                    ui->ui_system_setting_cameraname_cmbx->addItem(QString((char*)clientList.at(i)->camera_device_info[j].camera_device_name));
                }//end camera index
            }//end if client name
        }//end client index
    }
}

void ui_system_setting::on_ui_system_setting_cameraname_cmbx_currentTextChanged(const QString &arg1)
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
    set_camera_params(get_clinet_index(),get_camera_index());
}

void ui_system_setting::slot_reinit_widget()
{
    if(taskResult_info.client_cancel_flag == true) {
        qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
        this->close();
    }
}
