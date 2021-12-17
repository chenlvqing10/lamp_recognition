#include "ui_algorithm_brightnessfilter.h"
#include "ui_ui_algorithm_brightnessfilter.h"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <fstream>
#include <iostream>

#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include <QStringList>
#include <QList>
#include <QFileDialog>
#include <QDateTime>

#include <QSql>
#include <QSqlQueryModel>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "sql/sqlbase.h"

#ifdef __cplusplus
extern "C" {
#include "base/cJSON.h"
}
#endif

using namespace cv;
using namespace std;

extern dbConnect_info_t m_dbconnect_info;           //数据库信息
extern QList<client_all_info_t*> clientList;        //管理客户端的列表
extern server_info_t server_info;
extern QList<client_net_info_t*>TcpSocketList;//管理客户端socket ip port

static brightness_filter_model_t brightness_filter_model_info;              //亮度过滤模型输入参数
static QList<brightness_lamp_info_out_t*> brightness_lamp_info_out_list;    //亮度模型输出参数
static QList<brightness_lamp_info_out_t*> brightness_lamp_info_result_List; //亮度结果集

ui_algorithm_brightnessfilter::ui_algorithm_brightnessfilter(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ui_algorithm_brightnessfilter)
{
    ui->setupUi(this);
    setWindowTitle("亮度过滤器算法训练");

    ui->ui_algorithm_brightnessfilter_cols_edt->setText("0");
    ui->ui_algorithm_brightnessfilter_row_edt->setText("0");
    ui->ui_algorithm_brightnessfilter_cols_edt->setText("0");
    ui->ui_algorithm_brightnessfilter_Maskvalue_edt->setText("200");
    ui->ui_algorithm_brightnessfilter_offset_edt->setText("0");
    ui->ui_algorithm_brightnessfilter_off_x_offset_edt->setText("15");
    ui->ui_algorithm_brightnessfilter_off_y_offset_edt->setText("0");
    ui->ui_algorithm_brightnessfilter_time_offset_edt->setText("100");
}

ui_algorithm_brightnessfilter::~ui_algorithm_brightnessfilter()
{
    delete ui;
    memset(&brightness_filter_model_info,0,
           sizeof(brightness_imgcut_Point_t)+sizeof(brightness_algorithm_info_t)
           +sizeof(int)*7+sizeof(bool));//初始化

}



void ui_algorithm_brightnessfilter::slot_sendto_algorithm_brightnessfilter(imgcut_info_t *imgcut_info)
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;

    memset(&brightness_filter_model_info,0,
           sizeof(brightness_imgcut_Point_t)+sizeof(brightness_algorithm_info_t)
           +sizeof(int)*7+sizeof(bool));//初始化

    qDebug() <<"是否存在数据库中::" << imgcut_info->database_flag;
    m_database_flag = imgcut_info->database_flag;

    qDebug() <<"原图宽::" << imgcut_info->imgcut_parms.org_width;
    brightness_filter_model_info.imgcut_info.org_width= imgcut_info->imgcut_parms.org_width;

    qDebug() <<"原图长::" << imgcut_info->imgcut_parms.org_height;
    brightness_filter_model_info.imgcut_info.org_height= imgcut_info->imgcut_parms.org_height;

    qDebug() <<"截图横坐标::" << imgcut_info->imgcut_parms.x;
    brightness_filter_model_info.imgcut_info.x = imgcut_info->imgcut_parms.x;

    qDebug() <<"截图纵坐标::" << imgcut_info->imgcut_parms.y;
     brightness_filter_model_info.imgcut_info.y = imgcut_info->imgcut_parms.y;

    qDebug() <<"截图宽::"    << imgcut_info->imgcut_parms.width;
     brightness_filter_model_info.imgcut_info.width = imgcut_info->imgcut_parms.width;

    qDebug() <<"截图高::"    << imgcut_info->imgcut_parms.height;
    brightness_filter_model_info.imgcut_info.height = imgcut_info->imgcut_parms.height;

    qDebug() <<"客户端名字::"  <<QString((char*)imgcut_info->client_name);
    strcpy((char*)brightness_filter_model_info.algorithmInfo.client_name,(char*)imgcut_info->client_name);
    qDebug() <<"客户端uuid::"  <<QString((char*)imgcut_info->client_uuid);
    strcpy((char*)brightness_filter_model_info.algorithmInfo.clinet_uuid,(char*)imgcut_info->client_uuid);

    qDebug() <<"相机名字::"   << QString((char*)imgcut_info->camera_device_name);
    strcpy((char*)brightness_filter_model_info.algorithmInfo.camera_device_name,(char*)imgcut_info->camera_device_name);
    qDebug() <<"相机id::"  << imgcut_info->camera_id;
    brightness_filter_model_info.algorithmInfo.camera_id = imgcut_info->camera_id;

    qDebug() <<"算法名字::"   << QString((char*)imgcut_info->algorithm_name);
    strcpy((char*)brightness_filter_model_info.algorithmInfo.algorithm_name,(char*)imgcut_info->algorithm_name);
    qDebug() <<"算法版本号::" << QString((char*)imgcut_info->algorithm_version);
    strcpy((char*)brightness_filter_model_info.algorithmInfo.algorithm_version,(char*)imgcut_info->algorithm_version);
    qDebug() <<"算法id::"  << imgcut_info->algorithm_id;
    brightness_filter_model_info.algorithmInfo.algorithm_id = imgcut_info->algorithm_id;

    qDebug() <<"原图路径::" << QString((char*)imgcut_info->org_path);
    m_img_org_path = QString((char*)imgcut_info->org_path);
    qDebug() <<"保存路径::" << QString((char*)imgcut_info->save_path);
    m_img_cut_path = QString((char*)imgcut_info->save_path);


    qDebug() << "是否需要对图像进行缩放操作：" << imgcut_info->is_resize;
    brightness_filter_model_info.is_resize = imgcut_info->is_resize;

    ui->ui_algorithm_brightnessfilter_clientname->setText(QString((char*)imgcut_info->client_name));
    ui->ui_algorithm_brightnessfilter_clientname->setReadOnly(true);
    ui->ui_algorithm_brightnessfilter_cameraname->setText(QString((char*)imgcut_info->camera_device_name));
    ui->ui_algorithm_brightnessfilter_cameraname->setReadOnly(true);
    ui->ui_algorithm_brightnessfilter_algorithm_name->setText(QString((char*)imgcut_info->algorithm_name));
    ui->ui_algorithm_brightnessfilter_algorithm_name->setReadOnly(true);
    ui->ui_algorithm_brightnessfilter_algorithm_version->setText(QString((char*)imgcut_info->algorithm_version));
    ui->ui_algorithm_brightnessfilter_algorithm_version->setReadOnly(true);

    if(m_database_flag == true) {
        ui->ui_algorithm_brightnessfilter_Maskvalue_edt->setReadOnly(true);
        ui->ui_algorithm_brightnessfilter_row_edt->setReadOnly(true);
        ui->ui_algorithm_brightnessfilter_cols_edt->setReadOnly(true);
        ui->ui_algorithm_brightnessfilter_offset_edt->setReadOnly(true);
        ui->ui_algorithm_brightnessfilter_off_x_offset_edt->setReadOnly(true);
        ui->ui_algorithm_brightnessfilter_off_y_offset_edt->setReadOnly(true);
        ui->ui_algorithm_brightnessfilter_time_offset_edt->setReadOnly(true);

        ui->ui_algorithm_brightnessfilter_read_result_btn->setEnabled(false);
    }
}

