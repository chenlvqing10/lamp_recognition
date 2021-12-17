#ifndef SQLBASE_H
#define SQLBASE_H
#include <QString>
#include <QSql>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "base/eventbase.h"


typedef struct dbConnect_info {
    QString hostName;
    QString dbName;
    QString userName;
    QString password;
    QSqlDatabase dbconn;
}dbConnect_info_t;

typedef struct system_log_info {
    int log_id;
    QString user_id;
    int event_type_no;
    int event_direction_s;
    QString event_ack_res_s;
    int event_direction_t;
    QString event_ack_res_t;
    qint64 event_time;
    QString event_note;
}system_log_info_t ;

class sqlBase
{
public:
    sqlBase();
    ~sqlBase();
private:
    int  get_roomid(QSqlDatabase dbconn, client_all_info_t *client_all_info);
    int  get_cameraid(QSqlDatabase dbconn, client_all_info_t *client_all_info,int camera_index);
public:
    bool createConnect();
    int  exec_sql(QSqlDatabase dbconn, QString& sql);
    int  get_cameraidbyname(QSqlDatabase dbconn,QString& camera_name,QString& client_uuid);
    int  get_algorithmid(QSqlDatabase dbconn,QString& algorithm_name,QString& algorithm_version);
    int  get_maxid(QSqlDatabase dbconn,QString& sql);
    QString  get_userid(QSqlDatabase dbconn,QString& username);
    int get_userrole(QSqlDatabase dbconn, QString &username);
    int get_number_of_imgcap(QSqlDatabase dbconn, QString& client_uuid,QString& camera_device_name);

    int  insert_to_system_log(QSqlDatabase dbconn,system_log_info_t* system_log_info);
    int  insert_to_algorithm_info(QSqlDatabase dbconn,int id,QString& name,QString& version);
    int  insert_to_client_info(QSqlDatabase dbconn, client_all_info_t *client_all_info);
    int  insert_to_room_info(QSqlDatabase dbconn, client_all_info_t *client_all_info);
    int  insert_to_lamp_info(QSqlDatabase dbconn, client_all_info_t *client_all_info,int camera_index);
    int  insert_to_camera_info(QSqlDatabase dbconn, client_all_info_t *client_all_info,int camera_index);
    int  insert_to_server_info(QSqlDatabase dbconn,  server_info_t *m_server_info);
    int  insert_to_cap_record_info(QSqlDatabase dbconn,cap_record_info_t *cap_record_info);
    int  insert_to_lamp_color_template_info(QSqlDatabase dbconn,lamp_color_template_info_t* lamp_color_template_info);
    int  insert_to_brightness_template_info(QSqlDatabase dbconn,brightness_template_info_t* brightness_template_info);

    int  update_to_server_info(QSqlDatabase dbconn,  server_info_t *m_server_info);
    int  update_to_client_info(QSqlDatabase dbconn,int client_online,QString& client_uuid);

    bool is_client_in_database(QSqlDatabase dbconn,QString& client_uuid);
    bool is_roominfo_in_database(QSqlDatabase dbconn,QString& room_name,QString& substation_name);
    bool is_camerainfo_in_database(QSqlDatabase dbconn, QString& client_uuid, QString &camera_device_name);
    bool is_lampinfo_in_database(QSqlDatabase dbconn, QString& client_uuid, QString &camera_device_name);
    bool is_algorithminfo_in_database(QSqlDatabase dbconn,QString& algorithm_name);
    bool is_lamp_template_no_in_database(QSqlDatabase dbconn,  QString& client_uuid,int camera_id,int algorithm_id,int index);
};

#endif // SQLBASE_H
