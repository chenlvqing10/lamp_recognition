#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include "base/Qtmainwindow.h"
#include "sql/sqlbase.h"
#include "base/ui_rotate_management.h"
#include <QTimer>
#include "ui_algorithm_setting.h"
#include "ui_log_show.h"
#include "ui_system_setting.h"
#include "ui_image_capture.h"
#include "Thread/myserver.h"
#include <QLabel>

typedef struct client_label_info {
    QLabel* ui_client_show_lab1;
    QLabel* ui_client_show_lab2;
    QLabel* ui_client_show_lab3;
    QLabel* ui_client_show_lab4;
}client_label_info_t;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QtMainWindow
{
    Q_OBJECT

public:
    Q_INVOKABLE explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
 signals:
    void sig_StartServer(int);
    void sig_StopServer();
private slots:
    void slot_showTime();
    void on_mainWindow_listern_btn_clicked();
    void on_mainWindow_algorithm_setting_btn_clicked();
    void on_mainWindow_log_select_btn_clicked();
    void on_pushButton_clicked();
    void on_mainWindow_image_cap_btn_clicked();
    void on_mainWindow_system_setting_btn_clicked();
    void on_mainWindow_savepath_btn_clicked();
    void on_mainWindow_alarm_info_btn_clicked();

private:
    Ui::MainWindow *ui;
    ui_rotate_management *rotate;

    //定时器对象指针
    QTimer* timer;

    //窗体类对象指针数组  存放多个客户端窗体
    QWidget* tab[100];
    QList<client_label_info_t> labList;     //客户端窗体放置4个标签空前
    QStringList tab_obj_list;               //客户端显示名字

    //子界面对象
    ui_system_setting* system_setting_obj;
    ui_log_show* log_show_obj;
    ui_image_capture* image_capture_obj;
    ui_algorithm_setting* algorithm_setting_obj;

    //服务器对象指针
    myServer *myserver_obj;

private:
    void get_client_all_info(cJSON *json,int client_index);
    void draw_driver_info(int client_num,int camera_num);
    void draw_client_info(int client_num);
    int  get_camera_num_online(int client_num);
    void draw_image_info_for_video(int client_num,int camera_num);
    void show_newpwd_ui();
};
#endif // MAINWINDOW_H