void ui_algorithm_brightnessfilter::slot_clear_model_info()
{

}



//校验指示灯亮灭状态行列索引
int check_lamp_status(int num_row,brightness_lamp_info_out_t* out_info, brightness_lamp_info_out_t* res_info)
{
    bool flag = false;
    int* arr_out = nullptr;
    int* arr_result = nullptr;
    int count_row = 0;
    int count_cols = 0;

    for(int i=0;i<num_row;i++) {//行索引
        arr_out = new int[8];
        for(int i=0;i<8;i++) arr_out[i] = -99;
        arr_result = new int[8];
        for(int i=0;i<8;i++) arr_result[i] = -99;

        qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;

        for(int k=0;k<out_info->button_info_List.size();k++) {//遍历on按钮列表
            if(i == out_info->button_info_List[k]->row_index) {//提取每一行的on状态列索引 存放在arr_out数组中
                int cols_index_out = out_info->button_info_List[k]->cols_index;
                arr_out[cols_index_out] = cols_index_out;
            }
        }
        for(int i=0;i<brightness_filter_model_info.num_cols;i++) {
             qDebug() << "arr_out数值::" << arr_out[i];
        }

        for(int k=0;k<res_info->button_info_List.size();k++) {//遍历on按钮列表
            if(i == res_info->button_info_List[k]->row_index) {//提取每一行的on状态列索引 存放在arr_out数组中
                int cols_index_result  = res_info->button_info_List[k]->cols_index;
                arr_result[cols_index_result] = cols_index_result;
            }
        }

        for(int i=0;i<num_row;i++) {
             qDebug() << "arr_result数值::" << arr_result[i];
        }

        //比较数组列索引
        for(int i=0;i<8;i++) {
            if(arr_out[i] == arr_result[i]) {
                count_cols++;
            }
        }
        if(count_cols == 8)  count_row++;
        qDebug() << "i" << i << "count_cols::" << count_cols << "count_row::" << count_row;

        delete arr_out;arr_out = nullptr;
        delete arr_result;arr_result = nullptr;
        count_cols = 0;
        qDebug() << "-----------------------------------------------------------------------" ;
    }

    qDebug() << "count_row::" << count_row;
    if(count_row == 4) flag = true;

    return flag;
}

