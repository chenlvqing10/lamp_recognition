#include "sqlbase.h"
#include <QMessageBox>
#include <QDebug>
#include <QSqlError>

dbConnect_info_t m_dbconnect_info;
system_log_info_t m_system_log_info;

sqlBase::sqlBase()
{

}

sqlBase::~sqlBase()
{
    //delete m_dbconnect_info;
}

bool sqlBase::createConnect()
{
    m_dbconnect_info.hostName = "localhost";
    m_dbconnect_info.dbName   = "lamp_recognition";
    m_dbconnect_info.userName = "root";
    m_dbconnect_info.password = "123456";
    m_dbconnect_info.dbconn = QSqlDatabase::addDatabase("QMYSQL");
    m_dbconnect_info.dbconn.setHostName(m_dbconnect_info.hostName);
    m_dbconnect_info.dbconn.setDatabaseName(m_dbconnect_info.dbName);
    m_dbconnect_info.dbconn.setUserName(m_dbconnect_info.userName);
    m_dbconnect_info.dbconn.setPassword(m_dbconnect_info.password);
    if (!m_dbconnect_info.dbconn.open()) {
         //QMessageBox::information(0,"提示", "数据库连接失败!！");
        return false;
    }
    else {
        return true;
    }
}

int sqlBase::get_maxid(QSqlDatabase dbconn, QString& sql)
{
    int max_id = 0;
    QSqlQuery query(dbconn);
    query.prepare(sql);
    if(query.exec()) {
        while(query.next()) {
            max_id = query.value(0).toInt();
        }
    }

    if(max_id == 0)
        max_id = 1000;

    query.clear();

    return max_id;
}

QString sqlBase::get_userid(QSqlDatabase dbconn, QString &username)
{
    QString userid;
    QSqlQuery query(dbconn);
    query.prepare("SELECT user_info.user_id FROM user_info WHERE user_info.user_name = ?;");
    query.addBindValue(username);

    if(query.exec()) {
        while(query.next()) {
            userid = query.value(0).toString();
        }
    }
    query.clear();

    return userid;
}

int sqlBase::get_userrole(QSqlDatabase dbconn, QString &username)
{
    int user_role = -99;
    QSqlQuery query(dbconn);
    query.prepare("SELECT user_info.user_role FROM user_info WHERE user_info.user_name = ?;");
    query.addBindValue(username);

    if(query.exec()) {
        while(query.next()) {
            user_role = query.value(0).toInt();
        }
    }
    query.clear();

    return user_role;
}


int sqlBase::get_number_of_imgcap(QSqlDatabase dbconn, QString& client_uuid,QString& camera_device_name)
{
    int  number_imgcap = 0;
    QSqlQuery query(dbconn);
    query.prepare("SELECT COUNT(cap_record_info.cap_record_no) FROM cap_record_info \
                  WHERE cap_record_info.client_uuid = ? and cap_record_info.camera_device_name = ?;");
    query.addBindValue(client_uuid);
    query.addBindValue(camera_device_name);

    if(query.exec()) {
        while(query.next()) {
            number_imgcap = query.value(0).toInt();
        }
    }
    query.clear();

    return number_imgcap;
}

bool sqlBase::is_client_in_database(QSqlDatabase dbconn,  QString& client_uuid)
{
    int  num_record = 0;
    bool ret = false;
    QSqlQuery query(dbconn);
    query.prepare("SELECT COUNT(client_info.client_uuid) FROM client_info WHERE client_info.client_uuid = ?;");
    query.addBindValue(client_uuid);

    if(query.exec()) {
        while(query.next()) {
            num_record = query.value(0).toInt();
        }
    }
    query.clear();

    if(num_record == 0)
       ret = false;
    else if(num_record == 1)
       ret = true;

    return ret;
}

