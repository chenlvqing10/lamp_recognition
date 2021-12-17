#ifndef REGISTER_H
#define REGISTER_H

#include <QMainWindow>
#include "base/Qtmainwindow.h"
#include "base/ui_rotate_management.h"
#include <QSqlQuery>

#define USERNAME_MIN 5
#define USERNAME_MAX 20
#define PASSWD_MIN   6
#define PASSWD_MAX   20

namespace Ui {
class ui_register;
}

class ui_register : public QtMainWindow
{
    Q_OBJECT

public:
    Q_INVOKABLE  explicit ui_register(QWidget *parent = nullptr);
    ~ui_register();

private slots:
    void on_ui_register_confirm_btn_clicked();
    void on_ui_register_cancel_btn_clicked();

private:
    Ui::ui_register *ui;
    ui_rotate_management *rotate;
    int check_username_same();
    int insert_to_userifo();
    void set_system_log_for_register();
    int get_userid();
    QSqlQuery* query;
};

#endif // REGISTER_H