/*
//亮度模型处理函数
void brightness_model_handle(Mat img,brightness_lamp_info_out_t** brightness_lamp_on_info_out,
                             brightness_filter_model_t* _brightness_filter_model_info)
{
    Mat imgGray,imgBlur,imgTreshold;//Mat图像
    QList<int> yList;//纵坐标列表

    //图像的预处理 Proprocessing
    cvtColor(img, imgGray, COLOR_BGR2GRAY);//转换为灰度图
    GaussianBlur(imgGray, imgBlur, Size(3, 3), 3, 0);//高斯模糊

    //阈值化 二值化操作   取任意像素值p >= 200，并将其设置为255(白色)。像素值< 200被设置为0(黑色)
    threshold(imgBlur, imgTreshold, brightness_filter_model_info.maskVaule, 255, cv::THRESH_BINARY);
    imshow("ImageTreshold", imgTreshold);

    //添加轮廓检测 处理亮状态的指示灯
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(imgTreshold, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    vector<vector<Point>> conPoly(contours.size());
    vector<Rect> boundRect(contours.size());

    //qDebug() << "contours.size::" << contours.size();//轮廓数量
    qDebug() <<  "横坐标平均值：" << _brightness_filter_model_info->imgcut_info.width    / _brightness_filter_model_info->num_cols;
    qDebug() <<  "纵坐标平均值：" << _brightness_filter_model_info->imgcut_info.height   / _brightness_filter_model_info->num_row;

    for (int i = 0; i < (int)contours.size(); i++)
    {
        (*brightness_lamp_on_info_out)->lamp_status = 1;
        (*brightness_lamp_on_info_out)->lamp_num++;

        //int area = contourArea(contours[i]);
        //qDebug() <<"area:" << area ;

        float peri = arcLength(contours[i], true);//计算封闭曲线的周长
        approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);//多边形拟合曲线
        boundRect[i] = boundingRect(conPoly[i]);//轮廓垂直边界最小矩形
        //qDebug() <<  "conPoly size:" << conPoly[i].size();

        brightness_button_Point_t* brightness_button_Point = new brightness_button_Point_t;
        memset(brightness_button_Point,0,sizeof(brightness_button_Point_t));

        if(i==0) {
            brightness_button_Point->x              = boundRect[i].x ;
            brightness_button_Point->y              = boundRect[i].y;
            yList.append(brightness_button_Point->y);
        }
        else {
            brightness_button_Point->x = boundRect[i].x;
            //对纵坐标进行归一化处理
            for(int k=0;k<yList.size();k++) {
                int sub_abs = qAbs(boundRect[i].y - yList.at(k));
                if(sub_abs > 20) {
                    yList.append(boundRect[i].y);
                    brightness_button_Point->y = boundRect[i].y;
                }
                else {
                    brightness_button_Point->y = yList.at(k);
                    break;
                }
            }
        }

        brightness_button_Point->cols_index     = brightness_button_Point->x / (_brightness_filter_model_info->imgcut_info.width/_brightness_filter_model_info->num_cols  + (_brightness_filter_model_info->offset));
        brightness_button_Point->row_index      = brightness_button_Point->y / (_brightness_filter_model_info->imgcut_info.height/_brightness_filter_model_info->num_row +  (_brightness_filter_model_info->offset));

        (*brightness_lamp_on_info_out)->button_info_List.append(brightness_button_Point);

       putText(img,"on",Size(brightness_button_Point->x - (_brightness_filter_model_info->off_x_offset),
                             brightness_button_Point->y - (_brightness_filter_model_info->off_y_offset)),cv::FONT_HERSHEY_COMPLEX_SMALL,0.5,cv::Scalar(0, 255, 255),1,LINE_4,0);
       //drawContours(imgCopy, conPoly, i, Scalar(255, 0, 255), 2);
       imshow("contours check on", img);
       waitKey(1000);
    }
}

//自动添加off状态
void add_off_text_to_img(Mat img,
                         brightness_lamp_info_out_t** brightness_lamp_off_info_out,
                         brightness_filter_model_t*  _brightness_filter_model_info,
                         brightness_lamp_info_out_t*  brightness_lamp_on_info_out)
{

    int* a = nullptr;
    (*brightness_lamp_off_info_out)->lamp_status = 0;

    for(int i=0;i<_brightness_filter_model_info->num_row;i++) {//行索引
        int value_y = 0;
        a = new int[_brightness_filter_model_info->num_cols];
        for(int i=0;i<_brightness_filter_model_info->num_cols;i++)  a[i] = -99;

        for(int k=0;k<brightness_lamp_on_info_out->button_info_List.size();k++) {//遍历所有on状态按钮
            if(i == brightness_lamp_on_info_out->button_info_List[k]->row_index) {//比较行数据
                 int index = brightness_lamp_on_info_out->button_info_List[k]->cols_index;//提取列数据索引
                 a[index] = index;
                 value_y = brightness_lamp_on_info_out->button_info_List[k]->y;
             }//end if
        }//end k

        for(int i=0;i<_brightness_filter_model_info->num_cols;i++) qDebug() << "数值::" << a[i];

        qDebug() << "---------------------------------------------------------------------" ;
        for(int j=0;j<_brightness_filter_model_info->num_cols;j++) {
            if(j != a[j]) {
                brightness_button_Point_t* brightness_button_Point = new brightness_button_Point_t;
                memset(brightness_button_Point,0,sizeof(brightness_button_Point_t));
                brightness_button_Point->x = (_brightness_filter_model_info->imgcut_info.width / _brightness_filter_model_info->num_cols) * (j+1) - (_brightness_filter_model_info->off_x_offset);
                if(value_y == 0)
                     brightness_button_Point->y = (_brightness_filter_model_info->imgcut_info.height /_brightness_filter_model_info->num_row) * (i+1) - (_brightness_filter_model_info->off_y_offset);
                else
                     brightness_button_Point->y = value_y - (_brightness_filter_model_info->off_y_offset);
                brightness_button_Point->row_index = i;
                brightness_button_Point->cols_index = j;
                (*brightness_lamp_off_info_out)->lamp_num ++;
                (*brightness_lamp_off_info_out)->button_info_List.append(brightness_button_Point);

                QString str = "off";
                QByteArray str_ba = str.toLatin1();
                putText(img,str_ba.data(),Size(brightness_button_Point->x,brightness_button_Point->y),cv::FONT_HERSHEY_COMPLEX_SMALL,0.5,cv::Scalar(0, 255, 255),1,LINE_4,0);
                //drawContours(imgCopy, conPoly, i, Scalar(255, 0, 255), 2);
                imshow("contours check off", img);
                waitKey(1000);
            }
        }
        free(a);
        a = NULL;
    }
}
*/


/* 亮度模型处理函数 基于原图
* Mat imgorg：原图
* brightness_lamp_info_out_t** brightness_lamp_on_info_out：on状态输出参数
* brightness_filter_model_t* _brightness_filter_model_info
*/

