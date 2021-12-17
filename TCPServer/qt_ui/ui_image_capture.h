#ifndef UI_IMAGE_CAPTURE_H
#define UI_IMAGE_CAPTURE_H

#include <QMainWindow>
#include "ui_imgcap_log.h"
#include <QTimer>

namespace Ui {
class ui_image_capture;
}

class ui_image_capture : public QMainWindow
{
    Q_OBJECT

public:
    explicit ui_image_capture(QWidget *parent = nullptr);
    ~ui_image_capture();

private slots:
    void on_ui_imgcap_clientname_cmbx_currentIndexChanged(int index);
    void on_ui_imgcap_cameraname_cmbx_currentIndexChanged(int index);
    void on_ui_imgcap_clientname_cmbx_currentTextChanged(const QString &arg1);
    void on_ui_imacap_log_btn_clicked();
    void on_ui_imgcap_cap_btn_clicked();
    void slot_reshow_widget();

private:
    ui_imgcap_log* imgcap_log_obj;
    Ui::ui_image_capture *ui;
    QTimer* timer;
    void init_widget(int client_num);
    int get_clinet_index();
    int get_camera_index();
};

#endif // UI_IMAGE_CAPTURE_H
