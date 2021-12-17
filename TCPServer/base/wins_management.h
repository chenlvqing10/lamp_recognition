#ifndef WINS_MANAGEMENT_H
#define WINS_MANAGEMENT_H

#include <QMainWindow>
#include <QObject>
#include "base/Qtmainwindow.h"

class wins_management:public QObject
{
    Q_OBJECT
public:
  explicit wins_management(QObject *parent = nullptr);
  ~wins_management();
  int start_activity_win(const QMetaObject *aim_win);

  QtMainWindow *get_current_win() {
        return currnet_win;
  }

public slots:
 void back_activity_win();

protected:
  QtMainWindow *currnet_win;
  QList<const QMetaObject*> wins_list;
};

#endif // WINS_MANAGEMENT_H
