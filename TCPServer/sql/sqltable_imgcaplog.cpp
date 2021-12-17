#include "sqltable_imgcaplog.h"
#include <QDateTime>

SqlTable_ImgCapLog::SqlTable_ImgCapLog()
{

}

QVariant SqlTable_ImgCapLog::data(const QModelIndex &index, int role) const
{
     QVariant value=QSqlTableModel::data(index,role);
     if(role==Qt::DisplayRole &&index.column()==4){//重写第4列 日期
         long long int ms = value.toLongLong();
         QDateTime time = QDateTime::fromMSecsSinceEpoch(ms);
         value = time.toString("yyyy-MM-dd hh:mm:ss");
         return value;
     }
}
