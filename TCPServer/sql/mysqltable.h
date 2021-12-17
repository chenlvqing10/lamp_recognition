#ifndef MYSQLTABLE_H
#define MYSQLTABLE_H
#include <QObject>
#include <QSqlTableModel>

class MySqlTable :public QSqlTableModel
{
    Q_OBJECT
public:
    MySqlTable();
     QVariant data(const QModelIndex &index, int role=Qt::DisplayRole) const;
};

#endif // MYSQLTABLE_H
