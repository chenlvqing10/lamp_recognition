#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStandardItemModel>
#include <fstream>
#include <iostream>
#include <QDateTime>
#include <QPixmap>
#include <QDir>
#include <QSettings>
#include <QMessageBox>
#include "base/tablestyle.h"


extern bool listen_flag;//服务端监听结果
extern QList<client_all_info_t*> clientList;//管理客户端的列表
extern QList<client_net_info_t*>TcpSocketList;      //管理客户端socket ip port
extern taskThread_Result_info_t taskResult_info;//服务端处理客户端消息的业务逻辑结果
extern int  userrole;//得到用户权限

MainWindow::MainWindow(QWidget *parent)
    : QtMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    rotate = ui_rotate_management::getInstance();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);//调用析构

    /* 初始化 */
    for(int i=0;i<100;i++) {
        tab[i] = NULL;
    }
    labList.clear();
    tab_obj_list.clear();
    timer = NULL;//界面交互定时器

    system_setting_obj = NULL;
    log_show_obj = NULL;
    image_capture_obj = NULL;
    algorithm_setting_obj = NULL;

    myserver_obj = NULL;

    for(int i=0;i<100;i++) {
        QString str = QString("%1%2").arg("客户端").arg(i+1);
        qDebug() << "str::" << str  << Qt::endl;
        tab_obj_list.append(str);
    }

    //启动定时器 检测客户端与服务器的交互动态  进行主界面的界面操作
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(slot_showTime()));
    timer->start(100);

    //ui->tabWidget->setCurrentIndex(0);
    ui->tabWidget->tabBar()->setStyle(new TableStyle);



}

MainWindow::~MainWindow()
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
    delete ui;

    if(system_setting_obj) {
        delete system_setting_obj;
        system_setting_obj = NULL;
    }

    if(log_show_obj) {
        delete log_show_obj;
        log_show_obj = NULL;
    }

    if(image_capture_obj) {
        delete image_capture_obj;
        image_capture_obj = NULL;
    }

    if(algorithm_setting_obj) {
        delete algorithm_setting_obj;
        algorithm_setting_obj = NULL;
    }

    if(myserver_obj) {
        delete myserver_obj;
        myserver_obj = NULL;
    }

    labList.clear();
    tab_obj_list.clear();

    for(int i=0;i<clientList.size();i++) {
        TcpSocketList.removeAt(i);
        clientList.removeAt(i);
    }

    taskResult_info.client_cancel_flag = true;

}
static int timeout = 0;
void MainWindow::slot_showTime()
{
    //qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
    //qDebug() << "连接设备数::" << clientList.size();
    int client_num = clientList.size();
    int camera_num = get_camera_num_online(client_num);


    draw_driver_info(client_num,camera_num);//显示客户端信息 设备信息

    //重新绘制客户端
    if(taskResult_info.client_add_flag == true || taskResult_info.client_cancel_flag == true) {
        if(taskResult_info.client_cancel_flag == true) {
            timeout += 100;
            if(timeout >= 300) {
                 taskResult_info.client_cancel_flag = false;
                 timeout = 0;
            }
        }

        draw_client_info(client_num);//实时添加连接的客户端
        taskResult_info.client_add_flag     = false;
    }

    if(taskResult_info.client_show_flag == true) {
        int client_index = taskResult_info.current_client;
        draw_image_info_for_video(client_index,clientList.at(client_index)->camera_num);//显示该客户端所有的相机信息
        taskResult_info.client_show_flag = false;
    }//end show

    if(taskResult_info.newpwd_flag == true) {
        taskResult_info.newpwd_flag = false;
        show_newpwd_ui();
    }
}

