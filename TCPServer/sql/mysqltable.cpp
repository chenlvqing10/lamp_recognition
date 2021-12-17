#include "mysqltable.h"
#include <QDebug>
#include "base/eventbase.h"
#include <QDateTime>

MySqlTable::MySqlTable()
{

}

QVariant MySqlTable::data(const QModelIndex &index, int role) const
{
    QVariant value=QSqlTableModel::data(index,role);
    if(role==Qt::DisplayRole && ((index.column()==3) || (index.column() ==5)))//重写第三列 起始事件方向
    {
        switch (value.toInt()) {
            case EVENT_DIRECTION_SERVER_TO_CLIENT:
                value = "服务器->客户端";
                break;
            case EVENT_DIRECTION_CLIENT_TO_SERVER:
                value = "客户端->服务器";
                break;
            case EVENT_DIRECTION_SERVER_TO_APP:
                value = "服务器->APP";
                break;
            case EVENT_DIRECTION_APP_TO_SERVER:
                value = "APP->服务器";
                break;
            case EVENT_DIRECTION_SERVER_ONLY:
                value = "服务器单向";
                break;
            case EVENT_DIRECTION_CLIENT_ONLY:
                value = "客户端单向";
                break;
            case EVENT_DIRECTION_APP_ONLY:
                value = "APP单向";
                break;
            default:
                value = "";
                break;
        }
            return value;
    }

    if(role==Qt::DisplayRole &&index.column()==7){//重写第7列 日期
        long long int ms = value.toLongLong();
        QDateTime time = QDateTime::fromMSecsSinceEpoch(ms);
        value = time.toString("yyyy-MM-dd hh:mm:ss");
        return value;
    }

    return value;
}
