#include "wins_management.h"

wins_management::wins_management(QObject *parent) :
    QObject(parent)
{
    currnet_win = NULL;
    wins_list.clear();
}

wins_management::~wins_management()
{
    if (currnet_win) {
        currnet_win->deinit();
        currnet_win = NULL;
      }
      wins_list.clear();
}

int wins_management::start_activity_win(const QMetaObject *aim_win)
{
    const char *className;
    //如果已经在本界面了
    if (currnet_win && strcmp(currnet_win->metaObject()->className(),
        aim_win->className()) == 0)
        return 1;

    if (currnet_win) {
        currnet_win->deinit();
        wins_list.append(currnet_win->metaObject());
        className = currnet_win->metaObject()->className();
    }

    className = aim_win->className();
    //新建一个新的界面对象实例
    currnet_win = static_cast<QtMainWindow *>(aim_win->newInstance());
    currnet_win->setClassName();
    if (currnet_win) {
        //currnet_win->showMaximized();
        currnet_win->show();
    }
    return 0;
}

void wins_management::back_activity_win()
{
   const QMetaObject *last_win;
   if (!wins_list.isEmpty()) {//如果界面列表不是空的
       last_win = wins_list.last();//得到界面列表中最后一个界面对象
       wins_list.pop_back();
       if (currnet_win)
         currnet_win->deinit();
       //新建一个新的界面对象实例
       currnet_win = static_cast<QtMainWindow *>(last_win->newInstance());
       currnet_win->setClassName();
       if (currnet_win) {
         //currnet_win->showMaximized();
         currnet_win->show();
       }
     }
}