//监听客户端连接
void MainWindow::on_mainWindow_listern_btn_clicked()
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
    if(ui->mainWindow_port_ledt->text().isEmpty()) {
        QMessageBox::information(this,"提示", "请输入端口号!");
        return;
    }

    if(ui->mainWindow_listern_btn->text() == "监听") {
        myserver_obj = new myServer();
        connect(this,SIGNAL(sig_StartServer(int)),myserver_obj,SLOT(slot_StartServer(int)),Qt::UniqueConnection);
        connect(this,SIGNAL(sig_StopServer()),myserver_obj,SLOT(slot_StopServer()),Qt::UniqueConnection);

        ui->mainWindow_port_ledt->setEnabled(false);
        ui->mainWindow_listern_btn->setText("断开");
        int port = ui->mainWindow_port_ledt->text().toInt();

        //配置服务器信息 端口号
        QString fileName = QDir::currentPath();
        fileName += "/config/system.ini";
        qDebug() << "fileName:" << fileName << Qt::endl;
        QSettings scfg(fileName, QSettings::IniFormat);
        scfg.setIniCodec("UTF8");
        scfg.beginGroup("ServerInfo");
        scfg.setValue("server_port",port);
        scfg.endGroup();

        //发送启动监听客户端服务信号
        emit sig_StartServer(port);
        if(listen_flag == true) {
              qDebug() << "listening.........." << Qt::endl;
              setWindowTitle("LISTENING......");
        }
        else {
            QMessageBox::information(this,"提示", "服务端监听失败!");
            return;
        }
    }
    else {
        ui->mainWindow_port_ledt->setEnabled(true);
        ui->mainWindow_listern_btn->setText("监听");

        emit sig_StopServer();
        disconnect(this, 0, myserver_obj, 0);//断开信号槽防止两次绑定从而调用两次槽函数
        delete   myserver_obj;
        myserver_obj = NULL;

        setWindowTitle("no listening..........");
    }

}

//算法配置界面
void MainWindow::on_mainWindow_algorithm_setting_btn_clicked()
{
    if(clientList.size() == 0) {
        QMessageBox::information(NULL, "Warning!!", "没有可用的客户端!!");
        return;
    }
    algorithm_setting_obj = new ui_algorithm_setting();
    algorithm_setting_obj->show();
}

//日志查询界面
void MainWindow::on_mainWindow_log_select_btn_clicked()
{
    log_show_obj = new ui_log_show();
    log_show_obj->show();
}

//返回按钮
void MainWindow::on_pushButton_clicked()
{
    timer->stop();
    clientList.clear();
    this->close();
    rotate->back_activity_win();
}

//视频抓拍界面
void MainWindow::on_mainWindow_image_cap_btn_clicked()
{
    if(clientList.size() == 0) {
        QMessageBox::information(NULL, "Warning!!", "没有可用的客户端!!");
        return;
    }
    image_capture_obj = new ui_image_capture();
    image_capture_obj->show();
}

//系统设置界面
void MainWindow::on_mainWindow_system_setting_btn_clicked()
{
    system_setting_obj = new ui_system_setting();
    system_setting_obj->show();
}

void MainWindow::on_mainWindow_savepath_btn_clicked()
{

}

void MainWindow::draw_driver_info(int client_num, int camera_num)
{
    ui->mainWindow_clientnum_show_lab->setText(QString::number(client_num));
    ui->mainWindow_cameranum_show_lab->setText(QString::number(camera_num));
    if(userrole == 0)
        ui->mainWindow_userrole_show_lab->setText("管理员");
    else
        ui->mainWindow_userrole_show_lab->setText("普通用户");
}

