#include "ui_log_show.h"
#include "ui_ui_log_show.h"
#include <QDebug>
#include <QSqlTableModel>
#include <QSqlQuery>
#include <QTableView>
#include <QModelIndex>
#include "sql/mysqltable.h"
#include <QMessageBox>

QList<system_log_info_t> system_log_infoList;

ui_log_show::ui_log_show(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ui_log_show)
{
    ui->setupUi(this);
    ui_log_show_init();
}

ui_log_show::~ui_log_show()
{
    delete ui;

}


void ui_log_show::on_ui_log_show_event_type_cmbx_currentTextChanged(const QString &arg1)
{
    qDebug() << "arg1::" << arg1 << Qt::endl;

     MySqlTable *sql_table_model = new MySqlTable();
     sql_table_model->setTable("system_log_info");

     if(arg1 == "客户端连接事件")
         sql_table_model->setFilter("event_type_no = 1001");

     if(arg1 == "app绑定事件")
         sql_table_model->setFilter("event_type_no = 1002");

     if(arg1 == "服务器图片抓拍事件")
         sql_table_model->setFilter("event_type_no = 1003");

     if(arg1 == "app图片抓拍事件")
         sql_table_model->setFilter("event_type_no = 1004");

     if(arg1 == "客户端报警事件")
         sql_table_model->setFilter("event_type_no = 1009");

     if(arg1 == "用户登录事件")
         sql_table_model->setFilter("event_type_no = 1023");

     if(arg1 == "用户注册事件")
         sql_table_model->setFilter("event_type_no = 1022");

     if(arg1 == "用户密码重置事件")
         sql_table_model->setFilter("event_type_no = 1024");

     if(arg1 == "服务器视频封装保存事件")
         sql_table_model->setFilter("event_type_no = 1021");

     if(arg1 == "全部")
         sql_table_model->setFilter("");

      sql_table_model->select();
      ui->tableView->setModel(sql_table_model);

      sql_table_model->setHeaderData(0,Qt::Horizontal,"记录编号");
      sql_table_model->setHeaderData(1,Qt::Horizontal,"用户编号");
      sql_table_model->setHeaderData(2,Qt::Horizontal,"事件类别编号");
      sql_table_model->setHeaderData(3,Qt::Horizontal,"起始事件方向");
      sql_table_model->setHeaderData(4,Qt::Horizontal,"起始事件应答结果");
      sql_table_model->setHeaderData(5,Qt::Horizontal,"触发事件方向");
      sql_table_model->setHeaderData(6,Qt::Horizontal,"触发事件应答结果");
      sql_table_model->setHeaderData(7,Qt::Horizontal,"日志发生时间");
      sql_table_model->setHeaderData(8,Qt::Horizontal,"用户登录事件");
}

void ui_log_show::on_ui_log_show_event_type_select_btn_clicked()
{
    QDateTime start_datatiem = ui->dateTimeEdit_starttime->dateTime();
    QDateTime end_datatiem = ui->dateTimeEdit_endtime->dateTime();
    long long int ms_start = start_datatiem.toMSecsSinceEpoch();
    long long int ms_end = end_datatiem.toMSecsSinceEpoch();
    //qDebug() << "ms_start::" << ms_start << Qt::endl;
    //qDebug() << "ms_end::" << ms_end << Qt::endl;

    if(ms_start > ms_end) {
          QMessageBox::information(this,"提示", "起始时间不能大于结束时间！");
          return;
    }

    MySqlTable *sql_table_model = new MySqlTable();
    sql_table_model->setTable("system_log_info");
    QString  limit = "event_time > " + QString::number(ms_start) + " AND event_time < " + QString::number(ms_end);
    //qDebug() << "limit::" << limit << Qt::endl;
    sql_table_model->setFilter(limit);
    sql_table_model->select();
    ui->tableView->setModel(sql_table_model);

    sql_table_model->setHeaderData(0,Qt::Horizontal,"记录编号");
    sql_table_model->setHeaderData(1,Qt::Horizontal,"用户编号");
    sql_table_model->setHeaderData(2,Qt::Horizontal,"事件类别编号");
    sql_table_model->setHeaderData(3,Qt::Horizontal,"起始事件方向");
    sql_table_model->setHeaderData(4,Qt::Horizontal,"起始事件应答结果");
    sql_table_model->setHeaderData(5,Qt::Horizontal,"触发事件方向");
    sql_table_model->setHeaderData(6,Qt::Horizontal,"触发事件应答结果");
    sql_table_model->setHeaderData(7,Qt::Horizontal,"日志发生时间");
    sql_table_model->setHeaderData(8,Qt::Horizontal,"用户登录事件");
}

void ui_log_show::ui_log_show_init()
{
    ui->ui_log_show_event_type_cmbx->addItem("全部");
    ui->ui_log_show_event_type_cmbx->addItem("客户端连接事件");
    ui->ui_log_show_event_type_cmbx->addItem("app绑定事件");
    ui->ui_log_show_event_type_cmbx->addItem("服务器图片抓拍事件");
    ui->ui_log_show_event_type_cmbx->addItem("app图片抓拍事件");
    ui->ui_log_show_event_type_cmbx->addItem("客户端报警事件");
    ui->ui_log_show_event_type_cmbx->addItem("用户登录事件");
    ui->ui_log_show_event_type_cmbx->addItem("用户注册事件");
    ui->ui_log_show_event_type_cmbx->addItem("用户密码重置事件");
    ui->ui_log_show_event_type_cmbx->addItem("服务器视频封装保存事件");

    ui->dateTimeEdit_starttime->setCalendarPopup(true);
    ui->dateTimeEdit_starttime->setDateTime(QDateTime::currentDateTime());
    ui->dateTimeEdit_endtime->setCalendarPopup(true);
    ui->dateTimeEdit_endtime->setDateTime(QDateTime::currentDateTime());

    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    //QSqlTableModel *sql_table_model = new QSqlTableModel; //负责提取数据
    MySqlTable *sql_table_model = new MySqlTable();
    sql_table_model->setTable("system_log_info");
    sql_table_model->select();
    ui->tableView->setModel(sql_table_model);

    sql_table_model->setHeaderData(0,Qt::Horizontal,"记录编号");
    sql_table_model->setHeaderData(1,Qt::Horizontal,"用户编号");
    sql_table_model->setHeaderData(2,Qt::Horizontal,"事件类别编号");
    sql_table_model->setHeaderData(3,Qt::Horizontal,"起始事件方向");
    sql_table_model->setHeaderData(4,Qt::Horizontal,"起始事件应答结果");
    sql_table_model->setHeaderData(5,Qt::Horizontal,"触发事件方向");
    sql_table_model->setHeaderData(6,Qt::Horizontal,"触发事件应答结果");
    sql_table_model->setHeaderData(7,Qt::Horizontal,"日志发生时间");
    sql_table_model->setHeaderData(8,Qt::Horizontal,"用户登录事件");
}
