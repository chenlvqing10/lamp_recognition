#ifndef UI_SYSTEM_SETTING_H
#define UI_SYSTEM_SETTING_H

#include <QMainWindow>
#include <QTimer>
//#include "base/Qtmainwindow.h"
//#include "base/ui_rotate_management.h"

#define BRIGHTNESS_INIT     0
#define CONTRAST_INIT       32
#define SATURATION_INIT     64
#define SHARPRESS_INIT      50
#define WHITEBALANCE_INIT   4600
#define EXPOSURE_INIT       300
#define FPS_INIT            30
#define RESOLUTION_INIT     "1920 * 1080"

namespace Ui {
class ui_system_setting;
}

class ui_system_setting : public QMainWindow
{
    Q_OBJECT

public:
    explicit ui_system_setting(QWidget *parent = nullptr);
    ~ui_system_setting();

private:
    void init_widget(int client_num);
    void set_camera_params(int client_index,int camera_index);
    int  get_clinet_index();
    int  get_camera_index();
    QTimer* timer;

private:
    Ui::ui_system_setting *ui;
    //ui_rotate_management *rotate;
private slots:
    void on_ui_brightness_v_sld_valueChanged(int value);
    void on_ui_contrast_v_sld_valueChanged(int value);
    void on_ui_saturation_v_sld_valueChanged(int value);
    void on_ui_sharpress_v_sld_valueChanged(int value);
    void on_ui_whitebalance_v_sld_valueChanged(int value);
    void on_ui_exposure_v_sld_valueChanged(int value);
    void on_ui_fps_v_sld_valueChanged(int value);
    void on_ui_resolution_v_cmbx_currentIndexChanged(int index);
    void on_ui_system_setting_camera_setting_btn_clicked();

    void on_ui_system_setting_passwdchg_btn_clicked();
    void on_ui_system_setting_clientname_cmbx_currentIndexChanged(int index);
    void on_ui_system_setting_cameraname_cmbx_currentIndexChanged(int index);
    void on_ui_system_setting_clientname_cmbx_currentTextChanged(const QString &arg1);
    void on_ui_system_setting_cameraname_cmbx_currentTextChanged(const QString &arg1);

    void slot_reinit_widget();

};

#endif // UI_SYSTEM_SETTING_H