void MainWindow::draw_client_info(int client_num)
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;

    //初始化 清理内存
    for(int i=0;i<100;i++) {
        if(tab[i]) {// 分配了内存
            delete tab[i];
            tab[i] = NULL;
        }
    }
    labList.clear();

    ui->tabWidget->clear();//清除 按实际客户端数重载客户端
    for(int i=0;i<client_num;i++) {
        tab[i] = new QWidget();
        client_label_info_t client_label_infor;
        QVBoxLayout* verticalLayout = new QVBoxLayout(tab[i]);//垂直布局
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, 0);

        QWidget* ui_client_show_widget1 = new QWidget(tab[i]);//新的窗体一
        ui_client_show_widget1->setObjectName(QString::fromUtf8("ui_client_show_widget1"));

        QHBoxLayout* horizontalLayout_7 = new QHBoxLayout(ui_client_show_widget1);//在新的窗体一上放置水平布局
        horizontalLayout_7->setSpacing(0);
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        horizontalLayout_7->setContentsMargins(0, 0, 0, 0);

        client_label_infor.ui_client_show_lab1 = new QLabel(ui_client_show_widget1);//新的窗体一上放置一个标签控件
        client_label_infor.ui_client_show_lab1 ->setObjectName(QString::fromUtf8("ui_client_show_lab1"));
        client_label_infor.ui_client_show_lab1 ->setAlignment(Qt::AlignCenter);
        QString temp = QString("%1%2").arg("show image for client ").arg(i);
        client_label_infor.ui_client_show_lab1 ->setText(temp);
        horizontalLayout_7->addWidget(client_label_infor.ui_client_show_lab1);//将标签增加到布局中


        QSpacerItem* horizontalSpacer_9 = new QSpacerItem(18, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);//新的水平弹簧控件
        horizontalLayout_7->addItem(horizontalSpacer_9);//将弹簧控件增加到布局中

        client_label_infor.ui_client_show_lab2  = new QLabel(ui_client_show_widget1);
        //client_label_infor.ui_client_show_lab2->setObjectName(QString::fromUtf8("ui_client_show_lab2"));//新的窗体一上放置一个标签控件
        client_label_infor.ui_client_show_lab2->setAlignment(Qt::AlignCenter);
        temp = QString("%1%2").arg("show image for client ").arg(i);
        client_label_infor.ui_client_show_lab2 ->setText(temp);
        horizontalLayout_7->addWidget(client_label_infor.ui_client_show_lab2);//将标签增加到布局中

        horizontalLayout_7->setStretch(0, 20);//设置布局中控件的比例
        horizontalLayout_7->setStretch(1, 1);
        horizontalLayout_7->setStretch(2, 20);

        verticalLayout->addWidget(ui_client_show_widget1);//将新的窗体放到垂直布局中

        QSpacerItem* verticalSpacer_7 = new QSpacerItem(20, 4, QSizePolicy::Minimum, QSizePolicy::Expanding);//垂直弹簧
        verticalLayout->addItem(verticalSpacer_7);//将垂直弹簧放到垂直布局中

        QWidget* ui_client_show_widget2 = new QWidget(tab[i]);//新的窗体二
        //ui_client_show_widget2->setObjectName(QString::fromUtf8("ui_client_show_widget2"));
        QHBoxLayout* horizontalLayout_8 = new QHBoxLayout(ui_client_show_widget2);//在新的窗体一上放置水平布局
        horizontalLayout_8->setSpacing(0);
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        horizontalLayout_8->setContentsMargins(0, 0, 0, 0);

        client_label_infor.ui_client_show_lab3 = new QLabel(ui_client_show_widget2);//新的窗体一上放置一个标签控件
        //label[2]->setObjectName(QString::fromUtf8("ui_client_show_lab3"));
        client_label_infor.ui_client_show_lab3->setAlignment(Qt::AlignCenter);
        temp = QString("%1%2").arg("show image for client ").arg(i);
        client_label_infor.ui_client_show_lab3 ->setText(temp);
        horizontalLayout_8->addWidget(client_label_infor.ui_client_show_lab3);//将标签增加到布局中

        QSpacerItem* horizontalSpacer_10 = new QSpacerItem(18, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);//新的水平弹簧控件
        horizontalLayout_8->addItem(horizontalSpacer_10);//将弹簧控件增加到布局中

        client_label_infor.ui_client_show_lab4 = new QLabel(ui_client_show_widget2);//新的窗体一上放置一个标签控件
        //client_label_infor.ui_client_show_lab4->setObjectName(QString::fromUtf8("ui_client_show_lab4"));
        client_label_infor.ui_client_show_lab4->setAlignment(Qt::AlignCenter);
        temp = QString("%1%2").arg("show image for client ").arg(i);
        client_label_infor.ui_client_show_lab4 ->setText(temp);
        horizontalLayout_8->addWidget(client_label_infor.ui_client_show_lab4);//将标签增加到布局中

        labList.append(client_label_infor);

        horizontalLayout_8->setStretch(0, 20);//设置布局中控件的比例
        horizontalLayout_8->setStretch(1, 1);
        horizontalLayout_8->setStretch(2, 20);

        verticalLayout->addWidget(ui_client_show_widget2);//将新的窗体放到垂直布局中
        verticalLayout->setStretch(0, 40);//设置布局中控件的比例
        verticalLayout->setStretch(1, 1);
        verticalLayout->setStretch(2, 40);

        ui->tabWidget->insertTab(i,tab[i],tab_obj_list.at(i));
        ui->tabWidget->show();
    }

    ui->tabWidget->setCurrentIndex(0);

    //子界面对象删除
    delete system_setting_obj;
    system_setting_obj = NULL;
    delete log_show_obj;
    log_show_obj = NULL;
    delete image_capture_obj;
    image_capture_obj = NULL;
    delete algorithm_setting_obj;
    algorithm_setting_obj = NULL;
}

