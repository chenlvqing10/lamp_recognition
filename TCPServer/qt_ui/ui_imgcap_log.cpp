#include "ui_imgcap_log.h"
#include "ui_ui_imgcap_log.h"
#include <QSqlTableModel>
#include "sql/sqltable_imgcaplog.h"
#include <QMessageBox>
ui_imgcap_log::ui_imgcap_log(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ui_imgcap_log)
{
    ui->setupUi(this);
    ui_imgcap_log_init();


}

ui_imgcap_log::~ui_imgcap_log()
{
    delete ui;
}

void ui_imgcap_log::ui_imgcap_log_init()
{
    ui->dateTimeEdit_starttime->setCalendarPopup(true);
    ui->dateTimeEdit_starttime->setDateTime(QDateTime::currentDateTime());
    ui->dateTimeEdit_endtime->setCalendarPopup(true);
    ui->dateTimeEdit_endtime->setDateTime(QDateTime::currentDateTime());
    //QSqlTableModel *sql_table_model = new QSqlTableModel; //负责提取数据
    SqlTable_ImgCapLog *sql_table_model = new SqlTable_ImgCapLog();
    sql_table_model->setTable("cap_record_info");
    sql_table_model->select();
    ui->tableView->setModel(sql_table_model);
    sql_table_model->setHeaderData(0,Qt::Horizontal,"抓拍记录编号");
    sql_table_model->setHeaderData(1,Qt::Horizontal,"用户编号");
    sql_table_model->setHeaderData(2,Qt::Horizontal,"客户端UUID");
    sql_table_model->setHeaderData(3,Qt::Horizontal,"相机设备名");
    sql_table_model->setHeaderData(4,Qt::Horizontal,"抓取时间");
    sql_table_model->setHeaderData(5,Qt::Horizontal,"保存路径");

}

void ui_imgcap_log::on_ui_imgcap_log_select_btn_clicked()
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

    //设置查询的过滤条件 相当于where语句
    SqlTable_ImgCapLog *sql_table_model = new SqlTable_ImgCapLog();
    sql_table_model->setTable("cap_record_info");
    QString  limit = "cap_timestamp > " + QString::number(ms_start) + " AND cap_timestamp < " + QString::number(ms_end);
    //qDebug() << "limit::" << limit << Qt::endl;
    sql_table_model->setFilter(limit);
    sql_table_model->select();
    ui->tableView->setModel(sql_table_model);
}
