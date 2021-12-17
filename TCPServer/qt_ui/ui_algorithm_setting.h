#ifndef UI_ALGORITHM_SETTING_H
#define UI_ALGORITHM_SETTING_H

#include <QMainWindow>
#include "ui_algorithm_colorfilter.h"
#include "ui_algorithm_brightnessfilter.h"
#include <QImage>
#include "base/serverbase.h"
#include <QTimer>

//实现鼠标响应裁剪框
typedef struct myPoint {
    double x;
    double y;
}myPoint_t;

//定义截图参数结构体
typedef struct roiPoint
{
    myPoint_t   PointStart;   //截图起始坐标
    myPoint_t   PointEnd;     //截图结束坐标
    myPoint_t   PointRoi;     //截图框起始坐标 根据截图方向的不同而不同
    double      width;         //截图框的宽度
    double      height;        //截图框的长度
    double      ButtonAread;   //指示灯面积
}roiPoint_t;



QT_BEGIN_NAMESPACE
namespace Ui { class ui_algorithm_setting; }
QT_END_NAMESPACE

class ui_algorithm_setting : public QMainWindow
{
    Q_OBJECT

public:
    ui_algorithm_setting(QWidget *parent = nullptr);
    ~ui_algorithm_setting();

private slots:
    void on_ui_algorithm_setting_template_open_btn_clicked();
    void on_ui_algorithm_setting_template_cut_btn_clicked();
    void on_ui_algorithm_setting_algorithm_test_btn_clicked();
    void on_ui_algorithm_setting_algorithm_type_cmbx_currentIndexChanged(int index);
    void on_ui_algorithm_setting_algorithm_version_cmbx_currentIndexChanged(int index);
    void on_ui_algorithm_setting_client_cmbx_currentTextChanged(const QString &arg1);
    void on_ui_algorithm_setting_client_cmbx_currentIndexChanged(int index);
    void on_ui_algorithm_setting_camera_cmbx_currentIndexChanged(int index);
    void on_ui_algorithm_setting_algorithm_clear_btn_clicked();
    int check_model_in_database(QString client_name,QString camera_name,QString algorithm_name,QString algorithm_version,int index);
    void slot_reshow_widget();
signals:
    void sig_sendto_algorithm_colorFilte(imgcut_info_t*);
    void sig_clear_model_info_color();
    void sig_clear_model_info_brightness();
    void sig_sendto_algorithm_brightnessfilter(imgcut_info_t*);
private:
    Ui::ui_algorithm_setting *ui;;
    ui_algorithm_colorFilter* algorithm_colorFilter;
    ui_algorithm_brightnessfilter* algorithm_brightnessfilter;
    //QImage cvMat2QImage(const cv::Mat &mat);
public:
    void on_mouse(int event, int x, int y, int flags, void* userdata);
    void setimgPoint(double x,double y,double w,double h,QString& dir);

private:
    bool m_database_flag;
    QString m_client_uuid;
    int m_camera_id;
    int m_algorithm_id;

    bool    m_is_resize;
    int     m_img_org_width;
    int     m_img_org_height;
    QString m_org_path;
    QString m_save_path;

    QTimer* timer;

};
#endif // UI_ALGORITHM_SETTING_H
