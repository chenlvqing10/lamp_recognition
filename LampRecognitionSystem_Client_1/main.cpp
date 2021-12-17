#include "qt_ui/ui_lamp_mainwindow.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    qDebug() << "主程序启动" << endl;
    QApplication a(argc, argv);

    qDebug() << "主界面启动" << endl;
    ui_lamp_mainWindow w;
    w.show();

    return a.exec();
}
