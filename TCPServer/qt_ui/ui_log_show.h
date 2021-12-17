#ifndef UI_LOG_SHOW_H
#define UI_LOG_SHOW_H

#include <QMainWindow>
#include <QThread>
#include "sql/sqlbase.h"
#include <QList>

namespace Ui {
class ui_log_show;
}

class ui_log_show : public QMainWindow
{
    Q_OBJECT
    QThread ui_log_show_Thread;

public:
    explicit ui_log_show(QWidget *parent = nullptr);
    ~ui_log_show();

private slots:
    void on_ui_log_show_event_type_cmbx_currentTextChanged(const QString &arg1);

    void on_ui_log_show_event_type_select_btn_clicked();

private:
    Ui::ui_log_show *ui;
    void ui_log_show_init();
};

#endif // UI_LOG_SHOW_H
