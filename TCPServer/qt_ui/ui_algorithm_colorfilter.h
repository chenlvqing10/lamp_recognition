#ifndef UI_ALGORITHM_COLORFILTER_H
#define UI_ALGORITHM_COLORFILTER_H

#include <QMainWindow>
#include <QStandardItemModel>
#include "base/serverbase.h"

#define LOCATION_OFFSET 10000
#define MAX(i,j) (i>j)?i:j

typedef struct button_Point{
    int x;      //横坐标
    int y;      //纵坐标
    int width;
    int height;
    int sum;    //横坐标+归一化纵坐标*偏移系数=坐标系位置偏移值
    int location_index;
}button_Point_t;

typedef struct imgcut_Point{
    double org_width;     //原图宽
    double org_height;    //原图长
    double x;             //截图起始横坐标
    double y;             //截图起始坐坐标
    double width;         //截图框的宽度
    double height;        //截图框的长度
}imgcut_Point_t;

typedef struct hsv_color_info{
    char  lamp_color[20];//指示灯颜色
    int   hmin;//该指示灯颜色HSV控件阈值
    int   hmax;
    int   smin;
    int   smax;
    int   vmin;
    int   vmax;
}hsv_color_info_t;

typedef struct lamp_info_out {
    char  lamp_color[20];//指示灯颜色
    int   lamp_color_num;//该颜色指示灯数量
    QList<button_Point_t*> button_info_List;
}lamp_info_out_t;

typedef struct area_info {
    int area_min;
    int area_max;
}area_info_t;

//需要发送给客户端的信息
typedef struct algorithm_info {
    unsigned char algorithm_name[50];
    unsigned char algorithm_version[50];
    int           algorithm_id;
    unsigned short algorithm_train_res;//算法测试结果
    unsigned char  client_name[50];//对应哪一个客户端
    unsigned char  clinet_uuid[50];
    int            camera_id;
    unsigned char  camera_device_name[50];//对应哪一个摄像头
}algorithm_info_t;


typedef struct color_filter_model {
    bool                is_resize;      //是否缩放
    int                 color_num;      //该指示灯设备指示灯的颜色数量
    int                 time_offset;    //时间标记偏移
    area_info_t         area_info;      //面积过滤参数值
    imgcut_Point_t      imgcut_info;    //截图坐标
    algorithm_info_t    algorithmInfo;  //指示灯识别的算法信息
    QList<hsv_color_info_t*>  hsv_color_infoList;      //模型输出指示灯识别参数 轮廓坐标 颜色 该颜色指示灯数量
}color_filter_model_t ;


namespace Ui {
class ui_algorithm_colorFilter;
}

class ui_algorithm_colorFilter : public QMainWindow
{
    Q_OBJECT

public:
    explicit ui_algorithm_colorFilter(QWidget *parent = nullptr);
    ~ui_algorithm_colorFilter();

private slots:
    void on_ui_colorFilter_open_btn_clicked();
    void on_ui_colorFilter_close_btn_clicked();
    void on_ui_colorFilter_add_btn_2_clicked();
    void on_ui_colorFilter_match_add_btn_clicked();
    void on_ui_colorFilter_match_cancel_btn_clicked();
    void on_ui_colorFilter_algorithm_train_btn_clicked();
    void on_ui_colorFilter_cancel_btn_clicked();
    void on_ui_colorFilter_algorithm_compare_train_btn_clicked();
    void on_ui_colorFilter_model_sync_client_btn_clicked();

public slots:
    void slot_sendto_algorithm_colorFilte(imgcut_info_t* imgcut_info);
    void slot_clear_model_info();
private:
    Ui::ui_algorithm_colorFilter *ui;
    QStandardItemModel* ItemModel;
    QStandardItemModel* ItemModel_compare;
    bool is_template_in_database(QString& client_name,QString& camera_name,QString& algorithm_name,QString& algorithm_version);
    int get_model_info_from_database(QString& client_uuid,int camera_id,int algorithm_id);
private:
    bool filter_flag;
    bool database_flag;
    QString img_org_path;
    QString img_cut_path;
    QStringList  colorinfoList;
    QStringList  lampCompareList;
    int save_to_database(lamp_color_template_info_t* lamp_color_template_info,color_filter_model_t* color_filter_model,
                          QString& client_uuid,int camera_id,int algorithm_id);
};

#endif // UI_ALGORITHM_COLORFILTER_H
