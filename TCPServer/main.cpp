#include "qt_ui/mainwindow.h"
#include <QApplication>
#include <QFile>
#include "qt_ui/ui_login.h"
#include "base/serverbase.h"
#include "base/ui_rotate_management.h"
#include "sql/sqlbase.h"
#include <QDebug>
#include <QMessageBox>
static ui_rotate_management *rotate;
extern dbConnect_info_t m_dbconnect_info;

void qss_init(QApplication &a)
{
    QString qss;
    QFile qssFile(":/res/resource/qss/title.css");
    qssFile.open(QFile::ReadOnly);
    if(qssFile.isOpen())
    {
        qss = QLatin1String(qssFile.readAll());
        qApp->setStyleSheet(qss);
        qssFile.close();
    }

    QPixmap pixmap(":/res/resource/image/bk1.jpg");
    QPalette p = a.palette();
    p.setBrush(QPalette::Window, QBrush(pixmap));
    a.setPalette(p);
}

void server_init()
{
    ServerBase serverbase;
    serverbase.server_init();
}

int db_init()
{
    sqlBase sqlbase;
    bool ret_conn = sqlbase.createConnect();
    if(ret_conn == false) {
        return -1;
    }
    else {
        qDebug() << "数据库连接成功" << Qt::endl;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //style init
    qss_init(a);

    //init db
    int ret = db_init();
    if(ret < 0){
        QMessageBox::information(0,"提示", "数据库连接失败!!");
        return -1;
     }

    server_init();

    //qDebug() << "aaaaaaaaaaa" <<Qt::endl;

    /* Show the first UI */
    rotate = ui_rotate_management::getInstance();
    rotate->start_activity_win(&ui_login::staticMetaObject);

    return a.exec();
}