bool sqlBase::is_roominfo_in_database(QSqlDatabase dbconn, QString &room_name, QString &substation_name)
{
    int  num_record = 0;
    bool ret = false;
    QSqlQuery query(dbconn);
    query.prepare("SELECT COUNT(room_info.room_id) FROM room_info \
                   WHERE room_info.room_name = ? and room_info.substation_name = ?;");
    query.addBindValue(room_name);
    query.addBindValue(substation_name);

    if(query.exec()) {
        while(query.next()) {
            num_record = query.value(0).toInt();
        }
    }
    query.clear();

    if(num_record == 0)
       ret = false;
    else if(num_record == 1)
       ret = true;

    return ret;
}

bool sqlBase::is_camerainfo_in_database(QSqlDatabase dbconn, QString& client_uuid, QString &camera_device_name)
{
    int  num_record = 0;
    bool ret = false;
    QSqlQuery query(dbconn);
    query.prepare("SELECT COUNT(camera_info.camera_id) FROM camera_info \
                   WHERE camera_info.camera_device_name = ? AND camera_info.client_uuid = ?;");

    query.addBindValue(camera_device_name);
    query.addBindValue(client_uuid);

    if(query.exec()) {
        while(query.next()) {
            num_record = query.value(0).toInt();
        }
    }
    query.clear();

    if(num_record == 0)
       ret = false;
    else if(num_record == 1)
       ret = true;

    return ret;
}

bool sqlBase::is_lampinfo_in_database(QSqlDatabase dbconn, QString& client_uuid, QString &camera_device_name)
{
    int  num_record = 0;
    bool ret = false;
    QSqlQuery query(dbconn);
    query.prepare("SELECT COUNT(lamp_info.lamp_id) FROM lamp_info \
                   WHERE lamp_info.camera_id = \
                   (SELECT camera_id FROM camera_info \
                        WHERE camera_info.camera_device_name = ? AND camera_info.client_uuid = ?)");

    query.addBindValue(camera_device_name);
    query.addBindValue(client_uuid);

    if(query.exec()) {
        while(query.next()) {
            num_record = query.value(0).toInt();
        }
    }
    query.clear();

    if(num_record == 0)
       ret = false;
    else if(num_record == 1)
       ret = true;

    return ret;
}

bool sqlBase::is_algorithminfo_in_database(QSqlDatabase dbconn, QString &algorithm_name)
{
    int  num_record = 0;
    bool ret = false;
    QSqlQuery query(dbconn);
    query.prepare("SELECT COUNT(algorithm_info.algorithm_id) FROM algorithm_info \
                   WHERE algorithm_info.algorithm_name = ?;");

    query.addBindValue(algorithm_name);
    if(query.exec()) {
        while(query.next()) {
            num_record = query.value(0).toInt();
        }
    }
    query.clear();

    if(num_record == 0)
       ret = false;
    else if(num_record == 1)
       ret = true;

    return ret;
}


bool sqlBase::is_lamp_template_no_in_database(QSqlDatabase dbconn,  QString& client_uuid,int camera_id,int algorithm_id,int index)
{
    int  num_record = 0;
    bool ret = false;
    QSqlQuery query(dbconn);
    if(index == COLOR_FILTER_TEMPLATE)
        query.prepare("SELECT COUNT(lamp_color_template_info.lamp_template_no) FROM lamp_color_template_info \
                       WHERE lamp_color_template_info.client_uuid = ? \
                       AND lamp_color_template_info.camera_id = ?  \
                       AND lamp_color_template_info.algorithm_id = ?;");

    else if( index == BRIGHTNESS_FILTER_TEMPLATE)
        query.prepare("SELECT COUNT(brightness_template_info.brightness_template_no) FROM brightness_template_info  \
                       WHERE brightness_template_info.client_uuid = ? \
                       AND brightness_template_info.camera_id = ?  \
                       AND brightness_template_info.algorithm_id = ?;");

    query.addBindValue(client_uuid);
    qDebug() << "client_uuid:::" << client_uuid;

    query.addBindValue(camera_id);
    qDebug() << "camera_id:::" << camera_id;

    query.addBindValue(algorithm_id);
    qDebug() << "algorithm_id:::" << algorithm_id;

    if(query.exec()) {
        while(query.next()) {
            num_record = query.value(0).toInt();
        }
    }
    query.clear();

    if(num_record == 0)
       ret = false;
    else if(num_record == 1)
       ret = true;

    return ret;
}


