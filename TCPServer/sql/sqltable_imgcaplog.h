#ifndef SQLTABLE_IMGCAPLOG_H
#define SQLTABLE_IMGCAPLOG_H

#include <QObject>
#include <QSqlTableModel>

class SqlTable_ImgCapLog :public QSqlTableModel
{
    Q_OBJECT
public:
    SqlTable_ImgCapLog();
    QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;
};

#endif // SQLTABLE_IMGCAPLOG_H
