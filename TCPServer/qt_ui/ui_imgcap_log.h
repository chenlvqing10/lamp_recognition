#ifndef UI_IMGCAP_LOG_H
#define UI_IMGCAP_LOG_H

#include <QMainWindow>

namespace Ui {
class ui_imgcap_log;
}

class ui_imgcap_log : public QMainWindow
{
    Q_OBJECT

public:
    explicit ui_imgcap_log(QWidget *parent = nullptr);
    ~ui_imgcap_log();

private slots:
    void on_ui_imgcap_log_select_btn_clicked();

private:
    Ui::ui_imgcap_log *ui;
    void ui_imgcap_log_init();
};

#endif // UI_IMGCAP_LOG_H