int sqlBase::exec_sql(QSqlDatabase dbconn, QString& sql)
{
    QSqlQuery query(dbconn);
    query.exec("BEGIN");
    query.prepare(sql);
    if(query.exec()) {
        query.exec("COMMIT");
        return 1;
    }
    else {
        qDebug()<<query.lastError()<<Qt::endl;
        return -1;

    }
}



int sqlBase::insert_to_system_log(QSqlDatabase dbconn, system_log_info_t *system_log_info)
{
    QSqlQuery query(dbconn);
    query.exec("BEGIN");
    //query.exec("START TRANSACTION");
    query.prepare("INSERT INTO system_log_info(system_log_info.log_id,system_log_info.user_id,system_log_info.event_type_no, \
                  system_log_info.event_dirction_s,system_log_info.event_ack_res_s, \
                  system_log_info.event_dirction_t,system_log_info.event_ack_res_t, \
                  system_log_info.event_time,system_log_info.event_note) \
                  VALUES(?,?,?,?,?,?,?,?,?);");

    query.addBindValue(system_log_info->log_id);
    query.addBindValue(system_log_info->user_id);
    query.addBindValue(system_log_info->event_type_no);
    query.addBindValue(system_log_info->event_direction_s);
    query.addBindValue(system_log_info->event_ack_res_s);
    query.addBindValue(system_log_info->event_direction_t);
    query.addBindValue(system_log_info->event_ack_res_t);
    query.addBindValue(system_log_info->event_time);
    query.addBindValue(system_log_info->event_note);

     if(query.exec()) {
         query.exec("COMMIT");
         return 1;
     }
     else {
         qDebug()<<query.lastError()<<Qt::endl;
         return -1;

     }
}

int sqlBase::insert_to_algorithm_info(QSqlDatabase dbconn, int id, QString &name, QString &version)
{
    QSqlQuery query(dbconn);
    query.exec("BEGIN");
    //query.exec("START TRANSACTION");
    query.prepare("INSERT INTO algorithm_info(algorithm_info.algorithm_id,algorithm_info.algorithm_name,algorithm_info.algorithm_version) \
                   VALUES(?,?,?);");

    query.addBindValue(id);
    query.addBindValue(name);
    query.addBindValue(version);

     if(query.exec()) {
         query.exec("COMMIT");
         return 1;
     }
     else {
         qDebug()<<query.lastError()<<Qt::endl;
         return -1;
     }
}

int sqlBase::insert_to_client_info(QSqlDatabase dbconn, client_all_info_t *client_all_info)
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
    QSqlQuery query(dbconn);
    query.exec("BEGIN");
    //query.exec("START TRANSACTION");
    query.prepare("INSERT INTO client_info(client_info.client_uuid,client_info.client_name, \
                  client_info.client_ip,client_info.client_port,client_info.client_online,  \
                  client_info.client_version)   \
                  VALUES(?,?,?,?,?,?);");

    QString client_uuid = QString((char*)client_all_info->client_info.client_uuid);
    query.addBindValue(client_uuid);

    QString  client_name = QString((char*)client_all_info->client_info.client_name);
    query.addBindValue(client_name);

    QString  client_ip = QString((char*)client_all_info->client_info.client_ip);
    query.addBindValue(client_ip);

    query.addBindValue(client_all_info->client_info.client_port);
    query.addBindValue(client_all_info->client_info.client_online);

    QString  client_version = QString((char*)client_all_info->client_info.client_version);
    query.addBindValue(client_version);


     if(query.exec()) {
         query.exec("COMMIT");
         return 1;
     }
     else {
         qDebug()<<query.lastError()<<Qt::endl;
         return -1;

     }
}

