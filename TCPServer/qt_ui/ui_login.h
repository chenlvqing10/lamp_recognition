#ifndef LOGIN_H
#define LOGIN_H

#include <QMainWindow>
#include "base/Qtmainwindow.h"
#include "sql/sqlbase.h"
#include "base/timebase.h"
#include "base/ui_rotate_management.h"
#include <QSettings>

namespace Ui {
class ui_login;
}

typedef struct save_login_info {
    QString username;
    QString passwd;
}save_login_info_t;

#define USERNAME_FORMAT_CHECK 0
#define USERNAME_DB_CHECK     1
#define PASSWD_FORMAT_CHECK   0
#define PASSWD_DB_CHECK       1

#define USERNAME_MIN 5
#define USERNAME_MAX 20
#define PASSWD_MIN   6
#define PASSWD_MAX   20

class ui_login : public QtMainWindow
{
    Q_OBJECT

public:
     Q_INVOKABLE explicit ui_login(QWidget *parent = nullptr);
    ~ui_login();
    
private slots:
    void on_ui_login_login_btn_clicked();
    void on_ui_login_register_btn_clicked();
    void on_ui_login_rmbpasswd_ckBox_stateChanged(int arg1);
    //void on_ui_username_cmbox_currentIndexChanged(int index);
    void on_ui_login_fgtpasswd_btn_clicked();

private:
    Ui::ui_login *ui;
    sqlBase* sqlbase;
    TimeBase* timebase;
    int get_userid();
    int check_username(int check_flag);
    int check_passwd(int check_flag);
    int  get_userNum_ini();
    void set_userinfo_ini(QSettings* scfg,const QString& username,const QString& password,int index);
    QString get_username_ini(int index);
    QString get_password_ini(int index);
    void remove_ini_byGroup(int index,int num);
    void paintEvent(QPaintEvent *e);
    void set_system_log_for_login();
    ui_rotate_management *rotate;
};

#endif // LOGIN_H