Mat brightness_model_handle_for_imgorg(Mat imgorg,brightness_lamp_info_out_t** brightness_lamp_on_info_out,
                                        brightness_filter_model_t* _brightness_filter_model_info)
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
    Mat imgCopy,imgResize,imgCut;
    Mat imgGray,imgBlur,imgTreshold;//Mat图像
    Mat imgDst;
    //Mat imgPutText_dynamic;
    QList<int> yList;//纵坐标列表

    imgorg.copyTo(imgCopy);
    imgCopy.copyTo(imgDst);

    //是否缩放 输入参数1
    if(_brightness_filter_model_info->is_resize == 1) {
        cv::resize(imgCopy,imgResize,Size(),0.5,0.5);
    }

    //裁剪 输入参数2
    Rect rectRoi(_brightness_filter_model_info->imgcut_info.x,
                 _brightness_filter_model_info->imgcut_info.y,
                 _brightness_filter_model_info->imgcut_info.width,
                 _brightness_filter_model_info->imgcut_info.height);
    imgCut = imgResize(rectRoi);
    //imshow("imgCut", imgCut);
    //waitKey(1000);

    //图像的预处理 Proprocessing
    cvtColor(imgCut, imgGray, COLOR_BGR2GRAY);//转换为灰度图
    GaussianBlur(imgGray, imgBlur, Size(3, 3), 3, 0);//高斯模糊

    //阈值化 二值化操作   取任意像素值p >= 200，并将其设置为255(白色)。像素值< 200被设置为0(黑色)
    threshold(imgBlur, imgTreshold, _brightness_filter_model_info->maskVaule, 255, cv::THRESH_BINARY);
    imshow("ImageTreshold", imgTreshold);
    waitKey(1000);

    //添加轮廓检测 处理亮状态的指示灯
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(imgTreshold, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
    vector<vector<Point>> conPoly(contours.size());
    vector<Rect> boundRect(contours.size());

    qDebug() << "contours.size::" << contours.size();//轮廓数量
    qDebug() <<  "横坐标平均值：" << _brightness_filter_model_info->imgcut_info.width    / _brightness_filter_model_info->num_cols;
    qDebug() <<  "纵坐标平均值：" << _brightness_filter_model_info->imgcut_info.height   / _brightness_filter_model_info->num_row;

    for (int i = 0; i < (int)contours.size(); i++)
    {
        (*brightness_lamp_on_info_out)->lamp_status = 1;
        (*brightness_lamp_on_info_out)->lamp_num++;

        int area = contourArea(contours[i]);
        qDebug() <<"area:" << area ;

        float peri = arcLength(contours[i], true);//计算封闭曲线的周长
        approxPolyDP(contours[i], conPoly[i], 0.02 * peri, true);//多边形拟合曲线
        boundRect[i] = boundingRect(conPoly[i]);//轮廓垂直边界最小矩形
        qDebug() <<  "conPoly size:" << conPoly[i].size();

        brightness_button_Point_t* brightness_button_Point = new brightness_button_Point_t;
        memset(brightness_button_Point,0,sizeof(brightness_button_Point_t));

        if(i==0) {
            brightness_button_Point->x              = boundRect[i].x;
            brightness_button_Point->y              = boundRect[i].y;
            yList.append(brightness_button_Point->y);

        }
        else {
            brightness_button_Point->x = boundRect[i].x;
            //对纵坐标进行归一化处理
            for(int k=0;k<yList.size();k++) {
                int sub_abs = qAbs(boundRect[i].y - yList.at(k));
                if(sub_abs > 20) {
                    yList.append(boundRect[i].y);
                    brightness_button_Point->y = boundRect[i].y ;
                }
                else {
                    brightness_button_Point->y = yList.at(k);
                    break;
                }
            }
        }

        brightness_button_Point->cols_index     = brightness_button_Point->x / (_brightness_filter_model_info->imgcut_info.width/_brightness_filter_model_info->num_cols  + (_brightness_filter_model_info->offset));
        brightness_button_Point->row_index      = brightness_button_Point->y / (_brightness_filter_model_info->imgcut_info.height/_brightness_filter_model_info->num_row +  (_brightness_filter_model_info->offset));

        (*brightness_lamp_on_info_out)->button_info_List.append(brightness_button_Point);

        //原图时间标记
        int org_x = brightness_button_Point->x + _brightness_filter_model_info->imgcut_info.x;
        int org_y = brightness_button_Point->y + _brightness_filter_model_info->imgcut_info.y;
        QDateTime datatime = QDateTime::currentDateTime();
        QString time = datatime.toString("yyyy-MM-dd hh:mm:ss");
        QByteArray ba = time.toLatin1();

        //Mat imgPutText;

        //imgCopy.copyTo(imgPutText);
        //putText(imgPutText,ba.data(),
        //        Size(_brightness_filter_model_info->time_offset,_brightness_filter_model_info->time_offset),
        //        cv::FONT_HERSHEY_COMPLEX_SMALL,3,cv::Scalar(0, 255, 0),1,LINE_4,0);

        putText(imgDst,"on",
                Size(org_x * 2- (_brightness_filter_model_info->off_x_offset),org_y*2- (_brightness_filter_model_info->off_y_offset)),
                cv::FONT_HERSHEY_COMPLEX_SMALL,0.5,cv::Scalar(0, 255, 0),1,LINE_4,0);


        //addWeighted(imgPutText, 0.3, imgPutText_dynamic, 0.7, 0.0, imgDst); //这里调用了addWeighted函数，得到的结果存储在dst中


        namedWindow("imgDst on", WINDOW_NORMAL);
        imshow("imgDst on", imgDst);
        waitKey(1000);
    }
    destroyAllWindows();
    //释放图像内存
   imgCopy.release();
   imgResize.release();
   imgCut.release();
   imgGray.release();
   imgBlur.release();
   imgTreshold.release();

   return imgDst;
}

//自动添加off状态 基于原图
Mat add_off_text_to_img_for_imgorg(Mat imgorg,
                         brightness_lamp_info_out_t** brightness_lamp_off_info_out,
                         brightness_filter_model_t*  _brightness_filter_model_info,
                         brightness_lamp_info_out_t*  brightness_lamp_on_info_out)
{
    Mat imgCopy;
    Mat imgDst;
    Mat imgPutText_dynamic;

    imgorg.copyTo(imgCopy);
    imgCopy.copyTo(imgPutText_dynamic);


    int* a = nullptr;
    (*brightness_lamp_off_info_out)->lamp_status = 0;

    for(int i=0;i<_brightness_filter_model_info->num_row;i++) {//行索引
        int value_y = 0;
        a = new int[_brightness_filter_model_info->num_cols];
        for(int i=0;i<_brightness_filter_model_info->num_cols;i++)  a[i] = -99;

        for(int k=0;k<brightness_lamp_on_info_out->button_info_List.size();k++) {//遍历所有on状态按钮
            if(i == brightness_lamp_on_info_out->button_info_List[k]->row_index) {//比较行数据
                 int index = brightness_lamp_on_info_out->button_info_List[k]->cols_index;//提取列数据索引
                 a[index] = index;
                 value_y = brightness_lamp_on_info_out->button_info_List[k]->y;
             }//end if
        }//end k

        for(int i=0;i<_brightness_filter_model_info->num_cols;i++) qDebug() << "数值::" << a[i];

        qDebug() << "---------------------------------------------------------------------" ;
        for(int j=0;j<_brightness_filter_model_info->num_cols;j++) {
            if(j != a[j]) {
                brightness_button_Point_t* brightness_button_Point = new brightness_button_Point_t;
                memset(brightness_button_Point,0,sizeof(brightness_button_Point_t));
                brightness_button_Point->x = (_brightness_filter_model_info->imgcut_info.width / _brightness_filter_model_info->num_cols) * (j+1);
                if(value_y == 0)
                     brightness_button_Point->y = (_brightness_filter_model_info->imgcut_info.height /_brightness_filter_model_info->num_row) * (i+1);
                else
                     brightness_button_Point->y = value_y;
                brightness_button_Point->row_index = i;
                brightness_button_Point->cols_index = j;
                (*brightness_lamp_off_info_out)->lamp_num ++;
                (*brightness_lamp_off_info_out)->button_info_List.append(brightness_button_Point);

                //原图时间标记
                int org_x = brightness_button_Point->x + _brightness_filter_model_info->imgcut_info.x;
                int org_y = brightness_button_Point->y + _brightness_filter_model_info->imgcut_info.y;
                QDateTime datatime = QDateTime::currentDateTime();
                QString time = datatime.toString("yyyy-MM-dd hh:mm:ss");
                QByteArray ba = time.toLatin1();

                Mat imgPutText;
                imgorg.copyTo(imgPutText);
                putText(imgPutText,ba.data(),
                        Size(_brightness_filter_model_info->time_offset,_brightness_filter_model_info->time_offset),
                        cv::FONT_HERSHEY_COMPLEX_SMALL,3,cv::Scalar(0, 255, 0),1,LINE_4,0);

                putText(imgPutText_dynamic,"off",
                        Size(org_x * 2 - (_brightness_filter_model_info->off_x_offset),org_y * 2- (_brightness_filter_model_info->off_y_offset) ),
                        cv::FONT_HERSHEY_COMPLEX_SMALL,0.5,cv::Scalar(0, 255, 0),1,LINE_4,0);

                addWeighted(imgPutText, 0.3, imgPutText_dynamic, 0.7, 0.0, imgDst); //这里调用了addWeighted函数，得到的结果存储在dst中

               namedWindow("imgDst off", WINDOW_NORMAL);
               imshow("imgDst off", imgDst);
               waitKey(1000);
            }
        }
        free(a);
        a = NULL;
    }
    destroyAllWindows();
    imgCopy.release();
    imgPutText_dynamic.release();

    return imgDst;
}