int sqlBase::insert_to_room_info(QSqlDatabase dbconn, client_all_info_t *client_all_info)
{
    QString sql = "SELECT MAX(room_info.room_id) FROM room_info;";
    int room_id = get_maxid(m_dbconnect_info.dbconn,sql) + 1;
    qDebug()<<"room_id::" << room_id <<Qt::endl;


    QSqlQuery query(dbconn);
    query.exec("BEGIN");
    //query.exec("START TRANSACTION");
    query.prepare("INSERT INTO room_info(room_info.room_id,room_info.room_name,room_info.substation_name)   \
                  VALUES(?,?,?);");

    query.addBindValue(room_id);

    QString  room_name = QString((char*)client_all_info->room_info.room_name);
    query.addBindValue(room_name);

    QString  substation_name = QString((char*)client_all_info->room_info.substation_name);
    query.addBindValue(substation_name);

     if(query.exec()) {
         query.exec("COMMIT");
         return 1;
     }
     else {
         qDebug()<<query.lastError()<<Qt::endl;
         return -1;

     }
}

int sqlBase::get_roomid(QSqlDatabase dbconn, client_all_info_t *client_all_info)
{
     QSqlQuery query(dbconn);
     int room_id = 0;
     query.prepare("SELECT room_info.room_id FROM room_info WHERE room_info.room_name = ?;");
     QString  lamp_device_name = QString((char*)client_all_info->room_info.room_name);
     query.addBindValue(lamp_device_name);
     if(query.exec()) {
         while(query.next()) {
             room_id = query.value(0).toInt();
         }
     }
     return room_id;
}

int sqlBase::get_cameraid(QSqlDatabase dbconn, client_all_info_t *client_all_info,int camera_index)
{
    QSqlQuery query(dbconn);
    int camera_id = 0;
    query.prepare("SELECT camera_info.camera_id FROM camera_info WHERE camera_info.camera_device_name = ?;");
    QString  camera_device_name = QString((char*)client_all_info->camera_device_info[camera_index].camera_device_name);
    query.addBindValue(camera_device_name);
    if(query.exec()) {
        while(query.next()) {
            camera_id = query.value(0).toInt();
        }
    }
    return camera_id;
}

int sqlBase::get_cameraidbyname(QSqlDatabase dbconn,QString& camera_name,QString& client_uuid)
{
    QSqlQuery query(dbconn);
    int camera_id = 0;
    query.prepare("SELECT camera_info.camera_id FROM camera_info \
                   WHERE camera_info.camera_device_name = ? \
                   AND camera_info.client_uuid = ?;");

    query.addBindValue(camera_name);
    query.addBindValue(client_uuid);

    if(query.exec()) {
        while(query.next()) {
            camera_id = query.value(0).toInt();
        }
    }
    return camera_id;
}

int sqlBase::get_algorithmid(QSqlDatabase dbconn, QString &algorithm_name, QString &algorithm_version)
{
    QSqlQuery query(dbconn);
    int algorithmid = 0;
    query.prepare("SELECT algorithm_info.algorithm_id FROM algorithm_info \
                   WHERE algorithm_info.algorithm_name = ?  AND algorithm_info.algorithm_version = ?;");

    query.addBindValue(algorithm_name);
    query.addBindValue(algorithm_version);

    if(query.exec()) {
        while(query.next()) {
            algorithmid = query.value(0).toInt();
        }
    }
    return algorithmid;
}



