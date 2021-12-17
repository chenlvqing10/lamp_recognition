#include "ui_register.h"
#include "ui_ui_register.h"
#include <QRegExpValidator>
#include <QValidator>
#include <QMessageBox>
#include <QSqlQuery>
#include "sql/sqlbase.h"
#include "base/eventbase.h"
#include "base/timebase.h"
#include <QDebug>

extern bool is_register_flag;

extern dbConnect_info_t m_dbconnect_info;
extern system_log_info_t m_system_log_info;

ui_register::ui_register(QWidget *parent) :
    QtMainWindow(parent),
    ui(new Ui::ui_register)
{
    rotate = ui_rotate_management::getInstance();
    ui->setupUi(this);

    query = NULL;
    ui->ui_register_username_edt->setText("");
    ui->ui_register_passwd_edt->setText("");
    ui->ui_register_curpasswd_edt->setText("");
    //设置密码格式
    QRegExp regx("[a-zA-Z0-9]+$");
    QValidator *validator = new QRegExpValidator(regx, ui->ui_register_passwd_edt );
    ui->ui_register_passwd_edt->setValidator( validator );
    QValidator *validator1 = new QRegExpValidator(regx, ui->ui_register_curpasswd_edt );
    ui->ui_register_curpasswd_edt->setValidator( validator1 );
}

ui_register::~ui_register()
{
    delete ui;
}



void ui_register::on_ui_register_confirm_btn_clicked()
{
    int ret = 0;
    //用户名是否为空
    if(ui->ui_register_username_edt->text().isEmpty()) {
        QMessageBox::information(this,"提示", "用户名不能为空！");
        ui->ui_register_username_edt->setFocus();
        return;
    }
    else {
        //判断用户名是否符合格式
        if(ui->ui_register_username_edt->text().size() > USERNAME_MAX || ui->ui_register_username_edt->text().size() < USERNAME_MIN) {
            QMessageBox::information(this,"提示", "用户名格式错误:请输入至少5个字符，至多20个字符的用户名!!");
            ui->ui_register_username_edt->setText("");
            ui->ui_register_username_edt->setFocus();
            return;
        }
        else {
            //确认用户名是否已经存在
            int ret = check_username_same();
            if(ret > 0) {
                QMessageBox::information(this,"提示", "用户名已经存在，请重新输入！");
                ui->ui_register_username_edt->setText("");
                ui->ui_register_passwd_edt->setText("");
                ui->ui_register_curpasswd_edt->setText("");
                ui->ui_register_username_edt->setFocus();
                return;
            }
            if(ret < 0) {
                //密码确认
                if(ui->ui_register_passwd_edt->text().isEmpty()) {
                    QMessageBox::information(this,"提示", "密码不能为空！");
                    ui->ui_register_passwd_edt->setText("");
                    ui->ui_register_passwd_edt->setFocus();
                    return;
                }
                else  {
                    if(ui->ui_register_passwd_edt->text().size() > PASSWD_MAX || ui->ui_register_passwd_edt->text().size() < PASSWD_MIN) {
                        QMessageBox::information(this,"提示", "密码格式:请输入至少6个字符,至多20个字符的密码,注意密码仅支持数字和大小写字母！");
                        ui->ui_register_passwd_edt->setText("");
                        ui->ui_register_passwd_edt->setFocus();
                        return;
                    }
                    else {
                            if(ui->ui_register_curpasswd_edt->text().isEmpty()) {
                                 QMessageBox::information(this,"提示", "确认密码不能为空，请输入！");
                                 ui->ui_register_passwd_edt->setFocus();
                                 return;
                            }
                            else {
                                if(ui->ui_register_curpasswd_edt->text() != ui->ui_register_passwd_edt->text()) {
                                     QMessageBox::information(this,"提示", "密码和确认密码不一致，请重新输入！");
                                     ui->ui_register_curpasswd_edt->setText("");
                                     ui->ui_register_curpasswd_edt->setFocus();
                                     return;
                                }
                                else {
                                    //保存到数据库用户信息表中
                                     ret = insert_to_userifo();
                                     if(ret > 0) {
                                         QMessageBox::information(this,"提示", "注册成功！");

                                     }
                                     else {
                                         QMessageBox::information(this,"提示", "注册失败，请确认数据库状态！");
                                          ui->ui_register_curpasswd_edt->setText("");
                                          ui->ui_register_username_edt->setText("");
                                          ui->ui_register_passwd_edt->setText("");
                                         return;
                                     }


                                    //更新日志表
                                     set_system_log_for_register();
                                     sqlBase sqlbase;
                                     ret = sqlbase.insert_to_system_log(m_dbconnect_info.dbconn,&m_system_log_info);
                                     if(ret > 0) {
                                         QMessageBox::information(this,"提示", "日志保存成功!");
                                     }
                                     else {
                                         QMessageBox::information(this,"提示", "日志保存失败!");
                                         return;
                                     }

                                     //返回登录界面
                                     is_register_flag = true;
                                     rotate->back_activity_win();

                                }//end 确认密码和密码一致
                            }//end 确认密码不为空
                    }//end 密码格式检查
                }//end 密码为空检查
            }//end 用户名是否存在
        }//end 检查用户名格式
    }//检查用户名是否为空
}

