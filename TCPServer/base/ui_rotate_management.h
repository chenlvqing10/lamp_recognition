#ifndef UI_ROTATE_MANAGEMENT_H
#define UI_ROTATE_MANAGEMENT_H

#include "base/wins_management.h"

class ui_rotate_management : public wins_management
{
private:
  explicit ui_rotate_management(QObject *parent = nullptr);
  ~ui_rotate_management();
public:
    static ui_rotate_management *getInstance();
    int start_activity_win(const QMetaObject *aim_win);

     QtMainWindow *get_current_win() {
       return currnet_win;
     }

public slots:
     void back_activity_win();


};

#endif // UI_ROTATE_MANAGEMENT_H