int sqlBase::insert_to_lamp_info(QSqlDatabase dbconn, client_all_info_t *client_all_info,int camera_index)
{
    QString sql = "SELECT MAX(lamp_info.lamp_id) FROM lamp_info;";
    int lamp_id = get_maxid(m_dbconnect_info.dbconn,sql) + 1;
    qDebug()<<"lamp_id::" << lamp_id <<Qt::endl;

    QSqlQuery query(dbconn);
    query.exec("BEGIN");
    int room_id = get_roomid(dbconn,client_all_info);
    int camera_id = get_cameraid(dbconn,client_all_info,camera_index);

    //query.exec("START TRANSACTION");
    query.prepare("INSERT INTO lamp_info(lamp_info.lamp_id,lamp_info.camera_id,lamp_info.room_id,lamp_info.lamp_device_name)\
                  VALUES(?,?,?,?);");

    query.addBindValue(lamp_id);
    query.addBindValue(camera_id);
    query.addBindValue(room_id);

    QString  lamp_device_name = QString((char*)client_all_info->lamp_device_info[camera_index].lamp_device_name);
    query.addBindValue(lamp_device_name);

     if(query.exec()) {
         query.exec("COMMIT");
         return 1;
     }
     else {
         qDebug()<<query.lastError()<<Qt::endl;
         return -1;

     }
}

int sqlBase::insert_to_camera_info(QSqlDatabase dbconn, client_all_info_t *client_all_info,int camera_index)
{
    QString sql = "SELECT MAX(camera_info.camera_id) FROM camera_info;";
    int camera_id = get_maxid(m_dbconnect_info.dbconn,sql) + 1;
    qDebug()<<"camera_id::" << camera_id <<Qt::endl;

    QSqlQuery query(dbconn);
    query.exec("BEGIN");

    //query.exec("START TRANSACTION");
    query.prepare("INSERT INTO camera_info(camera_info.camera_id,camera_info.client_uuid,   \
                  camera_info.camera_device_name,camera_info.width,camera_info.height,  \
                  camera_info.fps,camera_info.white_balance_temp,camera_info.brightness,    \
                  camera_info.contrast,camera_info.saturation,camera_info.sharpness,camera_info.exposure)   \
                  VALUES(?,?,?,?,?,?,?,?,?,?,?,?);");

    query.addBindValue(camera_id);

    QString  client_uuid = QString((char*)client_all_info->client_info.client_uuid);
    query.addBindValue(client_uuid);

    QString  camera_device_name = QString((char*)client_all_info->camera_device_info[camera_index].camera_device_name);
    query.addBindValue(camera_device_name);

    query.addBindValue(client_all_info->camera_device_info[camera_index].camera_paras_info.width);
    query.addBindValue(client_all_info->camera_device_info[camera_index].camera_paras_info.height);
    query.addBindValue(client_all_info->camera_device_info[camera_index].camera_paras_info.fps);
    query.addBindValue(client_all_info->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_white_balance_temp);
    query.addBindValue(client_all_info->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_brightness);
    query.addBindValue(client_all_info->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_contrast);
    query.addBindValue(client_all_info->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_saturation);
    query.addBindValue(client_all_info->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_sharpness);
    query.addBindValue(client_all_info->camera_device_info[camera_index].camera_paras_info.camera_ctrl_paras.val_exposure);

     if(query.exec()) {
         query.exec("COMMIT");
         return 1;
     }
     else {
         qDebug()<<query.lastError()<<Qt::endl;
         return -1;

     }
}

int sqlBase::insert_to_server_info(QSqlDatabase dbconn, server_info_t *m_server_info)
{
    QSqlQuery query(dbconn);
    query.exec("BEGIN");

    query.prepare("INSERT INTO server_info(server_info.server_uuid,server_info.server_name,\
                  server_info.server_mac,server_info.server_ip,server_info.server_outip,\
                  server_info.server_port,server_info.server_version)\
                  VALUES(?,?,?,?,?,?,?);");

    QString  server_uuid = QString((char*)m_server_info->server_uuid);
    query.addBindValue(server_uuid);

    QString  server_name = QString((char*)m_server_info->server_name);
    query.addBindValue(server_name);

    QString  server_mac = QString((char*)m_server_info->server_mac);
    query.addBindValue(server_mac);

    QString  server_ip = QString((char*)m_server_info->server_ip);
    query.addBindValue(server_ip);

    QString  server_outip = QString((char*)m_server_info->server_outip);
    query.addBindValue(server_outip);

    query.addBindValue(m_server_info->server_port);

    QString  server_version = QString((char*)m_server_info->server_version);
    query.addBindValue(server_version);

     if(query.exec()) {
         query.exec("COMMIT");
         return 1;
     }
     else {
         qDebug()<<query.lastError()<<Qt::endl;
         return -1;

     }
}

