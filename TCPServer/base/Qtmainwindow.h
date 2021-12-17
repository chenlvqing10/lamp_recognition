#ifndef QTMAINWINDOW_H
#define QTMAINWINDOW_H

#include <QMainWindow>
#include <QObject>
#include <QWidget>

/* 子类化QMainWindow */
class QtMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    Q_INVOKABLE explicit QtMainWindow(QWidget *parent = 0);
    ~QtMainWindow();

    void sendSignalForMainUI(int event, int value);

    virtual void setClassName() {
        strncpy(className, metaObject()->className(),
          sizeof(className) - 1);
    }

public slots:
      virtual void deinit();

protected slots:
      virtual void slot_for_mainui(int event, int value);

signals:
      void signalForMainUI(int event, int value);

private:
      char className[256];
};

#endif // QTMAINWINDOW_H
