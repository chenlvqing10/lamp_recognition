#include "ui_login.h"
#include "ui_ui_login.h"
#include "ui_register.h"
#include <QPainter>
#include <QMessageBox>
#include <QStringList>
#include <QDebug>
#include <QList>
#include <QPalette>
#include <QSettings>
#include <QDir>
#include "ui_reset_passwd.h"
#include "mainwindow.h"
#include "base/eventbase.h"

QString username_login;
QString userid_login;
int     userrole;//得到用户权限
extern dbConnect_info_t m_dbconnect_info;
extern system_log_info_t m_system_log_info;

bool is_register_flag = false;
bool is_passwd_reset_flag = false;

static QStringList usernameList;
static QStringList passwdList;
static QList<save_login_info_t> save_login_infoList;
bool ui_start_flag = false;
ui_login::ui_login(QWidget *parent) :
    QtMainWindow(parent),
    ui(new Ui::ui_login)
{
    rotate = ui_rotate_management::getInstance();
    ui->setupUi(this);
    setFixedSize(600,400);
    sqlbase = new sqlBase;
    timebase = new TimeBase;

    //如果配置文件中有用户信息
    int num = get_userNum_ini();
    if( num > 0 ) {
        if(is_register_flag == true) {
            is_passwd_reset_flag = false;
        }
        else if(is_passwd_reset_flag == true) {
                is_passwd_reset_flag = false;
        }
        else {
            ui->ui_username_cmbox->clear();
            for(int i = 1;i <= num;i++) {
                ui->ui_username_cmbox->addItem(get_username_ini(i));
                ui_start_flag = true;
            }
        }
    }
    else
        ui->ui_username_cmbox->setCurrentText("");

    ui->ui_login_passwd_edt->setText("");

    //passwd can only number and letters
    QRegExp regx("[a-zA-Z0-9]+$");
    QValidator *validator = new QRegExpValidator(regx, ui->ui_login_passwd_edt );
    ui->ui_login_passwd_edt->setValidator( validator );
}

ui_login::~ui_login()
{
    delete ui;
}

//当最大化是重载paintEvent事件
void ui_login::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.drawPixmap(0,0,this->width(),this->height(),QPixmap(":/res/resource/image/bk1.jpg"));
}