int sqlBase::update_to_server_info(QSqlDatabase dbconn, server_info_t *m_server_info)
{
    QSqlQuery query(dbconn);
    query.exec("BEGIN");

    query.prepare("UPDATE server_info SET server_info.server_mac = ?,\
                  server_info.server_ip = ?,server_info.server_outip = ?,\
                  server_info.server_port= ?;");

    QString  server_mac = QString((char*)m_server_info->server_mac);
    query.addBindValue(server_mac);

    QString  server_ip = QString((char*)m_server_info->server_ip);
    query.addBindValue(server_ip);

    QString  server_outip = QString((char*)m_server_info->server_outip);
    query.addBindValue(server_outip);

    query.addBindValue(m_server_info->server_port);

     if(query.exec()) {
         query.exec("COMMIT");
         return 1;
     }
     else {
         qDebug()<<query.lastError()<<Qt::endl;
         return -1;

     }
}

int sqlBase::update_to_client_info(QSqlDatabase dbconn,int client_online,QString& client_uuid)
{
    QSqlQuery query(dbconn);
    query.exec("BEGIN");

    query.prepare("UPDATE client_info SET client_info.client_online = ? \
                   WHERE client_info.client_uuid = ?;");
    query.addBindValue(client_online);
    query.addBindValue(client_uuid);

     if(query.exec()) {
         query.exec("COMMIT");
         return 1;
     }
     else {
         qDebug()<<query.lastError()<<Qt::endl;
         return -1;

     }
}

int sqlBase::insert_to_cap_record_info(QSqlDatabase dbconn, cap_record_info_t *cap_record_info)
{
    QSqlQuery query(dbconn);
    query.exec("BEGIN");

    query.prepare("INSERT INTO cap_record_info(cap_record_info.cap_record_no,cap_record_info.user_id, \
                  cap_record_info.client_uuid,cap_record_info.camera_device_name, \
                  cap_record_info.cap_timestamp,cap_record_info.cap_image_path) \
                  VALUES(?,?,?,?,?,?);");


    query.addBindValue(cap_record_info->cap_record_no);
    query.addBindValue(cap_record_info->user_id);
    query.addBindValue(cap_record_info->client_uuid);
    query.addBindValue(cap_record_info->camera_device_name);
    query.addBindValue(cap_record_info->cap_timestamp);
    query.addBindValue(cap_record_info->cap_image_path);



     if(query.exec()) {
         query.exec("COMMIT");
         return 1;
     }
     else {
         qDebug()<<query.lastError()<<Qt::endl;
         return -1;

     }
}