void free_list_memory(QList<brightness_lamp_info_out_t*> imglist1,QList<brightness_lamp_info_out_t*> imglist2)
{
    //QList 释放内存 元素为指针类型
    foreach(brightness_lamp_info_out_t *brightness_lamp_info,imglist1)
    {
        if(brightness_lamp_info)
        {
            imglist1.removeOne(brightness_lamp_info);
            delete brightness_lamp_info;
            brightness_lamp_info = nullptr;
        }
    }

    foreach(brightness_lamp_info_out_t *brightness_lamp_info1,imglist2)
    {
        if(brightness_lamp_info1)
        {
            imglist2.removeOne(brightness_lamp_info1);
            delete brightness_lamp_info1;
            brightness_lamp_info1 = nullptr;
        }
    }
}



int ui_algorithm_brightnessfilter::save_to_database(brightness_template_info_t* brightness_template_info,brightness_filter_model_t* brightness_filter_model_info,
                      QString& client_uuid,int camera_id,int algorithm_id)
{
    sqlBase sqlbase;
    brightness_template_info->client_uuid = client_uuid;
    brightness_template_info->camera_id = camera_id;
    brightness_template_info->algorithm_id = algorithm_id;

    QString sql = "SELECT MAX(brightness_template_info.brightness_template_no) FROM brightness_template_info;";
    brightness_template_info->brightness_template_no = sqlbase.get_maxid(m_dbconnect_info.dbconn,sql) + 1;
    brightness_template_info->is_resize = brightness_filter_model_info->is_resize;
    brightness_template_info->imgorg_width = brightness_filter_model_info->imgcut_info.org_width;
    brightness_template_info->imgorg_height = brightness_filter_model_info->imgcut_info.org_height;
    brightness_template_info->imgcut_x = brightness_filter_model_info->imgcut_info.x;
    brightness_template_info->imgcut_y = brightness_filter_model_info->imgcut_info.y;
    brightness_template_info->imgcut_width = brightness_filter_model_info->imgcut_info.width;
    brightness_template_info->imgcut_height = brightness_filter_model_info->imgcut_info.height;
    brightness_template_info->time_offset = brightness_filter_model_info->time_offset;
    brightness_template_info->template_image_org_path = m_img_org_path;
    brightness_template_info->template_image_cut_path = m_img_cut_path;
    brightness_template_info->maskVaule = brightness_filter_model_info->maskVaule;
    brightness_template_info->num_row = brightness_filter_model_info->num_row;
    brightness_template_info->num_cols = brightness_filter_model_info->num_cols;
    brightness_template_info->offset = brightness_filter_model_info->offset;
    brightness_template_info->off_x_offset = brightness_filter_model_info->off_x_offset;
    brightness_template_info->off_y_offset = brightness_filter_model_info->off_y_offset;

    int ret = sqlbase.insert_to_brightness_template_info(m_dbconnect_info.dbconn,brightness_template_info);
    return ret;
}


int get_brightness_model_info_from_database(QString& client_uuid,int camera_id,int algorithm_id)
{
    QSqlQuery query(m_dbconnect_info.dbconn);
    query.prepare("SELECT * FROM brightness_template_info \
                   WHERE brightness_template_info.client_uuid = ? \
                   AND brightness_template_info.camera_id = ?  \
                   AND brightness_template_info.algorithm_id = ?;");

    query.addBindValue(client_uuid);
    qDebug() << "client_uuid:::" << client_uuid;

    query.addBindValue(camera_id);
    qDebug() << "camera_id:::" << camera_id;

    query.addBindValue(algorithm_id);
    qDebug() << "algorithm_id:::" << algorithm_id;

    if(query.exec()) {
        while(query.next()) {
             //brightness_filter_model_info->brightness_template_no     = query.value(0).toInt();
             //brightness_filter_model_info->client_uuid                = query.value(1).toString();
             //brightness_filter_model_info->camera_id                  = query.value(2).toInt();
             //brightness_filter_model_info->algorithm_id               = query.value(3).toInt();
             brightness_filter_model_info.imgcut_info.org_width          = query.value(4).toInt();
             brightness_filter_model_info.imgcut_info.org_height         = query.value(5).toInt();
             brightness_filter_model_info.is_resize                      = query.value(6).toInt();
             brightness_filter_model_info.imgcut_info.x                  = query.value(7).toInt();
             brightness_filter_model_info.imgcut_info.y                  = query.value(8).toInt();
             brightness_filter_model_info.imgcut_info.width              = query.value(9).toInt();
             brightness_filter_model_info.imgcut_info.height             = query.value(10).toInt();
             //brightness_filter_model_info.template_image_org_path      = query.value(11).toString();
             //brightness_filter_model_info->template_image_cut_path     = query.value(12).toString();
             brightness_filter_model_info.num_row                        = query.value(13).toInt();
             brightness_filter_model_info.num_cols                       = query.value(14).toInt();
             brightness_filter_model_info.maskVaule                      = query.value(15).toInt();
             brightness_filter_model_info.offset                         = query.value(16).toInt();
             brightness_filter_model_info.off_x_offset                   = query.value(17).toInt();
             brightness_filter_model_info.off_y_offset                   = query.value(18).toInt();
             brightness_filter_model_info.time_offset                    = query.value(19).toInt();
        }
    }
    else {
        qDebug()<< "数据库错误::" << query.lastError()<<Qt::endl;
        return -1;
    }

    query.clear();
    return 1;
}