void ui_login::set_system_log_for_login()
{
    QString userid = QString::number(get_userid());
    QString sql = "SELECT MAX(system_log_info.log_id) FROM system_log_info;";
    int log_id = sqlbase->get_maxid(m_dbconnect_info.dbconn,sql) + 1;
    int event_type_no = EVENT_TYPE_SERVER_LOGIN;
    int event_direction_s = EVENT_DIRECTION_SERVER_ONLY;
    QString event_ack_res_s = "";
    int event_direction_t = NULL;
    QString event_ack_res_t = "";
    qint64 event_time = timebase->getTimestamps();
    QString event_note = "服务器端用户登录事件";

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

void ui_login::on_ui_login_login_btn_clicked()
{
    usernameList.clear();
    passwdList.clear();
    //1.检查用户名是否为空
    if(ui->ui_username_cmbox->currentText().isEmpty()) {
        QMessageBox::information(this,"提示", "用户名不能为空！");
        return;
    }
    else {
        int ret = check_username(USERNAME_FORMAT_CHECK);
        if(ret < 0) {
            QMessageBox::information(this,"提示", "用户名格式错误!！");
            ui->ui_username_cmbox->setCurrentText("");
            ui->ui_username_cmbox->setFocus();
            return;
        }
        else if(ret > 0)
            qDebug()<< "username format ok！" << Qt::endl;

        ret = check_username(USERNAME_DB_CHECK);
        if(ret < 0) {
            QMessageBox::information(this,"提示", "用户名错误！");
            ui->ui_username_cmbox->setFocus();
            ui->ui_username_cmbox->setCurrentText("");
            ui->ui_login_passwd_edt->setText("");
            ui->ui_login_comment_lab->setText("请确认是否注册，如果未注册，请先注册!!");
            QPalette pe;
            pe.setColor(QPalette::WindowText,Qt::red);
            ui->ui_login_comment_lab->setPalette(pe);
            return;
        }
        else if(ret > 0) {
            qDebug()<< "compare username from db ok！" << Qt::endl;
            ui->ui_login_comment_lab->setText("");
            ret = check_passwd(PASSWD_FORMAT_CHECK);
            if(ret < 0) {
                //2.检查密码是否为空
                if(ui->ui_login_passwd_edt->text().isEmpty()) {
                    QMessageBox::information(this,"提示", "密码不能为空！");
                    ui->ui_login_passwd_edt->setFocus();
                    return;
                }
                else {
                    QMessageBox::information(this,"提示", "密码格式错误!！");
                    ui->ui_login_passwd_edt->setText("");
                    ui->ui_login_passwd_edt->setFocus();
                    return;
                }
            }
            else if (ret > 0)
                qDebug()<< "passwd format ok！" << Qt::endl;

            ret = check_passwd(PASSWD_DB_CHECK);
            if(ret < 0) {
                QMessageBox::information(this,"提示", "密码错误！");
                ui->ui_login_passwd_edt->setFocus();
                ui->ui_login_passwd_edt->setText("");
                ui->ui_login_comment_lab->setText("如果忘记密码，请点击[忘记密码]进行密码重置!!");
                QPalette pe;
                pe.setColor(QPalette::WindowText,Qt::red);
                ui->ui_login_comment_lab->setPalette(pe);
                return;
            }
            else {
                qDebug()<< "compare passwd from db ok！" << Qt::endl;

                //处理是否勾选了保存密码
                if(ui->ui_login_rmbpasswd_ckBox->isChecked()) {
                    qDebug()<< "勾选密码被选中！" << Qt::endl;
                    int num = get_userNum_ini();
                    QString fileName = QDir::currentPath();
                    fileName += "/result/ini/net.ini";
                    QSettings scfg(fileName, QSettings::IniFormat);
                    scfg.setIniCodec("UTF8");
                    if(num > 0) {
                        //qDebug()<< "having contains in ini file" << Qt::endl;
                        bool username_same_flag = false;
                        for(int i = 1;i <= num;i++) {
                            //qDebug()<< "user name get::" << get_username_ini(i) << Qt::endl;
                            //qDebug()<< "user combox name get::" << ui->ui_username_cmbox->itemText(ui->ui_username_cmbox->currentIndex()) << Qt::endl;
                            if(get_username_ini(i) == ui->ui_username_cmbox->currentText()) {
                                username_same_flag = true;
                                break;
                            }
                        }

                        qDebug() << "username_same_flag:" << username_same_flag <<Qt::endl;
                        if(username_same_flag == false) {
                            set_userinfo_ini(&scfg,usernameList.at(0),passwdList.at(0),num+1);
                        }
                    }
                    else {
                        qDebug()<< "having no contains in ini file" << Qt::endl;
                        set_userinfo_ini(&scfg,usernameList.at(0),passwdList.at(0),1);
                    }
                }
                else {
                    //如果没有选中 那么从配置列表中去除
                     qDebug()<< "没有勾选密码被选中！" << Qt::endl;
                     int num = get_userNum_ini();
                     if(num > 0) {
                         for(int i = 1;i <= num;i++) {
                             //qDebug()<< "user name get::" << get_username_ini(i) << Qt::endl;
                             //qDebug()<< "user combox name get::" << ui->ui_username_cmbox->itemText(ui->ui_username_cmbox->currentIndex()) << Qt::endl;
                             //qDebug()<< "user combox current text get::" << ui->ui_username_cmbox->currentText() << Qt::endl;
                             if(get_username_ini(i) == ui->ui_username_cmbox->currentText()) {
                                 remove_ini_byGroup(i,num);
                                 ui->ui_username_cmbox->removeItem(i-1);
                                 break;
                             }
                         }
                     }
                }

                //得到登录的用户名和用户ID
                username_login = ui->ui_username_cmbox->currentText();
                userid_login = sqlbase->get_userid(m_dbconnect_info.dbconn,username_login);
                userrole = sqlbase->get_userrole(m_dbconnect_info.dbconn,username_login);
                qDebug() << "username_login::" << username_login << "userid_login:::" << userid_login <<  "userrole::" << userrole  << Qt::endl;

                ui->ui_login_comment_lab->setText("");
                ui->ui_username_cmbox->setCurrentText("");
                ui->ui_login_passwd_edt->setText("");
                ui->ui_login_rmbpasswd_ckBox->setCheckState(Qt::Unchecked);

                //更新日志表
                set_system_log_for_login();
                ret = sqlbase->insert_to_system_log(m_dbconnect_info.dbconn,&m_system_log_info);
                if(ret > 0) {
                    QMessageBox::information(this,"提示", "日志保存成功!");
                }
                else {
                    QMessageBox::information(this,"提示", "日志保存失败!");
                    return;
                }

                //切换到主界面
                QMessageBox::information(this,"提示", "登录成功！");
                rotate->start_activity_win(&MainWindow::staticMetaObject);
            }
        }
    }

}

int ui_login::check_username(int check_flag)
{
    int ret = 0;
    //check username format
    if(check_flag == USERNAME_FORMAT_CHECK) {
        if((ui->ui_username_cmbox->currentText().length() > USERNAME_MAX) || (ui->ui_username_cmbox->currentText().length() < USERNAME_MIN)) {
            ret =  -1;
        }
        else
            ret = 1;
    }

    //check username from database
    if(check_flag == USERNAME_DB_CHECK) {
        QSqlQuery query(m_dbconnect_info.dbconn);
        QString sql ="SELECT user_info.user_name from user_info;";
        query.prepare(sql);
        if(query.exec()) {
            while(query.next()) {
               if(query.value(0).toString() == ui->ui_username_cmbox->currentText()) {
                   usernameList.append(query.value(0).toString());
                   qDebug() << "username:" << usernameList.at(0) <<Qt::endl;
                   break;
               }
            }
            qDebug() << "size:" << usernameList.size() << Qt::endl;
            query.clear();
            if(usernameList.size() > 0)
                ret = 1;
            else
                ret = -1;
        }
    }

    return ret;
}

int ui_login::check_passwd(int check_flag)
{
    int ret = 0;
    if(check_flag == PASSWD_FORMAT_CHECK) {
        if((ui->ui_login_passwd_edt->text().length() > PASSWD_MAX) || (ui->ui_login_passwd_edt->text().length() < PASSWD_MIN)) {
             ret = -1;
        }
        else
            ret = 1;
    }

    if(check_flag == PASSWD_DB_CHECK) {
        QSqlQuery query(m_dbconnect_info.dbconn);
        query.prepare("SELECT user_info.user_pwd from user_info WHERE user_info.user_name = ?;");
        query.addBindValue(usernameList.at(0));
        if(query.exec()) {
            while(query.next()) {
                if(query.value(0).toString() == ui->ui_login_passwd_edt->text()) {
                   passwdList.append(query.value(0).toString());
                   qDebug() << "password:" << passwdList.at(0) <<Qt::endl;
                   break;
                }
            }//end while
            if(passwdList.size() > 0) {
                ret = 1;
            }
            else
                ret  = -1;
        }
    }

    return ret;
}

int ui_login::get_userNum_ini()
{
    //读取ini的文件
    QString fileName = QDir::currentPath();
    fileName += "/result/ini/net.ini";
    QSettings scfg(fileName,QSettings::IniFormat);
    scfg.beginGroup("LoginUserNum");
    qDebug() << "userNum::" << scfg.value("UserNum").toInt() ;

    return (scfg.value("UserNum").toInt());
}

void ui_login::set_userinfo_ini(QSettings* scfg,const QString& username,const QString& password,int index)
{
    qDebug()<< "username::" << username << Qt::endl;
    qDebug()<< "password::" << password   << Qt::endl;

    QString group = "LoginUserNum";
    scfg->beginGroup(group);
    scfg->setValue("UserNum",index);
    scfg->endGroup();

    QString group1 = QString("%1%2").arg("UserInfo").arg(index);
    qDebug()<< "group1::" << group1 << Qt::endl;
    scfg->beginGroup(group1);
    scfg->setValue("username",username);
    scfg->setValue("password",password);
    scfg->endGroup();
}

QString ui_login::get_username_ini(int index)
{
    QString fileName = QDir::currentPath();
    fileName += "/result/ini/net.ini";
    QSettings scfg(fileName, QSettings::IniFormat);
    scfg.setIniCodec("UTF8");

    QString group_userinfo = QString("%1%2").arg("UserInfo").arg(index);
    qDebug() << "group_userinfo:" << group_userinfo << Qt::endl;
    scfg.beginGroup(group_userinfo);
    QString username = scfg.value("username").toString();
    qDebug() << "username:" << username << Qt::endl;
    scfg.endGroup();

    return username;
}

QString ui_login::get_password_ini(int index)
{
    QString fileName = QDir::currentPath();
    fileName += "/result/ini/net.ini";
    QSettings scfg(fileName, QSettings::IniFormat);
    scfg.setIniCodec("UTF8");

    QString group_userinfo = QString("%1%2").arg("UserInfo").arg(index);
    qDebug() << "group_userinfo:" << group_userinfo << Qt::endl;
    scfg.beginGroup(group_userinfo);
    QString password = scfg.value("password").toString();
    qDebug() << "password:" << password << Qt::endl;
    scfg.endGroup();

    return password;
}

void ui_login::remove_ini_byGroup(int index,int num)
{
    QString fileName = QDir::currentPath();
    fileName += "/result/ini/net.ini";
    QSettings scfg(fileName, QSettings::IniFormat);
    scfg.setIniCodec("UTF8");

    QString group_userinfo = QString("%1%2").arg("UserInfo").arg(index);
    qDebug() << "remove group_userinfo:" << group_userinfo << Qt::endl;
    scfg.beginGroup(group_userinfo);
    scfg.remove("");
    scfg.endGroup();

    //把剩下的用户信息保存在结构体列表中 删除配置文件相关内容
    for(int i = index + 1;i <= num;i++) {
        QString username = get_username_ini(i);
        QString password = get_password_ini(i);
        save_login_info_t save_login_info;
        save_login_info.username = username;
        save_login_info.passwd = password;
        save_login_infoList.append(save_login_info);
        QString newgroup = QString("%1%2").arg("UserInfo").arg(i);
        scfg.beginGroup(newgroup);
        scfg.remove("");
        scfg.endGroup();
    }

    //重新构建配置文件
    for(int i= index;i<save_login_infoList.size() + index;i++) {
        QString newgroup1 = QString("%1%2").arg("UserInfo").arg(i);
        scfg.beginGroup(newgroup1);
        scfg.setValue("username",save_login_infoList[i-index].username);
        scfg.setValue("password",save_login_infoList[i-index].passwd);
        scfg.endGroup();
    }

   //修改数量
    scfg.beginGroup("LoginUserNum");
    scfg.setValue("UserNum",save_login_infoList.size());
    scfg.endGroup();
    save_login_infoList.clear();
}

void ui_login::on_ui_login_register_btn_clicked()
{
    rotate->start_activity_win(&ui_register::staticMetaObject);

}

void ui_login::on_ui_login_rmbpasswd_ckBox_stateChanged(int arg1)
{
    if(ui->ui_login_rmbpasswd_ckBox->isChecked()) {
        int num = get_userNum_ini();
        for(int i = 1;i <= num;i++) {
            if(get_username_ini((i)) == ui->ui_username_cmbox->currentText()) {
                ui->ui_login_passwd_edt->setText(get_password_ini(i));
                break;
            }
        }
    }
    else
    {
       ui->ui_login_passwd_edt->setText("");
    }
}



void ui_login::on_ui_login_fgtpasswd_btn_clicked()
{
    rotate->start_activity_win(&ui_change_passwd::staticMetaObject);
}

int ui_login::get_userid()
{
    int userid;
    QSqlQuery query(m_dbconnect_info.dbconn);
    query.prepare("SELECT user_info.user_id FROM user_info WHERE user_info.user_name  = ?;");
    query.addBindValue(usernameList.at(0));
    if(query.exec()) {
        while(query.next()) {
            userid = query.value(0).toInt();
        }
    }//end while
    else
        return -1;

    return userid;
}