int sqlBase::insert_to_lamp_color_template_info(QSqlDatabase dbconn,lamp_color_template_info_t* lamp_color_template_info)
{
    QSqlQuery query(dbconn);
    query.exec("BEGIN");
    query.prepare("INSERT INTO lamp_color_template_info(lamp_color_template_info.lamp_template_no, \
                  lamp_color_template_info.client_uuid,lamp_color_template_info.camera_id,lamp_color_template_info.algorithm_id, \
                  lamp_color_template_info.imgorg_width,lamp_color_template_info.imgorg_height, \
                  lamp_color_template_info.is_resize, \
                  lamp_color_template_info.imgcut_x,lamp_color_template_info.imgcut_y,  \
                  lamp_color_template_info.imgcut_width,lamp_color_template_info.imgcut_height, \
                  lamp_color_template_info.time_offset, \
                  lamp_color_template_info.template_image_org_path,lamp_color_template_info.template_image_cut_path, \
                  lamp_color_template_info.area_min,lamp_color_template_info.area_max,lamp_color_template_info.color_num, \
                  lamp_color_template_info.color_info_1,lamp_color_template_info.color_info_2,lamp_color_template_info.color_info_3, \
                  lamp_color_template_info.color_info_4,lamp_color_template_info.color_info_5,lamp_color_template_info.color_info_6, \
                  lamp_color_template_info.color_info_7,lamp_color_template_info.color_info_8,lamp_color_template_info.color_info_9, \
                  lamp_color_template_info.color_info_10) \
                  VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");

    query.addBindValue(lamp_color_template_info->lamp_template_no);
    qDebug() << "lamp_template_no:::" << lamp_color_template_info->lamp_template_no;

    query.addBindValue(lamp_color_template_info->client_uuid);
    qDebug() << "client_uuid:::" << lamp_color_template_info->client_uuid;

    query.addBindValue(lamp_color_template_info->camera_id);
    qDebug() << "camera_id:::" << lamp_color_template_info->camera_id;

    query.addBindValue(lamp_color_template_info->algorithm_id);
    qDebug() << "algorithm_id:::" << lamp_color_template_info->algorithm_id;

    query.addBindValue(lamp_color_template_info->imgorg_width);
    qDebug() << "imgorg_width:::" << lamp_color_template_info->imgorg_width;

    query.addBindValue(lamp_color_template_info->imgorg_height);
    qDebug() << "imgorg_height:::" << lamp_color_template_info->imgorg_height;

    query.addBindValue(lamp_color_template_info->is_resize);
    qDebug() << "is_resize:::" << lamp_color_template_info->is_resize;

    query.addBindValue(lamp_color_template_info->imgcut_x);
    qDebug() << "imgcut_x:::" << lamp_color_template_info->imgcut_x;

    query.addBindValue(lamp_color_template_info->imgcut_y);
    qDebug() << "imgcut_y:::" << lamp_color_template_info->imgcut_y;

    query.addBindValue(lamp_color_template_info->imgcut_width);
    qDebug() << "imgcut_width:::" << lamp_color_template_info->imgcut_width;

    query.addBindValue(lamp_color_template_info->imgcut_height);
    qDebug() << "imgcut_height:::" << lamp_color_template_info->imgcut_height;

    query.addBindValue(lamp_color_template_info->time_offset);
    qDebug() << "time_offset:::" << lamp_color_template_info->time_offset;

    query.addBindValue(lamp_color_template_info->template_image_org_path);
    qDebug() << "template_image_org_path:::" << lamp_color_template_info->template_image_org_path;

    query.addBindValue(lamp_color_template_info->template_image_cut_path);
    qDebug() << "template_image_cut_path:::" << lamp_color_template_info->template_image_cut_path;

    query.addBindValue(lamp_color_template_info->area_min);
    qDebug() << "area_min:::" << lamp_color_template_info->area_min;

    query.addBindValue(lamp_color_template_info->area_max);
    qDebug() << "area_max:::" << lamp_color_template_info->area_max;

    query.addBindValue(lamp_color_template_info->color_num);
    qDebug() << "color_num:::" << lamp_color_template_info->color_num;

    for(int i=0;i<lamp_color_template_info->color_num;i++) { //0 1 2 3
        qDebug() << "i:::" << i;
        query.addBindValue(lamp_color_template_info->colorinfoList.at(i));
        qDebug() << "colorinfoList[" << i << "]:::"<< lamp_color_template_info->colorinfoList.at(i);

    }
    for(int i=0;i<10-lamp_color_template_info->color_num;i++) {//0 1 2 3 4 5
        qDebug() << "i:::" << i;
        query.addBindValue("");

    }

    if(query.exec()) {
        query.exec("COMMIT");
        return 1;
    }
    else {
        qDebug()<<query.lastError()<<Qt::endl;
        return -1;
    }
}