void print_brightness_lamp_info_list(QList<brightness_lamp_info_out_t*> _brightness_lamp_info_out_list)
{
    //打印
    for(int i=0;i<_brightness_lamp_info_out_list.size();i++) {
       qDebug() << "亮灭状态:" << _brightness_lamp_info_out_list.at(i)->lamp_status;
       qDebug() << "亮灭状态数量:" << _brightness_lamp_info_out_list.at(i)->lamp_num;
       for(int j=0;j<_brightness_lamp_info_out_list.at(i)->button_info_List.size();j++) {
           qDebug() << "亮灭状态位置坐标信息[x,y,cols_index,row_index]::["
                    << _brightness_lamp_info_out_list.at(i)->button_info_List.at(j)->x << ","
                    << _brightness_lamp_info_out_list.at(i)->button_info_List.at(j)->y << ","
                    << _brightness_lamp_info_out_list.at(i)->button_info_List.at(j)->cols_index << ","
                    << _brightness_lamp_info_out_list.at(i)->button_info_List.at(j)->row_index  << "]";
       }
    }
}


int compare_img(QList<brightness_lamp_info_out_t*> imglist1,QList<brightness_lamp_info_out_t*> imglist2,brightness_filter_model_t*  brightness_filter_model_info)
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;

    //首先校验亮灭按钮数量
    if(imglist1[0]->lamp_num == imglist2[0]->lamp_num
    && imglist1[1]->lamp_num == imglist2[1]->lamp_num) {
        QMessageBox::information(NULL, "About!!", "模型识别得到的亮灭状态按钮的数量同实际结果一致!!");
    }
    else {
         QMessageBox::information(NULL, "About!!", "模型识别得到的亮灭状态按钮的数量同实际结果不一致,请调整相关参数!!");
         free_list_memory(imglist1,imglist2);
         return -1;
    }

    //校验行列索引
    int on_flag = check_lamp_status(brightness_filter_model_info->num_row,imglist1[0],imglist2[0]);
    int off_flag =check_lamp_status(brightness_filter_model_info->num_row,imglist1[1],imglist2[1]);
     if(on_flag == true && off_flag == true)
          QMessageBox::information(NULL, "About!!", "模型识别得到的亮灭状态按钮的行列索引值同实际结果一致!!");
     else {
         QMessageBox::information(NULL, "About!!", "模型识别得到的亮灭状态按钮的行列索引值同实际结果不一致，请调整参数或者检查实际结果文件!!");
         free_list_memory(imglist1,imglist2);
         return -1;
     }

     return 0;
}

//执行算法模型  训练产生输入参数
void ui_algorithm_brightnessfilter::on_ui_algorithm_brightnessfilter_train_btn_clicked()
{
     qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
     Mat imgOrg,imgCopy;

     brightness_filter_model_info.maskVaule         = ui->ui_algorithm_brightnessfilter_Maskvalue_edt->text().toInt();      //二值化阈值
     brightness_filter_model_info.num_row           = ui->ui_algorithm_brightnessfilter_row_edt->text().toInt();            //行数
     brightness_filter_model_info.num_cols          = ui->ui_algorithm_brightnessfilter_cols_edt->text().toInt();           //列数
     brightness_filter_model_info.offset            = ui->ui_algorithm_brightnessfilter_offset_edt->text().toInt();         //位置偏移坐标
     brightness_filter_model_info.off_x_offset      = ui->ui_algorithm_brightnessfilter_off_x_offset_edt->text().toInt();     //off状态位置x偏移坐标
     brightness_filter_model_info.off_y_offset      = ui->ui_algorithm_brightnessfilter_off_y_offset_edt->text().toInt();     //off状态位置y偏移坐标
     brightness_filter_model_info.time_offset       = ui->ui_algorithm_brightnessfilter_time_offset_edt->text().toInt();     //时间标记位置偏移坐标

     //客户端信息 摄像头信息  算法信息
     QString client_uuid = QString((char*)brightness_filter_model_info.algorithmInfo.clinet_uuid);
     int camera_id = brightness_filter_model_info.algorithmInfo.camera_id;
     int algorithm_id = brightness_filter_model_info.algorithmInfo.algorithm_id;

     //判断该客户端所对应的相机拍摄的指示灯图片是否训练
     if(m_database_flag == false && brightness_filter_model_info.algorithmInfo.algorithm_train_res == 0) {//模板不存在 训练生成参数
         QMessageBox::information(NULL, "About!!", "将训练生成参数!!");

         //亮度过滤器 文件对话框 打开来自原图的图像
         QFile file_org(m_img_org_path);
         if(!file_org.open(QFile::ReadOnly))
         {
             QMessageBox::information(NULL, "Warning!!", "图片未选择!!");
             return;
         }else{
             QByteArray ba = file_org.readAll();
             imgOrg = imdecode(vector<char>(ba.begin(), ba.end()), 1);
         }

         imgOrg.copyTo(imgCopy);

         //亮度模型处理
         brightness_lamp_info_out_t* brightness_lamp_on_info_out = new brightness_lamp_info_out_t;//指示灯亮状态
         memset(brightness_lamp_on_info_out,0,sizeof(int)*2);
         imgCopy = brightness_model_handle_for_imgorg(imgCopy,&brightness_lamp_on_info_out,&brightness_filter_model_info);
         brightness_lamp_info_out_list.append(brightness_lamp_on_info_out);

         //标记off状态函数
         brightness_lamp_info_out_t* brightness_lamp_off_info_out = new brightness_lamp_info_out_t;//指示灯灭状态
         memset(brightness_lamp_off_info_out,0,sizeof(int)*2);
         imgCopy = add_off_text_to_img_for_imgorg(imgCopy,&brightness_lamp_off_info_out,
                             &brightness_filter_model_info,
                             brightness_lamp_on_info_out);

        brightness_lamp_info_out_list.append(brightness_lamp_off_info_out);
        qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;

        //打印
        print_brightness_lamp_info_list(brightness_lamp_info_out_list);

        int ret = compare_img(brightness_lamp_info_out_list,brightness_lamp_info_result_List,&brightness_filter_model_info);
        if(ret <0)
            return;
        if(ret == 0)
             brightness_filter_model_info.algorithmInfo.algorithm_train_res = 1;

        free_list_memory(brightness_lamp_info_out_list,brightness_lamp_info_result_List);

        //增加到数据库中
        brightness_template_info_t* brightness_template_info = new brightness_template_info_t;
        memset(brightness_template_info,0,sizeof(int)*17);
        brightness_template_info->client_uuid = "";
        brightness_template_info->template_image_cut_path = "";

        ret = save_to_database(brightness_template_info,&brightness_filter_model_info,client_uuid,camera_id,algorithm_id);
        if(ret > 0)
            QMessageBox::information(NULL, "About!!", "插入到亮度过滤算法模板数据库中成功!!");
        else if(ret < 0)
            QMessageBox::information(NULL, "About!!", "插入到亮度过滤算法模板数据库中失败!!");
     }
     else if(m_database_flag == true || brightness_filter_model_info.algorithmInfo.algorithm_train_res == 1) {//模板存在 从数据库中获取数据
         QMessageBox::information(NULL, "About!!", "将通过数据库中保存的参数运行模型!!");
         int ret = get_brightness_model_info_from_database(client_uuid,camera_id,algorithm_id);
         if(ret > 0) {
             QMessageBox::information(NULL, "About!!", "从数据库中得到模型参数成功!!");
             brightness_filter_model_info.algorithmInfo.algorithm_train_res = 1;

             ui->ui_algorithm_brightnessfilter_Maskvalue_edt->setText(QString::number(brightness_filter_model_info.maskVaule));
             ui->ui_algorithm_brightnessfilter_row_edt->setText(QString::number(brightness_filter_model_info.num_row));
             ui->ui_algorithm_brightnessfilter_cols_edt->setText(QString::number(brightness_filter_model_info.num_cols));
             ui->ui_algorithm_brightnessfilter_offset_edt->setText(QString::number(brightness_filter_model_info.offset));
             ui->ui_algorithm_brightnessfilter_off_x_offset_edt->setText(QString::number(brightness_filter_model_info.off_x_offset));
             ui->ui_algorithm_brightnessfilter_off_y_offset_edt->setText(QString::number(brightness_filter_model_info.off_y_offset));
             ui->ui_algorithm_brightnessfilter_time_offset_edt->setText(QString::number(brightness_filter_model_info.time_offset));
         }
         else {
             QMessageBox::information(NULL, "About!!", "从数据库中得到模型参数失败!!");
             brightness_filter_model_info.algorithmInfo.algorithm_train_res = 0;
         }
     }
}

