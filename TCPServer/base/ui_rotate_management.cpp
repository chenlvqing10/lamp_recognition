#include "ui_rotate_management.h"
#include <QtDebug>

static ui_rotate_management *instance = nullptr;

ui_rotate_management::ui_rotate_management(QObject *parent):wins_management(parent)
{

}

ui_rotate_management::~ui_rotate_management()
{

}

//获取界面的对象是咧
ui_rotate_management *ui_rotate_management::getInstance()
{
    if (instance == nullptr) {
        /* new a ui_rotate_management object */
        instance = new ui_rotate_management();
    }
    return instance;
}

int ui_rotate_management::start_activity_win(const QMetaObject *aim_win)
{
    const char *className;
    int ret = 0;
    ret = wins_management::start_activity_win(aim_win);
    if(ret) {
        qDebug() << "start_activity_win failed\n" <<Qt::endl;
        return ret;
    }

    return 0;
}

void ui_rotate_management::back_activity_win()
{
    const char *className;
    if (!wins_list.isEmpty()) {
        className = currnet_win->metaObject()->className();
        qDebug() << "className:" << className <<Qt::endl;
        wins_management::back_activity_win();
     }
}