void ui_register::on_ui_register_cancel_btn_clicked()
{
    rotate->back_activity_win();
}

int ui_register::check_username_same()
{
    bool is_same = false;
    query = new QSqlQuery(m_dbconnect_info.dbconn);
    QString sql ="SELECT COUNT(user_info.user_id) FROM user_info WHERE user_info.user_name = ?;";
    query->prepare(sql);
    query->addBindValue(ui->ui_register_username_edt->text());

    if(query->exec()) {
        while(query->next()) {
           if(query->value(0).toInt() == 1) {
               is_same = true;
               qDebug() << "username:" << query->value(0).toString() << Qt::endl;
               break;
           }
        }
    }

    query->clear();

    if(is_same == true) {
        return 1;
    }
    else
        return -1;
}

int ui_register::insert_to_userifo()
{
    sqlBase sqlbase;
    QString sql = "SELECT MAX(user_info.user_id) FROM user_info;";
    int user_id = sqlbase.get_maxid(m_dbconnect_info.dbconn,sql) + 1;
    qDebug() << "user_id::" << user_id << Qt::endl;

    query = new QSqlQuery(m_dbconnect_info.dbconn);
    query->prepare("INSERT INTO user_info(user_info.user_id,user_info.user_name,user_info.user_pwd,user_info.user_role) \
                  VALUES(?,?,?,1);");
    query->addBindValue( QString::number(user_id));
    query->addBindValue(ui->ui_register_username_edt->text());
    query->addBindValue(ui->ui_register_passwd_edt->text());
    if(query->exec()) {
        query->clear();
        return 1;
    }
    else {
        query->clear();
        return -1;
    }
}

int ui_register::get_userid()
{
    int userid;
    QSqlQuery query(m_dbconnect_info.dbconn);
    query.prepare("SELECT user_info.user_id FROM user_info WHERE user_info.user_name  = ?;");
    query.addBindValue(ui->ui_register_username_edt->text());
    if(query.exec()) {
        while(query.next()) {
            userid = query.value(0).toInt();
        }
    }//end while
    else
        return -1;

    return userid;
}


void ui_register::set_system_log_for_register()
{
    sqlBase sqlbase;
    TimeBase timebase;
    QString userid = QString::number(get_userid());
    QString sql = "SELECT MAX(system_log_info.log_id) FROM system_log_info;";
    int log_id = sqlbase.get_maxid(m_dbconnect_info.dbconn,sql) + 1;
    int event_type_no = EVENT_TYPE_SERVER_REGISTER;
    int event_direction_s = EVENT_DIRECTION_SERVER_ONLY;
    QString event_ack_res_s = "";
    int event_dirction_t = NULL;
    QString event_ack_res_t = "";
    qint64 event_time = timebase.getTimestamps();
    QString event_note = "服务器端用户注册事件";

    m_system_log_info.log_id = log_id;
    m_system_log_info.user_id = userid;
    m_system_log_info.event_type_no = event_type_no;
    m_system_log_info.event_direction_s = event_direction_s;
    m_system_log_info.event_ack_res_s   = event_ack_res_s;
    m_system_log_info.event_direction_t  = event_dirction_t;
    m_system_log_info.event_ack_res_t   = event_ack_res_t;
    m_system_log_info.event_time = event_time;
    m_system_log_info.event_note = event_note;
}