int MainWindow::get_camera_num_online(int client_num)
{
    int camera_num = 0;
    for(int i=0;i<client_num;i++)
        camera_num += clientList.at(i)->camera_num;

    return camera_num;
}

void MainWindow::draw_image_info_for_video(int client_num, int camera_num)
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
     for(int j=0;j<camera_num;j++) {
         //保存成jpg文件  根据报警信息
         QString fileName = QDir::currentPath();
         fileName += "/result/image/client";
         QString image_path = QString("%1%2%3%4%5%6%7").arg(fileName).arg(client_num + 1).arg("/camera").arg(j).arg("_image").arg(taskResult_info.number_frame[j]).arg(".jpg");
         qDebug() << "image_path:" <<  image_path <<Qt::endl;


         QPixmap map(image_path);
         if(j == 0) {
             labList.at(client_num).ui_client_show_lab1->setPixmap(map);
             labList.at(client_num).ui_client_show_lab1->setScaledContents(true);
             labList.at(client_num).ui_client_show_lab1->resize(labList.at(client_num).ui_client_show_lab1->width(),labList.at(client_num).ui_client_show_lab1->height());
         }
         else if(j == 1) {
             labList.at(client_num).ui_client_show_lab2->setPixmap(map);
             labList.at(client_num).ui_client_show_lab2->setScaledContents(true);
             labList.at(client_num).ui_client_show_lab2->resize(labList.at(client_num).ui_client_show_lab1->width(),labList.at(client_num).ui_client_show_lab1->height());
         }
         else if(j == 2) {
             labList.at(client_num).ui_client_show_lab3->setPixmap(map);
             labList.at(client_num).ui_client_show_lab3->setScaledContents(true);
             labList.at(client_num).ui_client_show_lab3->resize(labList.at(client_num).ui_client_show_lab1->width(),labList.at(client_num).ui_client_show_lab1->height());
         }
         else if(j == 3) {
             labList.at(client_num).ui_client_show_lab4->setPixmap(map);
             labList.at(client_num).ui_client_show_lab4->setScaledContents(true);
             labList.at(client_num).ui_client_show_lab4->resize(labList.at(client_num).ui_client_show_lab1->width(),labList.at(client_num).ui_client_show_lab1->height());
         }
     }//end camera index

}

void MainWindow::show_newpwd_ui()
{
    //删除配置文件
    QString fileName = QDir::currentPath();
    fileName += "/result/ini/net.ini";
    QFile file;
    file.remove(fileName);

    //删除定时器对象
    timer->stop();
    delete timer;
    timer = NULL;

    //客户端界面信息清理
    for(int i=0;i<clientList.size();i++) {
        delete tab[i];
        tab[i] = NULL;
    }
    labList.clear();
    tab_obj_list.clear();

    //子界面对象删除
    delete system_setting_obj;
    system_setting_obj = NULL;
    delete log_show_obj;
    log_show_obj = NULL;
    delete image_capture_obj;
    image_capture_obj = NULL;
    delete algorithm_setting_obj;
    algorithm_setting_obj = NULL;

    //删除客户端信息
    clientList.clear();
    //断开与客户端的连接
    for(int i=TcpSocketList.size() - 1;i>=0;i--)
        TcpSocketList.at(i)->TcpClientSocket->disconnectFromHost();
    TcpSocketList.clear();

    //关闭本界面
    this->close();

    //返回登录窗口
    rotate->back_activity_win();
}

void MainWindow::on_mainWindow_alarm_info_btn_clicked()
{

}

