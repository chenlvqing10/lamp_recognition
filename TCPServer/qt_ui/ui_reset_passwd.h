#ifndef UI_CHANGE_PASSWD_H
#define UI_CHANGE_PASSWD_H

#include <QMainWindow>
#include "base/Qtmainwindow.h"
#include "sql/sqlbase.h"
#include "base/ui_rotate_management.h"

namespace Ui {
class ui_change_passwd;
}

#define USERNAME_MIN 5
#define USERNAME_MAX 20
#define PASSWD_MIN   6
#define PASSWD_MAX   20

class ui_change_passwd : public QtMainWindow
{
    Q_OBJECT

public:
    Q_INVOKABLE explicit ui_change_passwd(QWidget *parent = nullptr);
    ~ui_change_passwd();

private slots:
    void on_ui_change_passwd_submit_btn_clicked();
    void on_ui_change_passwd_cancel_btn_clicked();

private:
    Ui::ui_change_passwd *ui;
    ui_rotate_management* rotate;
     QSqlQuery* query;
    int check_username_db();
    int update_to_userifo();
    int get_userid();
    void set_system_log_for_reset_passwd();
};

#endif // UI_CHANGE_PASSWD_H