void ui_algorithm_brightnessfilter::on_ui_algorithm_brightnessfilter_read_result_btn_clicked()
{
    brightness_lamp_info_out_t* brightness_lamp_on_info_out = new brightness_lamp_info_out_t;//指示灯亮状态
    memset(brightness_lamp_on_info_out,0,sizeof(int)*2);
    brightness_lamp_info_out_t* brightness_lamp_off_info_out = new brightness_lamp_info_out_t;//指示灯灭状态
    memset(brightness_lamp_off_info_out,0,sizeof(int)*2);

    QString strPath = QFileDialog::getOpenFileName(this, "打开图片", tr("c:\\"), tr("Images (*.txt *.*)"));
    QFile file(strPath);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::information(NULL, "Warning!!", "读入失败!!");
        return;
    }
    else {
        qDebug() << "filename:" << strPath;
        QString line;
        QTextStream in(&file);  //用文件构造流
        line = in.readLine();//读取一行放到字符串里
        qDebug() << line;
        qDebug() << "str1 :::" << line.section(',', 0, 0).trimmed();

        int status = line.section(',', 2, 2).trimmed().toInt();
        brightness_button_Point_t* brightness_button_info = new brightness_button_Point;
        brightness_button_info->row_index = line.section(',', 0, 0).trimmed().toInt();
        brightness_button_info->cols_index = line.section(',', 1, 1).trimmed().toInt();
        brightness_button_info->x    =  -99;
        brightness_button_info->y    =  -99;

        if(status == 1) {
            brightness_lamp_on_info_out->button_info_List.append(brightness_button_info);
            brightness_lamp_on_info_out->lamp_status = 1;
            brightness_lamp_on_info_out->lamp_num++;
        }
        else if(status == 0) {
            brightness_lamp_off_info_out->button_info_List.append(brightness_button_info);
            brightness_lamp_off_info_out->lamp_status = 0;
            brightness_lamp_off_info_out->lamp_num++;
        }



        while(!line.isNull() && line != "")//字符串有内容
        {
            qDebug() << line;
            qDebug() << "str1 :::" << line.section(',', 0, 0).trimmed();
            line=in.readLine();//循环读取下行
            if(line != "") {
                int status = line.section(',', 2, 2).trimmed().toInt();
                brightness_button_Point_t* brightness_button_info = new brightness_button_Point;
                brightness_button_info->row_index = line.section(',', 0, 0).trimmed().toInt();
                brightness_button_info->cols_index = line.section(',', 1, 1).trimmed().toInt();
                brightness_button_info->x    =  -99;
                brightness_button_info->y    =  -99;

                if(status == 1) {
                    brightness_lamp_on_info_out->button_info_List.append(brightness_button_info);
                    brightness_lamp_on_info_out->lamp_status = 1;
                    brightness_lamp_on_info_out->lamp_num++;
                }
                else if(status == 0) {
                    brightness_lamp_off_info_out->button_info_List.append(brightness_button_info);
                    brightness_lamp_off_info_out->lamp_status = 0;
                    brightness_lamp_off_info_out->lamp_num++;
                }
            }

        }//end while
    }//end else

    brightness_lamp_info_result_List.append(brightness_lamp_on_info_out);
    brightness_lamp_info_result_List.append(brightness_lamp_off_info_out);

    file.close();

    //打印
    for(int i=0;i<brightness_lamp_info_result_List.size();i++) {
       qDebug() << "亮灭状态:" << brightness_lamp_info_result_List.at(i)->lamp_status;
       qDebug() << "亮灭状态数量:" << brightness_lamp_info_result_List.at(i)->lamp_num;
       for(int j=0;j<brightness_lamp_info_result_List.at(i)->button_info_List.size();j++) {
           qDebug() << "亮灭状态位置坐标信息[x,y,cols_index,row_index]::["
                    << brightness_lamp_info_result_List.at(i)->button_info_List.at(j)->x << ","
                    << brightness_lamp_info_result_List.at(i)->button_info_List.at(j)->y << ","
                    << brightness_lamp_info_result_List.at(i)->button_info_List.at(j)->cols_index << ","
                    << brightness_lamp_info_result_List.at(i)->button_info_List.at(j)->row_index  << "]";
       }
    }

   QMessageBox::information(NULL, "Warning!!", "读入成功!!");
}

