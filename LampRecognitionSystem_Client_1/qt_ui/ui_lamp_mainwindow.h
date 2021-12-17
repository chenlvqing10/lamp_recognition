#ifndef UI_LAMP_MAINWINDOW_H
#define UI_LAMP_MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QHostAddress>
#include <QMessageBox>
#include <QTimer>

#include "servers/Thread/taskthread.h"
#include "base/include/thread_base.h"


QT_BEGIN_NAMESPACE
namespace Ui { class ui_lamp_mainWindow; }
QT_END_NAMESPACE


enum ConnectID{
    connectID_0 =0,
    connectID_1,
    connectID_2,
    connectID_3,
    connectID_4,
    connectID_5,
    connectID_6,
    connectID_7,
    connectID_8,
    connectID_9,
    connectID_10,
    connectID_11
};

class ui_lamp_mainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ui_lamp_mainWindow(QWidget *parent = nullptr);
    ~ui_lamp_mainWindow();


private slots:
    void on_ui_lamp_connect_btn1_clicked();
    void on_ui_lamp_connect_btn2_clicked();
    void on_ui_lamp_connect_btn3_clicked();
    void on_ui_lamp_connect_btn4_clicked();
    void on_ui_lamp_connect_btn5_clicked();
    void on_ui_lamp_connect_btn6_clicked();
    void on_ui_lamp_connect_btn7_clicked();
    void on_ui_lamp_connect_btn8_clicked();
    void on_ui_lamp_connect_btn9_clicked();
    void on_ui_lamp_connect_btn10_clicked();
    void on_ui_lamp_connect_btn11_clicked();
    void on_ui_lamp_connect_btn12_clicked();

private:
    Ui::ui_lamp_mainWindow *ui;
    void set_client_info(QString path);
    void get_client_info(QString path);
    void get_room_info();
    QString getVersion();

private:
    TaskThread* taskThread1;
    QTcpSocket *tcpClient;  //定义
    QTimer* timer;          //定时器

signals:
    void sig_TaskThread(taskthread_info_t*);

};
#endif // UI_LAMP_MAINWINDOW_H
