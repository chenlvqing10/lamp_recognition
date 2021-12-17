#include "ui_lamp_mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ui_lamp_mainWindow w;
    w.show();
    return a.exec();
}