QList<brightness_lamp_info_out_t*> get_brightness_lamp_info_out_list(Mat img)
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
    QList<brightness_lamp_info_out_t*> brightness_info_out_list;
    Mat imgCopy;
    img.copyTo(imgCopy);//以后用imgCopy处理

    //亮度模型处理
    brightness_lamp_info_out_t* brightness_lamp_on_info_out = new brightness_lamp_info_out_t;//指示灯亮状态
    memset(brightness_lamp_on_info_out,0,sizeof(int)*2);
    imgCopy = brightness_model_handle_for_imgorg(imgCopy,&brightness_lamp_on_info_out,&brightness_filter_model_info);
    brightness_info_out_list.append(brightness_lamp_on_info_out);

     //标记off状态函数
     brightness_lamp_info_out_t* brightness_lamp_off_info_out = new brightness_lamp_info_out_t;//指示灯灭状态
     memset(brightness_lamp_off_info_out,0,sizeof(int)*2);
     imgCopy = add_off_text_to_img_for_imgorg(imgCopy,&brightness_lamp_off_info_out,
                         &brightness_filter_model_info,
                         brightness_lamp_on_info_out);

    brightness_info_out_list.append(brightness_lamp_off_info_out);

    return brightness_info_out_list;
}
void ui_algorithm_brightnessfilter::on_ui_algorithm_brightnessfilter_test_btn_clicked()
{
    //打开本地图片1
    Mat imgfrompath;
    QString strPath = QFileDialog::getOpenFileName(this, "打开图片", tr("c:\\"), tr("Images (*.png *.xpm *.jpg)"));
    QFile file(strPath);
    if(!file.open(QFile::ReadOnly))
    {
        QMessageBox::information(NULL, "Warning!!", "图片未选择!!");
        return;
    }else{
        QByteArray ba = file.readAll();
        imgfrompath = imdecode(vector<char>(ba.begin(), ba.end()), 1);
    }

    QList<brightness_lamp_info_out_t*> brightness_info1 = get_brightness_lamp_info_out_list(imgfrompath);
    print_brightness_lamp_info_list(brightness_info1);

    //打开本地图片2
    Mat imgfrompath2;
    QString strPath2 = QFileDialog::getOpenFileName(this, "打开图片", tr("c:\\"), tr("Images (*.png *.xpm *.jpg)"));
    QFile file2(strPath2);
    if(!file2.open(QFile::ReadOnly))
    {
        QMessageBox::information(NULL, "Warning!!", "图片未选择!!");
        return;
    }else{
        QByteArray ba2 = file2.readAll();
        imgfrompath2 = imdecode(vector<char>(ba2.begin(), ba2.end()), 1);
    }

    QList<brightness_lamp_info_out_t*> brightness_info2 = get_brightness_lamp_info_out_list(imgfrompath2);
    print_brightness_lamp_info_list(brightness_info2);

    //比较输出特征参数列表
    int ret = compare_img(brightness_info1,brightness_info2,&brightness_filter_model_info);
    if(ret <0)
        return;
    if(ret == 0)
         brightness_filter_model_info.algorithmInfo.algorithm_train_res = 1;

    free_list_memory(brightness_info1,brightness_info2);
}


void ui_algorithm_brightnessfilter::on_ui_algorithm_brightnessfilter_send_to_client_btn_clicked()
{
    if(brightness_filter_model_info.algorithmInfo.algorithm_train_res == 0) {
        QMessageBox::information(NULL, "Warning!!", "没有从数据库取得数据，请先点击\"算法训练\"!!");
        return;
    }
    int client_index = 0;
    for(int i=0;i<clientList.size();i++) {
        if(!strcmp((char*)clientList.at(i)->client_info.client_uuid,(char*)brightness_filter_model_info.algorithmInfo.clinet_uuid)) {
            client_index = i;
        }
    }
    //组织成json字符串  发送给对应的客户端
    cJSON* root = cJSON_CreateObject();
    cJSON_AddItemToObject(root,"cmd",cJSON_CreateString(CMD_SERVER_TRAIN_BRIGHTNESS_ALGORITHM));
    cJSON_AddItemToObject(root,"server_uuid",cJSON_CreateString(server_info.server_uuid));
    cJSON_AddItemToObject(root,"client_uuid",cJSON_CreateString((char*)brightness_filter_model_info.algorithmInfo.clinet_uuid));
    cJSON_AddItemToObject(root,"camera_name",cJSON_CreateString((char*)brightness_filter_model_info.algorithmInfo.camera_device_name));
    cJSON_AddItemToObject(root,"is_resize",cJSON_CreateNumber(brightness_filter_model_info.is_resize));
    cJSON_AddItemToObject(root,"imgorg_width",cJSON_CreateNumber(brightness_filter_model_info.imgcut_info.org_width));
    cJSON_AddItemToObject(root,"imgorg_height",cJSON_CreateNumber(brightness_filter_model_info.imgcut_info.org_height));
    cJSON_AddItemToObject(root,"imgcut_info_x",cJSON_CreateNumber(brightness_filter_model_info.imgcut_info.x));
    cJSON_AddItemToObject(root,"imgcut_info_y",cJSON_CreateNumber(brightness_filter_model_info.imgcut_info.y));
    cJSON_AddItemToObject(root,"imgcut_info_width",cJSON_CreateNumber(brightness_filter_model_info.imgcut_info.width));
    cJSON_AddItemToObject(root,"imgcut_info_height",cJSON_CreateNumber(brightness_filter_model_info.imgcut_info.height));
    cJSON_AddItemToObject(root,"num_row",cJSON_CreateNumber(brightness_filter_model_info.num_row));
    cJSON_AddItemToObject(root,"num_cols",cJSON_CreateNumber(brightness_filter_model_info.num_cols));
    cJSON_AddItemToObject(root,"maskVaule",cJSON_CreateNumber(brightness_filter_model_info.maskVaule));
    cJSON_AddItemToObject(root,"location_offset",cJSON_CreateNumber(brightness_filter_model_info.offset));
    cJSON_AddItemToObject(root,"off_x_offset",cJSON_CreateNumber(brightness_filter_model_info.off_x_offset));
    cJSON_AddItemToObject(root,"off_y_offset",cJSON_CreateNumber(brightness_filter_model_info.off_y_offset));
    cJSON_AddItemToObject(root,"time_offset",cJSON_CreateNumber(brightness_filter_model_info.time_offset));

    char* json_paras_info = cJSON_Print(root);
    qDebug() << "json::" << QString(json_paras_info);

    TcpSocketList.at(client_index)->TcpClientSocket->write(json_paras_info);
    QMessageBox::information(NULL, "About!!", "发送到客户端成功!!");
}

