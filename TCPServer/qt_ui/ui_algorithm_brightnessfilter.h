#ifndef UI_ALGORITHM_BRIGHTNESSFILTER_H
#define UI_ALGORITHM_BRIGHTNESSFILTER_H

#include <QMainWindow>
#include "base/eventbase.h"

//需要发送给客户端的信息
typedef struct brightness_algorithm_info {
    unsigned char algorithm_name[50];
    unsigned char algorithm_version[50];
    int           algorithm_id;
    unsigned short algorithm_train_res;//算法测试结果
    unsigned char  client_name[50];//对应哪一个客户端
    unsigned char  clinet_uuid[50];
    int            camera_id;
    unsigned char  camera_device_name[50];//对应哪一个摄像头
}brightness_algorithm_info_t;

typedef struct brightness_imgcut_Point{
    double org_width;     //原图宽
    double org_height;    //原图长
    double x;             //截图起始横坐标
    double y;             //截图起始坐坐标
    double width;         //截图框的宽度
    double height;        //截图框的长度
}brightness_imgcut_Point_t;

typedef struct brightness_button_Point{
    int x;      //横坐标
    int y;      //纵坐标
    int cols_index; //列索引
    int row_index;  //行索引
}brightness_button_Point_t;

typedef  struct brightness_button
;

typedef struct brightness_lamp_info_out {
    int  lamp_status;       //指示灯开关状态 1:亮  0:灭
    int  lamp_num;          //指示灯开关亮灭数量
    QList<brightness_button_Point_t*> button_info_List;  //指示灯位置坐标
}brightness_lamp_info_out_t;

typedef struct brightness_filter_model {
    bool    is_resize;      //是否缩放
    int     maskVaule;      //二值化阈值
    int     num_row;        //行数
    int     num_cols;       //列数
    int     offset;         //位置偏移值
    int     off_x_offset;    //off标记x偏移值
    int     off_y_offset;    //off标y记偏移值
    int     time_offset;    //时间标记偏移值

    brightness_imgcut_Point_t      imgcut_info;    //截图坐标
    brightness_algorithm_info_t    algorithmInfo;  //指示灯识别的算法信息

}brightness_filter_model_t ;//模型 输入参数


namespace Ui {
class ui_algorithm_brightnessfilter;
}

class ui_algorithm_brightnessfilter : public QMainWindow
{
    Q_OBJECT

public:
    explicit ui_algorithm_brightnessfilter(QWidget *parent = nullptr);
    ~ui_algorithm_brightnessfilter();

private slots:
    void on_ui_algorithm_brightnessfilter_train_btn_clicked();
    void on_ui_algorithm_brightnessfilter_read_result_btn_clicked();
    void on_ui_algorithm_brightnessfilter_test_btn_clicked();

    void on_ui_algorithm_brightnessfilter_send_to_client_btn_clicked();

public slots:
    void slot_sendto_algorithm_brightnessfilter(imgcut_info_t *imgcut_info);
    void slot_clear_model_info();
private:
    Ui::ui_algorithm_brightnessfilter *ui;
    bool m_database_flag;
    QString m_img_org_path;
    QString m_img_cut_path;
    int save_to_database(brightness_template_info_t* brightness_template_info,brightness_filter_model_t* brightness_filter_model_info,
                          QString& client_uuid,int camera_id,int algorithm_id);
};

#endif // UI_ALGORITHM_BRIGHTNESSFILTER_H
