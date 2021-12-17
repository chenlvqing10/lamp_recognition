#ifndef UI_LAMP_MAINWINDOW_H
#define UI_LAMP_MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class ui_lamp_mainWindow; }
QT_END_NAMESPACE

class ui_lamp_mainWindow : public QMainWindow
{
    Q_OBJECT

public:
    ui_lamp_mainWindow(QWidget *parent = nullptr);
    ~ui_lamp_mainWindow();

private:
    Ui::ui_lamp_mainWindow *ui;
};
#endif // UI_LAMP_MAINWINDOW_H
