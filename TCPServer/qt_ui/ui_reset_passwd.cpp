#include "ui_reset_passwd.h"
#include "ui_ui_reset_passwd.h"
#include <QDebug>
#include <QMessageBox>
#include "sql/sqlbase.h"
#include "base/timebase.h"
#include "base/eventbase.h"
#include <QDir>
#include <QFile>

extern bool is_passwd_reset_flag;
extern dbConnect_info_t m_dbconnect_info;
extern system_log_info_t m_system_log_info;

ui_change_passwd::ui_change_passwd(QWidget *parent) :
    QtMainWindow(parent),
    ui(new Ui::ui_change_passwd)
{
    rotate = ui_rotate_management::getInstance();
    query = NULL;
    ui->setupUi(this);

}

ui_change_passwd::~ui_change_passwd()
{
    delete ui;
}


void ui_change_passwd::on_ui_change_passwd_submit_btn_clicked()
{
    int ret = 0;
    //用户名是否为空
    if(ui->ui_change_passwd_username_edt->text().isEmpty()) {
        QMessageBox::information(this,"提示", "用户名不能为空！");
        ui->ui_change_passwd_username_edt->setFocus();
        return;
    }
    else {
        //判断用户名是否符合格式
        if(ui->ui_change_passwd_username_edt->text().size() > USERNAME_MAX || ui->ui_change_passwd_username_edt->text().size() < USERNAME_MIN) {
            QMessageBox::information(this,"提示", "用户名格式错误:请输入至少5个字符，至多20个字符的用户名!!");
            ui->ui_change_passwd_username_edt->setText("");
            ui->ui_change_passwd_username_edt->setFocus();
            return;
        }
        else {
             //用户名是否在数据库中
            ret = check_username_db();
            if(ret < 0) {
                QMessageBox::information(this,"提示", "该用户名不存在！");
                ui->ui_change_passwd_username_edt->setText("");
                ui->ui_change_passwd_username_edt->setFocus();
                return;
            }
            else {//in db
                //密码确认
                if(ui->ui_change_passwd_newpwd_edt->text().isEmpty()) {
                    QMessageBox::information(this,"提示", "密码不能为空！");
                    ui->ui_change_passwd_newpwd_edt->setFocus();
                    return;
                }
                else  {
                    if(ui->ui_change_passwd_newpwd_edt->text().size() > PASSWD_MAX || ui->ui_change_passwd_newpwd_edt->text().size() < PASSWD_MIN) {
                        QMessageBox::information(this,"提示", "密码格式:请输入至少6个字符,至多20个字符的密码,注意密码仅支持数字和大小写字母！");
                        ui->ui_change_passwd_newpwd_edt->setText("");
                        ui->ui_change_passwd_newpwd_edt->setFocus();
                        return;
                    }
                    else {
                        if(ui->ui_change_passwd_curpwd_edt->text().isEmpty()) {
                            QMessageBox::information(this,"提示", "确认密码不能为空！");
                            ui->ui_change_passwd_curpwd_edt->setFocus();
                            return;
                        }
                        else {
                            if(ui->ui_change_passwd_curpwd_edt->text() != ui->ui_change_passwd_newpwd_edt->text()) {
                                QMessageBox::information(this,"提示", "新密码和确认密码不一致，请重新输入！");
                                ui->ui_change_passwd_curpwd_edt->setText("");
                                ui->ui_change_passwd_curpwd_edt->setFocus();
                                return;
                            }
                            else {
                                //更新数据库
                                ret = update_to_userifo();
                                if(ret < 0) {
                                    QMessageBox::information(this,"提示", "更新失败，请检查数据库状态！");
                                    ui->ui_change_passwd_username_edt->setText("");
                                    ui->ui_change_passwd_newpwd_edt->setText("");
                                    ui->ui_change_passwd_curpwd_edt->setText("");
                                    ui->ui_change_passwd_username_edt->setFocus();
                                    return;
                                }
                                else {
                                    QMessageBox::information(this,"提示", "更新成功!");

                                    //更新日志表
                                    sqlBase sqlbase;
                                    set_system_log_for_reset_passwd();
                                    ret = sqlbase.insert_to_system_log(m_dbconnect_info.dbconn,&m_system_log_info);
                                    if(ret > 0) {
                                        QMessageBox::information(this,"提示", "日志保存成功!");
                                    }
                                    else {
                                        QMessageBox::information(this,"提示", "日志保存失败!");
                                        return;
                                    }

                                    //删除配置文件
                                    QString fileName = QDir::currentPath();
                                    fileName += "/result/ini/net.ini";
                                    QFile file;
                                    file.remove(fileName);


                                    //返回登录界面
                                    is_passwd_reset_flag = true;
                                    rotate->back_activity_win();

                                }

                            }
                        }
                    }//end check passwd format is true
                }//end check passwd empty
            }//end check username is in db
        }//end check username format is true
    }//end check username empty


}

void ui_change_passwd::on_ui_change_passwd_cancel_btn_clicked()
{
      rotate->back_activity_win();
}

int ui_change_passwd::check_username_db()
{
    bool is_same = false;
    query = new QSqlQuery(m_dbconnect_info.dbconn);
    QString sql ="SELECT COUNT(user_info.user_id) FROM user_info WHERE user_info.user_name = ?;";
    query->prepare(sql);
    query->addBindValue(ui->ui_change_passwd_username_edt->text());

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

int ui_change_passwd::update_to_userifo()
{
    query = new QSqlQuery(m_dbconnect_info.dbconn);
    query->prepare("UPDATE user_info SET user_info.user_pwd = ? WHERE user_info.user_name = ?;");
    query->addBindValue(ui->ui_change_passwd_newpwd_edt->text());
    query->addBindValue(ui->ui_change_passwd_username_edt->text());
    if(query->exec()) {
        query->clear();
        return 1;
    }
    else {
        query->clear();
        return -1;
    }
}

int ui_change_passwd::get_userid()
{
    int userid;
    QSqlQuery query(m_dbconnect_info.dbconn);
    query.prepare("SELECT user_info.user_id FROM user_info WHERE user_info.user_name  = ?;");
    query.addBindValue(ui->ui_change_passwd_username_edt->text());
    if(query.exec()) {
        while(query.next()) {
            userid = query.value(0).toInt();
        }
    }//end while
    else
        return -1;

    return userid;
}

void ui_change_passwd::set_system_log_for_reset_passwd()
{
    sqlBase sqlbase;
    TimeBase timebase;
    QString userid = QString::number(get_userid());
    QString sql = "SELECT MAX(system_log_info.log_id) FROM system_log_info;";
    int log_id = sqlbase.get_maxid(m_dbconnect_info.dbconn,sql) + 1;
    int event_type_no = EVENT_TYPE_SERVER_PASSWD_RESET;
    int event_direction_s = EVENT_DIRECTION_SERVER_ONLY;
    QString event_ack_res_s = "";
    int event_direction_t = NULL;
    QString event_ack_res_t = "";
    qint64 event_time = timebase.getTimestamps();
    QString event_note = "服务器端用户密码重置事件";

    m_system_log_info.log_id = log_id;
    m_system_log_info.user_id = userid;
    m_system_log_info.event_type_no = event_type_no;
    m_system_log_info.event_direction_s = event_direction_s;
    m_system_log_info.event_ack_res_s   = event_ack_res_s;
    m_system_log_info.event_direction_t  = event_direction_t;
    m_system_log_info.event_ack_res_t   = event_ack_res_t;
    m_system_log_info.event_time = event_time;
    m_system_log_info.event_note = event_note;
}
