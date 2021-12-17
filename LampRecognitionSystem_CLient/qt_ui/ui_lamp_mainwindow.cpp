#include "ui_lamp_mainwindow.h"
#include "ui_ui_lamp_mainwindow.h"

ui_lamp_mainWindow::ui_lamp_mainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ui_lamp_mainWindow)
{
    ui->setupUi(this);
}

ui_lamp_mainWindow::~ui_lamp_mainWindow()
{
    delete ui;
}

