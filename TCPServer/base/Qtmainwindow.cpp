#include "Qtmainwindow.h"


QtMainWindow::QtMainWindow(QWidget *parent):QMainWindow(parent)
{
    memset(className, 0, sizeof(className));
    connect(this, SIGNAL(signalForMainUI(int, int)),
            this, SLOT(slot_for_mainui(int, int)));
}

QtMainWindow::~QtMainWindow()
{

}

void QtMainWindow::sendSignalForMainUI(int event, int value)
{
    emit signalForMainUI(event, value);
}

void QtMainWindow::deinit()
{
    disconnect(SIGNAL(signalForMainUI(int, int)));
    this->deleteLater();
}

void QtMainWindow::slot_for_mainui(int event, int value)
{

}