int sqlBase::insert_to_brightness_template_info(QSqlDatabase dbconn,brightness_template_info_t* brightness_template_info)
{
    QSqlQuery query(dbconn);
    query.exec("BEGIN");
    query.prepare("INSERT INTO brightness_template_info(brightness_template_info.brightness_template_no,    \
                   brightness_template_info.client_uuid,brightness_template_info.camera_id,    \
                   brightness_template_info.algorithm_id,  \
                   brightness_template_info.imgorg_width,brightness_template_info.imgorg_height,   \
                   brightness_template_info.is_resize,brightness_template_info.imgcut_x,   \
                   brightness_template_info.imgcut_y,brightness_template_info.imgcut_width,    \
                   brightness_template_info.imgcut_height,  \
                   brightness_template_info.template_image_org_path, \
                   brightness_template_info.template_image_cut_path,    \
                   brightness_template_info.num_row,brightness_template_info.num_cols, \
                   brightness_template_info.maskVaule,brightness_template_info.location_offset,    \
                   brightness_template_info.off_x_offset,brightness_template_info.off_y_offset,brightness_template_info.time_offset)   \
                   VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);");

    query.addBindValue(brightness_template_info->brightness_template_no);
    qDebug() << "brightness_template_no:::" << brightness_template_info->brightness_template_no;

    query.addBindValue(brightness_template_info->client_uuid);
    qDebug() << "client_uuid:::" << brightness_template_info->client_uuid;

    query.addBindValue(brightness_template_info->camera_id);
    qDebug() << "camera_id:::" << brightness_template_info->camera_id;

    query.addBindValue(brightness_template_info->algorithm_id);
    qDebug() << "algorithm_id:::" << brightness_template_info->algorithm_id;

    query.addBindValue(brightness_template_info->imgorg_width);
    qDebug() << "imgorg_width:::" << brightness_template_info->imgorg_width;

    query.addBindValue(brightness_template_info->imgorg_height);
    qDebug() << "imgorg_height:::" << brightness_template_info->imgorg_height;

    query.addBindValue(brightness_template_info->is_resize);
    qDebug() << "is_resize:::" << brightness_template_info->is_resize;

    query.addBindValue(brightness_template_info->imgcut_x);
    qDebug() << "imgcut_x:::" << brightness_template_info->imgcut_x;

    query.addBindValue(brightness_template_info->imgcut_y);
    qDebug() << "imgcut_y:::" << brightness_template_info->imgcut_y;

    query.addBindValue(brightness_template_info->imgcut_width);
    qDebug() << "imgcut_width:::" << brightness_template_info->imgcut_width;

    query.addBindValue(brightness_template_info->imgcut_height);
    qDebug() << "imgcut_height:::" << brightness_template_info->imgcut_height;

    query.addBindValue(brightness_template_info->template_image_org_path);
    qDebug() << "template_image_org_path:::" << brightness_template_info->template_image_org_path;

    query.addBindValue(brightness_template_info->template_image_cut_path);
    qDebug() << "template_image_cut_path:::" << brightness_template_info->template_image_cut_path;

    query.addBindValue(brightness_template_info->num_row);
    qDebug() << "num_row:::" << brightness_template_info->num_row;

    query.addBindValue(brightness_template_info->num_cols);
    qDebug() << "num_cols:::" << brightness_template_info->num_cols;

    query.addBindValue(brightness_template_info->maskVaule);
    qDebug() << "maskVaule:::" << brightness_template_info->maskVaule;

    query.addBindValue(brightness_template_info->offset);
    qDebug() << "offset:::" << brightness_template_info->offset;

    query.addBindValue(brightness_template_info->off_x_offset);
    qDebug() << "off_x_offset:::" << brightness_template_info->off_x_offset;

    query.addBindValue(brightness_template_info->off_y_offset);
    qDebug() << "off_y_offset:::" << brightness_template_info->off_y_offset;

    query.addBindValue(brightness_template_info->time_offset);
    qDebug() << "time_offset:::" << brightness_template_info->time_offset;

    if(query.exec()) {
        query.exec("COMMIT");
        return 1;
    }
    else {
        qDebug()<<query.lastError()<<Qt::endl;
        return -1;
    }
}


