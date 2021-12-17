#include "ui_algorithm_colorfilter.h"
#include "ui_ui_algorithm_colorfilter.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/opencv.hpp>
#include "ui_algorithm_setting.h"
using namespace cv;
using namespace std;

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


extern dbConnect_info_t m_dbconnect_info;           //数据库信息
extern QList<client_all_info_t*> clientList;        //管理客户端的列表
extern server_info_t server_info;                   //服务器信息
extern QList<client_net_info_t*>TcpSocketList;      //管理客户端socket ip port
static color_filter_model_t color_filter_model_info;      //颜色过滤模型

ui_algorithm_colorFilter::ui_algorithm_colorFilter(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ui_algorithm_colorFilter)
{
    ui->setupUi(this);
    setWindowTitle("颜色过滤器算法训练");
    ItemModel = new QStandardItemModel(this);
    ItemModel_compare = new QStandardItemModel(this);
    ui->ui_colorFilter_listview->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::SelectedClicked);

}

ui_algorithm_colorFilter::~ui_algorithm_colorFilter()
{
    delete ui;
    delete ItemModel;
    delete ItemModel_compare;
}

//快速排序
void quickSort(int arr[], int low, int high)
{
    int temp = 0;
    int i = low;
    int j = high;
    if (low < high) {
        temp = arr[low];//中心轴点
        while (i < j) {
            //从数组右边开始移动  j
            while ((j > i) && (arr[j] >= temp))
                j--;			//右侧数据的数值>中心轴点 移动j
            if (i < j) {									//右侧数据的数值<中心轴点 交换j与空点数值
                arr[i] = arr[j];
                i++;
            }

            //从数组左边开始移动  i
            while ((i < j) && (arr[i] < temp))
                i++;				//左侧数据的数值<中心轴点 移动i
            if (i < j) {									//左侧数据的数值>中心轴点 交换i与空点数值
                arr[j] = arr[i];
                j--;
            }
        }

        //i==j
        arr[i] = temp;//放置中心轴点数据

        //递归
        quickSort(arr, low, i - 1);
        quickSort(arr, i+1, high);
    }
}

//打开颜色过滤器  进行颜色过滤操作 得到hsv参数
void ui_algorithm_colorFilter::on_ui_colorFilter_open_btn_clicked()
{
   //颜色过滤器 文件对话框 自动加载截图路径
   Mat img_cut,imgHSV;
   QString strPath = img_cut_path;
   //qDebug("strPath = [%s]", strPath.toUtf8().data());

   QFile file(strPath);
   if(!file.open(QFile::ReadOnly))
   {
       QMessageBox::information(NULL, "Warning!!", "图片未选择!!");
       return;
   }else{
       QByteArray ba = file.readAll();
       img_cut = imdecode(vector<char>(ba.begin(), ba.end()), 1);
   }

    Mat mask;
    int hmin = 0, smin = 0, vmin = 0;
    int hmax = 179, smax = 255, vmax = 255;
    QString s_hmin,s_smin,s_vmin,s_hmax,s_smax,s_vmax;
    s_hmin.setNum(hmin);
    s_smin.setNum(smin);
    s_vmin.setNum(vmin);
    s_hmax.setNum(hmax);
    s_smax.setNum(smax);
    s_vmax.setNum(vmax);

    ui->ui_colorFilter_hmin_lab->setText(s_hmin);
    ui->ui_colorFilter_smin_lab->setText(s_smin);
    ui->ui_colorFilter_vmin_lab->setText(s_vmin);
    ui->ui_colorFilter_hmax_lab->setText(s_hmax);
    ui->ui_colorFilter_smax_lab->setText(s_smax);
    ui->ui_colorFilter_vmax_lab->setText(s_vmax);

    namedWindow("Trackbars", WINDOW_NORMAL); // Create Window
    createTrackbar("Hue Min", "Trackbars", &hmin, 179);
    createTrackbar("Hue Max", "Trackbars", &hmax, 179);
    createTrackbar("Sat Min", "Trackbars", &smin, 255);
    createTrackbar("Sat Max", "Trackbars", &smax, 255);
    createTrackbar("Val Min", "Trackbars", &vmin, 255);
    createTrackbar("Val Max", "Trackbars", &vmax, 255);
    filter_flag = true;
    while (true) {
        if(filter_flag == true) {
            cvtColor(img_cut, imgHSV, COLOR_BGR2HSV);//转换为HSV颜色模型
            Scalar lower(hmin, smin, vmin);//标量赋值
            Scalar upper(hmax, smax, vmax);//标量赋值

            inRange(imgHSV, lower, upper, mask);//图像二值化

            //同步过滤器到主界面
            s_hmin.setNum(hmin);
            s_smin.setNum(smin);
            s_vmin.setNum(vmin);
            s_hmax.setNum(hmax);
            s_smax.setNum(smax);
            s_vmax.setNum(vmax);
            ui->ui_colorFilter_hmin_lab->setText(s_hmin);
            ui->ui_colorFilter_smin_lab->setText(s_smin);
            ui->ui_colorFilter_vmin_lab->setText(s_vmin);
            ui->ui_colorFilter_hmax_lab->setText(s_hmax);
            ui->ui_colorFilter_smax_lab->setText(s_smax);
            ui->ui_colorFilter_vmax_lab->setText(s_vmax);

            imshow("Origin Picture", img_cut);
            imshow("Mask Picture", mask);
            waitKey(1);
        }
        else
            break;

    }
}


//关闭颜色过滤器
void ui_algorithm_colorFilter::on_ui_colorFilter_close_btn_clicked()
{
    filter_flag = false;
    destroyWindow("Origin Picture");
    destroyWindow("Mask Picture");
    destroyWindow("Trackbars");
}


//将通过HSV过滤的参数添加到listView中
void ui_algorithm_colorFilter::on_ui_colorFilter_add_btn_2_clicked()
{
    QByteArray bi = ui->ui_colorFilter_color_cmbx->currentText().toLatin1();
       if(bi.isEmpty()) {
           QMessageBox::information(NULL, "Warning!!", "color name can not empty!!");
           return;
       }

       if(ui->ui_colorFilter_hmin_lab->text().isEmpty() || ui->ui_colorFilter_hmax_lab->text().isEmpty() || ui->ui_colorFilter_smin_lab->text().isEmpty()
       || ui->ui_colorFilter_smax_lab->text().isEmpty() || ui->ui_colorFilter_vmin_lab->text().isEmpty() || ui->ui_colorFilter_vmax_lab->text().isEmpty()) {
           QMessageBox::information(NULL, "Warning!!", "HSV value can not empty!!");
           return;
       }

       int num = colorinfoList.size();
       bool same_flag = false;

       QString colorinfo = ui->ui_colorFilter_color_cmbx->currentText()
               + "," + ui->ui_colorFilter_hmin_lab->text() + "," + ui->ui_colorFilter_smin_lab->text() + "," + ui->ui_colorFilter_vmin_lab->text()
               + "," + ui->ui_colorFilter_hmax_lab->text() + "," + ui->ui_colorFilter_smax_lab->text() + "," + ui->ui_colorFilter_vmax_lab->text();

       qDebug() << "colorinfo::" << colorinfo <<Qt::endl;
       qDebug() << "colorinfo.size()::" << num <<Qt::endl;

       if(num == 0) {
           colorinfoList.append(colorinfo);
       }
       else {
           for(int i=0;i<num;i++) {
               qDebug() << "i::::" << i <<Qt::endl;
               if(ui->ui_colorFilter_color_cmbx->currentText() == colorinfoList[i].section(',', 0, 0).trimmed()  ) {
                   qDebug() << "the same" <<Qt::endl;
                   colorinfoList.removeAt(i);
                   ItemModel->removeRow(i);
                   colorinfoList.append(colorinfo);
                   same_flag = true;
                   break;
               }
           }
       }

       if((same_flag == false) && (num > 0)) {
           colorinfoList.append(colorinfo);
       }
       qDebug() << "后面的after colorinfo.size()::" << colorinfoList.size() <<Qt::endl;

       QStandardItem *item = new QStandardItem(colorinfo);
       ItemModel->appendRow(item);
       ui->ui_colorFilter_listview->setModel(ItemModel);
       //ui->ui_colorFilter_listview->setFixedSize(200,300);
}


//将实际结果集添加到listView中 其与颜色过滤参数列表中的颜色顺序需要相同
void ui_algorithm_colorFilter::on_ui_colorFilter_match_add_btn_clicked()
{
    if((ui->ui_colorFilter_match_colorname_edt->text().isEmpty()) || (ui->ui_colorFilter_match_colornum_edt->text().isEmpty())) {
            QMessageBox::information(NULL, "Warning!!", "compare color name and num value can not empty!!");
            return;
        }
         QString lamp_info = ui->ui_colorFilter_match_colorname_edt->text() + "," + ui->ui_colorFilter_match_colornum_edt->text();
        int num = lampCompareList.size();
        bool same_flag = false;
        if(num == 0) {
            lampCompareList.append(lamp_info);
            ui->ui_colorFilter_color_cmbx->addItem(ui->ui_colorFilter_match_colorname_edt->text());
        }
        else {
            for(int i=0;i<num;i++) {
                qDebug() << "i::::" << i <<Qt::endl;
                if(ui->ui_colorFilter_match_colorname_edt->text() == lampCompareList[i].section(',', 0, 0).trimmed()  ) {
                    qDebug() << "the same" <<Qt::endl;
                    lampCompareList.removeAt(i);
                    ItemModel_compare->removeRow(i);
                    lampCompareList.append(lamp_info);
                    same_flag = true;
                    break;
                }
            }
        }

        if((same_flag == false) && (num > 0)) {
            lampCompareList.append(lamp_info);
            ui->ui_colorFilter_color_cmbx->addItem(ui->ui_colorFilter_match_colorname_edt->text());
        }
        qDebug() << "后面的after lamp_info.size()::" << lampCompareList.size() <<Qt::endl;

        QStandardItem *item = new QStandardItem(lamp_info);
        ItemModel_compare->appendRow(item);
        ui->ui_colorFilter_compare_listview->setModel(ItemModel_compare);
        //ui->ui_colorFilter_compare_listview->setFixedSize(200,300);
}

//删除实际结果集
void ui_algorithm_colorFilter::on_ui_colorFilter_match_cancel_btn_clicked()
{
    QItemSelectionModel *selmodel = ui->ui_colorFilter_compare_listview->selectionModel();
    QModelIndexList modelIndexlist = selmodel->selectedIndexes();
    for(int i=0;i<modelIndexlist.size();i++) {
        QString str =modelIndexlist.at(i).data().toString();
        for(int j =0;j<lampCompareList.size();j++) {
            if(lampCompareList[j].section(',', 0, 0).trimmed() == str.section(',',0,0).trimmed()) {
             lampCompareList.removeAt(i);
            }
        }
     ui->ui_colorFilter_compare_listview->model()->removeRow(modelIndexlist.at(i).row());
   }
}

//生成特征参数
QList<lamp_info_out_t*> color_filter_model_run(Mat imgOrg)//传入一张源图像 //模型输出指示灯识别参数
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
    QList<lamp_info_out_t*>  modelout_lampinfoList;
    Mat imgCopy,imgResize,imgCut,imghsv,imgMask;
    Mat imgDst;
    Mat imgPutText_dynamic;
    imgOrg.copyTo(imgCopy);
    imgOrg.copyTo(imgPutText_dynamic);

    qDebug() << "图像宽::" << imgCopy.cols;
    qDebug() << "图像高::" << imgCopy.rows;

    QList<int> yList;//纵坐标列表


    //是否缩放 输入参数1
    if(color_filter_model_info.is_resize == true) {
         cv::resize(imgCopy,imgResize,Size(),0.5,0.5);
    }

    //裁剪 输入参数2
    Rect rectRoi(color_filter_model_info.imgcut_info.x,
                 color_filter_model_info.imgcut_info.y,
                 color_filter_model_info.imgcut_info.width,
                 color_filter_model_info.imgcut_info.height);
    imgCut = imgResize(rectRoi);

    //转换为HSV颜色模型
    cvtColor(imgCut, imghsv, COLOR_BGR2HSV);

    //颜色过滤 二值化处理 输入参数3
    for (int i = 0; i < color_filter_model_info.color_num; i++) {
        //输出参数结构体初始化
        lamp_info_out_t* lamp_info_out = new lamp_info_out_t ;
        memset(lamp_info_out,0,sizeof(char)*20 + sizeof(int));

        strcpy(lamp_info_out->lamp_color,color_filter_model_info.hsv_color_infoList.at(i)->lamp_color);
        qDebug() << "过滤颜色：："<< lamp_info_out->lamp_color;

        Scalar lower(color_filter_model_info.hsv_color_infoList.at(i)->hmin,
                     color_filter_model_info.hsv_color_infoList.at(i)->smin,
                     color_filter_model_info.hsv_color_infoList.at(i)->vmin);

        Scalar upper(color_filter_model_info.hsv_color_infoList.at(i)->hmax,
                     color_filter_model_info.hsv_color_infoList.at(i)->smax,
                     color_filter_model_info.hsv_color_infoList.at(i)->vmax);

        inRange(imghsv, lower, upper, imgMask);
        imshow("img cut mask mask",imgMask);
        waitKey(1000);

        //添加轮廓检测
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours(imgMask, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);//轮廓检测函数
        vector<vector<Point>> conPoly(contours.size()); //多边形拟合曲线 判断轮廓形状
        vector<Rect> boundRect(contours.size());  //轮廓边界最小矩形
        qDebug() << "contours.size::" << contours.size();    //轮廓数量

        for (int j = 0; j < (int)contours.size(); j++)
        {
            //通过面积过滤一下杂点  输入参数4
            int area = contourArea(contours[j]);
            qDebug() <<"area:" << area ;

            if ((area > color_filter_model_info.area_info.area_min ) && (area < color_filter_model_info.area_info.area_max))//输入信息6
            {
                lamp_info_out->lamp_color_num++;
                qDebug() <<  "lamp_color_num:" << lamp_info_out->lamp_color_num;

                button_Point_t* button_info = new button_Point_t;
                memset(button_info,0,sizeof(button_Point_t));
                float peri = arcLength(contours[j], true);//计算封闭曲线的周长
                approxPolyDP(contours[j], conPoly[j], 0.02 * peri, true);//多边形拟合曲线
                //qDebug() <<  "conPoly size:" << conPoly[j].size();
                boundRect[j] = boundingRect(conPoly[j]);//轮廓垂直边界最小矩形

                if(i==0 && j == 0) {
                    button_info->x        = boundRect[j].x;
                    button_info->y        = boundRect[j].y;
                    button_info->width    = boundRect[j].width;
                    button_info->height   = boundRect[j].height;
                    button_info->sum      = button_info->x + button_info->y  * LOCATION_OFFSET;
                    yList.append(button_info->y);
                }
                else {
                    button_info->x        = boundRect[j].x;

                    //对纵坐标进行归一化处理
                    for(int k=0;k<yList.size();k++) {
                        int sub_abs = qAbs(boundRect[j].y - yList.at(k));
                        if(sub_abs > boundRect[j].height) {
                            yList.append(boundRect[j].y);
                            button_info->y = boundRect[j].y;
                        }
                        else {
                            button_info->y = yList.at(k);
                            break;
                        }
                    }

                    button_info->width    = boundRect[j].width;
                    button_info->height   = boundRect[j].height;
                    button_info->sum      = button_info->x + button_info->y  * LOCATION_OFFSET;
                }
                qDebug() <<  "button_info->y:" << button_info->y;

                lamp_info_out->button_info_List.append(button_info);

                //在对应坐标点绘制轮廓 并标记颜色标签文本
                //drawContours(imgCut, conPoly, j, Scalar(255, 0, 255), 2);
                //putText(imgCut,lamp_info_out->lamp_color,
                 //       Size(lamp_info_out->button_info_List.at(lamp_info_out->lamp_color_num-1)->x-10,lamp_info_out->button_info_List.at(lamp_info_out->lamp_color_num - 1)->y-5),
                //        cv::FONT_HERSHEY_COMPLEX_SMALL,0.5,cv::Scalar(0, 255, 255),1,LINE_4,0);
                //imshow("contours check model", imgCut);
                //waitKey(1000);


                int org_x = lamp_info_out->button_info_List.at(lamp_info_out->lamp_color_num-1)->x + color_filter_model_info.imgcut_info.x;
                int org_y = lamp_info_out->button_info_List.at(lamp_info_out->lamp_color_num-1)->y + color_filter_model_info.imgcut_info.y;
                //qDebug() << "org_x:::" << org_x;
                //qDebug() << "org_y:::" << org_y;

                //缩略图标记
                //putText(imgResize,lamp_info_out->lamp_color,
                //        Size(org_x,org_y),
                //        cv::FONT_HERSHEY_COMPLEX_SMALL,0.5,cv::Scalar(0, 255, 255),1,LINE_4,0);
                //imshow("imgResize", imgResize);
                //waitKey(1000);

                //原图标记
                //时间标记
                QDateTime datatime = QDateTime::currentDateTime();
                QString time = datatime.toString("yyyy-MM-dd hh:mm:ss");
                QByteArray ba = time.toLatin1();

                Mat imgPutText;
                imgCopy.copyTo(imgPutText);
                putText(imgPutText,ba.data(),
                        Size(color_filter_model_info.time_offset,color_filter_model_info.time_offset),
                        cv::FONT_HERSHEY_COMPLEX_SMALL,3,cv::Scalar(0, 0, 255),1,LINE_4,0);
                putText(imgPutText_dynamic,lamp_info_out->lamp_color,
                        Size(org_x * 2 - 5,org_y * 2 - 10),
                        cv::FONT_HERSHEY_COMPLEX_SMALL,0.5,cv::Scalar(0, 255, 255),1,LINE_4,0);

                addWeighted(imgPutText, 0.3, imgPutText_dynamic, 0.7, 0.0, imgDst); //这里调用了addWeighted函数，得到的结果存储在dst中
                imshow("imgDstimgDst", imgDst);
                //QString imgname = QString("%1%2%3%4").arg(QString(lamp_info_out->lamp_color)).arg("_").arg(lamp_info_out->lamp_color_num).arg(".jpg");
                //QByteArray ba1 = imgname.toLatin1();
                //imwrite(ba1.data(),imgDst);
                waitKey(1000);

            }//end area
        }//end contours
        modelout_lampinfoList.append(lamp_info_out);
    }//end color

    //按钮的总数 分配位置信息数组内存
    int lamp_info_list1_num_all = 0;
    for(int i=0;i<modelout_lampinfoList.size();i++) {
         lamp_info_list1_num_all +=  modelout_lampinfoList.at(i)->lamp_color_num;
    }

    //位置数组
    int* arr = (int*)malloc(sizeof(int) * lamp_info_list1_num_all);
    int count= 0;
    for(int i=0;i<modelout_lampinfoList.size();i++) {
        for(int j=0;j<modelout_lampinfoList.at(i)->button_info_List.size();j++)
        {
            arr[count] = modelout_lampinfoList.at(i)->button_info_List.at(j)->sum;
            count++;
        }
    }

    //快速排序 生成位置偏移索引
    quickSort(arr,0,lamp_info_list1_num_all-1);
    for(int i=0;i<modelout_lampinfoList.size();i++) {
        for(int j=0;j<modelout_lampinfoList.at(i)->button_info_List.size();j++)
        {
            for(int z=0;z<lamp_info_list1_num_all;z++) {
                if(modelout_lampinfoList.at(i)->button_info_List.at(j)->sum == arr[z])
                    modelout_lampinfoList.at(i)->button_info_List.at(j)->location_index = z;
            }
        }
    }
    free(arr);


    destroyWindow("img cut cut");
    destroyWindow("img cut mask mask");
    destroyWindow("imgOrg");

    imgCopy.release();
    imgResize.release();
    imgCut.release();
    imghsv.release();
    imgMask.release();
    imgDst.release();
    imgPutText_dynamic.release();

    return modelout_lampinfoList;
}

void print_model_in_info(color_filter_model_t* color_filter_model)
{
    //打印指示灯模型输入信息
    qDebug() << "指示灯共有的颜色数量::" << color_filter_model->color_num << Qt::endl;
    qDebug() << "指示灯截图坐标(org_w,org_h,x,y,w,h)::["
                                           << color_filter_model->imgcut_info.org_width << ","
                                           << color_filter_model->imgcut_info.org_height << ","
                                           << color_filter_model->imgcut_info.x << ","
                                           << color_filter_model->imgcut_info.y << ","
                                           << color_filter_model->imgcut_info.width << ","
                                           << color_filter_model->imgcut_info.height << "]"<< Qt::endl;
    qDebug() << "是否缩放::" << color_filter_model->is_resize << Qt::endl;
    qDebug() << "面积过滤数值(min,max)::[" << color_filter_model->area_info.area_min <<","
                                         << color_filter_model->area_info.area_max << "]" << Qt::endl;

    for(int i = 0;i < color_filter_model->color_num;i++) {
        qDebug() << "指示灯颜色::" << color_filter_model->hsv_color_infoList.at(i)->lamp_color;
        qDebug() << "HSV参数:::(hmin,smin,vmin,hmax,smax,vmax):::["
                 << color_filter_model->hsv_color_infoList.at(i)->hmin << ","
                 << color_filter_model->hsv_color_infoList.at(i)->smin << ","
                 << color_filter_model->hsv_color_infoList.at(i)->vmin << ","
                 << color_filter_model->hsv_color_infoList.at(i)->hmax << ","
                 << color_filter_model->hsv_color_infoList.at(i)->smax << ","
                 << color_filter_model->hsv_color_infoList.at(i)->vmax << "]" << Qt::endl;
    }



}

void print_model_out_info(QList<lamp_info_out_t*>lamp_infoList_out,int color_num)
{
    //打印指示灯输出信息
    for(int i = 0;i < color_num;i++) {
        qDebug() << "指示灯颜色::" << lamp_infoList_out.at(i)->lamp_color
                 << "指示灯数量::" << lamp_infoList_out.at(i)->lamp_color_num;

        for(int j=0;j<lamp_infoList_out.at(i)->button_info_List.size();j++)
        {
            qDebug() << "坐标系(x,y,w,h,-):::["
                    << lamp_infoList_out.at(i)->button_info_List.at(j)->x << ","
                    << lamp_infoList_out.at(i)->button_info_List.at(j)->y << ","
                    << lamp_infoList_out.at(i)->button_info_List.at(j)->width << ","
                    << lamp_infoList_out.at(i)->button_info_List.at(j)->height << ","
                    << lamp_infoList_out.at(i)->button_info_List.at(j)->sum << "]"
                    <<  Qt::endl;
        }

    }
}

int ui_algorithm_colorFilter::save_to_database(lamp_color_template_info_t* lamp_color_template_info,color_filter_model_t* color_filter_model,
                      QString& client_uuid,int camera_id,int algorithm_id)
{
    sqlBase sqlbase;
    lamp_color_template_info->client_uuid = client_uuid;
    lamp_color_template_info->camera_id = camera_id;
    lamp_color_template_info->algorithm_id = algorithm_id;

    QString sql = "SELECT MAX(lamp_color_template_info.lamp_template_no) FROM lamp_color_template_info;";
    lamp_color_template_info->lamp_template_no = sqlbase.get_maxid(m_dbconnect_info.dbconn,sql) + 1;
    lamp_color_template_info->is_resize = color_filter_model->is_resize;
    lamp_color_template_info->imgorg_width = color_filter_model->imgcut_info.org_width;
    lamp_color_template_info->imgorg_height = color_filter_model->imgcut_info.org_height;
    lamp_color_template_info->imgcut_x = color_filter_model->imgcut_info.x;
    lamp_color_template_info->imgcut_y = color_filter_model->imgcut_info.y;
    lamp_color_template_info->imgcut_width = color_filter_model->imgcut_info.width;
    lamp_color_template_info->imgcut_height = color_filter_model->imgcut_info.height;
    lamp_color_template_info->time_offset = color_filter_model->time_offset;
    lamp_color_template_info->template_image_org_path = img_org_path;
    lamp_color_template_info->template_image_cut_path = img_cut_path;
    lamp_color_template_info->area_min = color_filter_model->area_info.area_min;
    lamp_color_template_info->area_max = color_filter_model->area_info.area_max;
    lamp_color_template_info->color_num = color_filter_model->color_num;
    for(int i=0;i<lamp_color_template_info->color_num;i++) {
        QString str1 = QString("%1%2").arg(color_filter_model->hsv_color_infoList.at(i)->lamp_color).arg(",");
        QString str2 = QString("%1%2").arg(color_filter_model->hsv_color_infoList.at(i)->hmin).arg(",");
        QString str3 = QString("%1%2").arg(color_filter_model->hsv_color_infoList.at(i)->smin).arg(",");
        QString str4 = QString("%1%2").arg(color_filter_model->hsv_color_infoList.at(i)->vmin).arg(",");
        QString str5 = QString("%1%2").arg(color_filter_model->hsv_color_infoList.at(i)->hmax).arg(",");
        QString str6 = QString("%1%2").arg(color_filter_model->hsv_color_infoList.at(i)->smax).arg(",");
        QString str7 = QString::number(color_filter_model->hsv_color_infoList.at(i)->vmax);

        QString str = str1 + str2 + str3 + str4 + str5 + str6 + str7;
        qDebug() << "str:::" << str;
        lamp_color_template_info->colorinfoList.append(str);
    }

     int ret = sqlbase.insert_to_lamp_color_template_info(m_dbconnect_info.dbconn,lamp_color_template_info);
     return ret;
}

int ui_algorithm_colorFilter::get_model_info_from_database(QString& client_uuid,int camera_id,int algorithm_id)
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;

    QSqlQuery query(m_dbconnect_info.dbconn);
    query.prepare("SELECT * FROM lamp_color_template_info \
                   WHERE lamp_color_template_info.client_uuid = ? \
                   AND lamp_color_template_info.camera_id = ?  \
                   AND lamp_color_template_info.algorithm_id = ?;");

    query.addBindValue(client_uuid);
    qDebug() << "client_uuid:::" << client_uuid;

    query.addBindValue(camera_id);
    qDebug() << "camera_id:::" << camera_id;

    query.addBindValue(algorithm_id);
    qDebug() << "algorithm_id:::" << algorithm_id;
    if(query.exec()) {
        while(query.next()) {
             //lamp_color_template_info->lamp_template_no           = query.value(0).toInt();
             //lamp_color_template_info->client_uuid                = query.value(1).toString();
             //lamp_color_template_info->camera_id                  = query.value(2).toInt();
             //lamp_color_template_info->algorithm_id               = query.value(3).toInt();
             color_filter_model_info.imgcut_info.org_width          = query.value(4).toInt();
             color_filter_model_info.imgcut_info.org_height         = query.value(5).toInt();
             color_filter_model_info.is_resize                      = query.value(6).toInt();
             color_filter_model_info.imgcut_info.x                  = query.value(7).toInt();
             color_filter_model_info.imgcut_info.y                  = query.value(8).toInt();
             color_filter_model_info.imgcut_info.width              = query.value(9).toInt();
             color_filter_model_info.imgcut_info.height             = query.value(10).toInt();
             color_filter_model_info.time_offset                    = query.value(11).toInt();
             //color_filter_model_info->template_image_org_path     = query.value(12).toString();
             //color_filter_model_info->template_image_cut_path     = query.value(13).toString();
             color_filter_model_info.area_info.area_min             = query.value(14).toInt();
             color_filter_model_info.area_info.area_max             = query.value(15).toInt();
             color_filter_model_info.color_num                      = query.value(16).toInt();
             for(int i=17;i<=26;i++) {
                 if(query.value(i).toString() != "") {
                     QString str = query.value(i).toString();
                     hsv_color_info_t* hsv_color_info = new hsv_color_info_t;
                     memset(hsv_color_info,0,sizeof(hsv_color_info_t)); //初始化
                     QByteArray temp_bi = str.section(',', 0, 0).trimmed().toLatin1();
                     strcpy(hsv_color_info->lamp_color,temp_bi.data());
                     hsv_color_info->hmin = str.section(',', 1, 1).trimmed().toInt();
                     hsv_color_info->smin = str.section(',', 2, 2).trimmed().toInt();
                     hsv_color_info->vmin = str.section(',', 3, 3).trimmed().toInt();
                     hsv_color_info->hmax = str.section(',', 4, 4).trimmed().toInt();
                     hsv_color_info->smax = str.section(',', 5, 5).trimmed().toInt();
                     hsv_color_info->vmax = str.section(',', 6, 6).trimmed().toInt();

                     color_filter_model_info.hsv_color_infoList.append(hsv_color_info);
                 }
             }

        }
    }
    else {
        qDebug()<< "数据库错误::" << query.lastError()<<Qt::endl;
        return -1;
    }

    query.clear();

    return 1;
}
/*
QString get_client_uuid(QString& client_name)
{
     QString client_uuid;
    for(int i=0;i<clientList.size();i++) {
        if(QString((char*)clientList.at(i)->client_info.client_name) == client_name)
            client_uuid = QString((char*)clientList.at(i)->client_info.client_uuid);
    }
    return  client_uuid;
}

int get_camera_id(QString& camera_name,QString& client_uuid)
{
    sqlBase sqlbase;
    int camera_id;

    for(int i=0;i<clientList.size();i++) {
        for(int j=0;j<clientList.at(i)->camera_num;j++) {
            if(QString((char*)clientList.at(i)->camera_device_info[j].camera_device_name) == camera_name)
                camera_id = sqlbase.get_cameraidbyname(m_dbconnect_info.dbconn,camera_name,client_uuid);
        }
    }
    return camera_id;
}

int get_algorithm_id(QString& algorithm_name,QString& algorithm_version)
{
    sqlBase sqlbase;
    int algorithm_id;
    algorithm_id = sqlbase.get_algorithmid(m_dbconnect_info.dbconn,algorithm_name,algorithm_version);
    return algorithm_id;
}

*/
//执行算法 识别指示灯 并同实际结果集进行比较 训练过程
void ui_algorithm_colorFilter::on_ui_colorFilter_algorithm_train_btn_clicked()
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
    qDebug() << "本次训练结果::" << color_filter_model_info.algorithmInfo.algorithm_train_res;

    sqlBase sqlbase;
    Mat imgOrg,imgCopy;

    //客户端信息 摄像头信息  算法信息
    QString client_uuid = QString((char*)color_filter_model_info.algorithmInfo.clinet_uuid);
    int camera_id = color_filter_model_info.algorithmInfo.camera_id;
    int algorithm_id = color_filter_model_info.algorithmInfo.algorithm_id;

    //判断该客户端所对应的相机拍摄的指示灯图片是否训练
    if(database_flag == false && color_filter_model_info.algorithmInfo.algorithm_train_res == 0) {//模板不存在 训练生成参数

        if(ui->ui_colorFilter_time_text_edt->text().isEmpty()) {
             QMessageBox::information(NULL, "Warning!!", "时间标记坐标不能为空!!");
             return;
        }
        else {
            color_filter_model_info.time_offset = ui->ui_colorFilter_time_text_edt->text().toInt();
        }

        QMessageBox::information(NULL, "About!!", "将训练生成参数!!");
        if(colorinfoList.size() == 0 || lampCompareList.size() == 0 || colorinfoList.size() != lampCompareList.size()) {
            QMessageBox::information(NULL, "Warning!!", "指示灯颜色为空或信息不匹配!!");
            return;
        }
        if((ui->ui_colorFilter_area_min_edt->text().isEmpty()) || (ui->ui_colorFilter_area_max_edt->text().isEmpty())) {
            QMessageBox::information(NULL, "Warning!!", "面积不能为空!!");
            return;
        }

        if(ui->ui_colorFilter_area_min_edt->text().toInt() >= ui->ui_colorFilter_area_max_edt->text().toInt()) {
            QMessageBox::information(NULL, "Warning!!", "面积值设置错误!!");
            return;
        }

        //加载原图
        QString strPath = img_org_path;
        QFile file(strPath);
        if(!file.open(QFile::ReadOnly))
        {
            QMessageBox::information(NULL, "Warning!!", "图片未选择!!");
            return;
        }else{
            QByteArray ba = file.readAll();
            imgOrg = imdecode(vector<char>(ba.begin(), ba.end()), 1);
        }
        imgOrg.copyTo(imgCopy);

        color_filter_model_info.area_info.area_min = ui->ui_colorFilter_area_min_edt->text().toInt();
        color_filter_model_info.area_info.area_max = ui->ui_colorFilter_area_max_edt->text().toInt();

        color_filter_model_info.color_num =  colorinfoList.size();
        QByteArray temp_bi;
        //对不同颜色的指示灯进行颜色过滤 得到相关的轮廓坐标以及颜色
        for (int i = 0; i < color_filter_model_info.color_num; i++) { //i代表指示灯颜色
            hsv_color_info_t* hsv_color_info = new hsv_color_info_t;//动态分配内存
            memset(hsv_color_info,0,sizeof(hsv_color_info_t)); //对于不同的颜色 先初始化
            //得到颜色参数
            temp_bi = colorinfoList[i].section(',', 0, 0).trimmed().toLatin1();
            strcpy(hsv_color_info->lamp_color,temp_bi.data());
            //得到HSV参数
            hsv_color_info->hmin = colorinfoList[i].section(',', 1, 1).trimmed().toInt();
            hsv_color_info->smin = colorinfoList[i].section(',', 2, 2).trimmed().toInt();
            hsv_color_info->vmin = colorinfoList[i].section(',', 3, 3).trimmed().toInt();
            hsv_color_info->hmax = colorinfoList[i].section(',', 4, 4).trimmed().toInt();
            hsv_color_info->smax = colorinfoList[i].section(',', 5, 5).trimmed().toInt();
            hsv_color_info->vmax = colorinfoList[i].section(',', 6, 6).trimmed().toInt();
            color_filter_model_info.hsv_color_infoList.append(hsv_color_info);
        }

        //定义指示灯颜色输出列表
        QList<lamp_info_out_t*>lamp_infoList_out;
        lamp_infoList_out = color_filter_model_run(imgCopy);
        print_model_out_info(lamp_infoList_out,color_filter_model_info.color_num);


         //检查算法测试结果 模型输出参数同实际结果比较 仅匹配颜色信息 颜色数量  实际模型中按需求增加位置信息筛选
         int count_compare = 0;
         for(int i = 0;i < color_filter_model_info.color_num;i++) {
             QByteArray bi_color = lampCompareList[i].section(',',0,0).trimmed().toLatin1();
             int  lampnum = lampCompareList[i].section(',',1,1).trimmed().toInt();
             qDebug() << "指示灯颜色::" << lamp_infoList_out.at(i)->lamp_color;
             if(!strcmp(lamp_infoList_out.at(i)->lamp_color,bi_color.data())) {//比较颜色内容是否与实际相同
                 qDebug() << "指示灯共有的颜色数量::" << lamp_infoList_out.at(i)->lamp_color_num;
                 if(lamp_infoList_out.at(i)->lamp_color_num = lampnum) {//比较该颜色指示灯数量是否与实际想用
                     qDebug() << "匹配成功" << Qt::endl;
                     count_compare++;
                 }
                 else
                     break;
             }
             else
                 break;
         }

         if(count_compare == color_filter_model_info.color_num) {

             QMessageBox::information(NULL, "About!!", "算法测试成功!!");
             //打印指示灯模型输入输出信息
             print_model_in_info(&color_filter_model_info);
             print_model_out_info(lamp_infoList_out,color_filter_model_info.color_num);
             lamp_infoList_out.clear();//算法测试成功 保留输入参数删除输出参数


             //数据库用结构体定义并初始化
             lamp_color_template_info_t* lamp_color_template_info = new lamp_color_template_info_t;
             memset(lamp_color_template_info,0,sizeof(int)*14);
             lamp_color_template_info->client_uuid = "";
             lamp_color_template_info->template_image_cut_path = "";

             //保存到数据库中
             int ret = save_to_database(lamp_color_template_info,&color_filter_model_info,client_uuid,camera_id,algorithm_id);
             if(ret > 0) {
                 QMessageBox::information(NULL, "About!!", "插入到颜色过滤算法模板数据库中成功!!");
                 color_filter_model_info.algorithmInfo.algorithm_train_res = 1;
             }
             else if(ret < 0)
                 QMessageBox::information(NULL, "About!!", "插入到颜色过滤算法模板数据库中失败!!");

             //释放内存
             delete lamp_color_template_info;
             lamp_color_template_info = NULL;

         }
         else {
             QMessageBox::information(NULL, "About!!", "算法测试失败，请调整相关的参数!!");
             color_filter_model_info.algorithmInfo.algorithm_train_res = 0;
             color_filter_model_info.hsv_color_infoList.clear();//算法测试失败 删除一定的输入信息
             lamp_infoList_out.clear();//算法测试失败 删除输出信息
             return;
         }
    }
    else if(database_flag == true || color_filter_model_info.algorithmInfo.algorithm_train_res ==1) {//模板存在 从数据库中获取数据  或者训练之后在同一页面点击
          QMessageBox::information(NULL, "About!!", "将通过数据库中保存的参数运行模型!!");

          qDebug() << "client_uuid::" <<    client_uuid;
          qDebug() << "camera_id::" <<      camera_id;
          qDebug() << "algorithm_id::" <<   algorithm_id;

          int ret = get_model_info_from_database(client_uuid,camera_id,algorithm_id);
          if(ret > 0) {
              QMessageBox::information(NULL, "About!!", "从数据库中得到模型参数成功!!");
              color_filter_model_info.algorithmInfo.algorithm_train_res = 1;

              ui->ui_colorFilter_area_min_edt->setText(QString::number(color_filter_model_info.area_info.area_min));
              ui->ui_colorFilter_area_max_edt->setText(QString::number(color_filter_model_info.area_info.area_max));
              ui->ui_colorFilter_time_text_edt->setText(QString::number(color_filter_model_info.time_offset));


          }
          else {
              QMessageBox::information(NULL, "About!!", "从数据库中得到模型参数失败!!");
              color_filter_model_info.algorithmInfo.algorithm_train_res = 0;
              color_filter_model_info.hsv_color_infoList.clear();//算法测试失败 删除一定的输入信息
          }

    }
    imgCopy.release();
}

void ui_algorithm_colorFilter::slot_sendto_algorithm_colorFilte(imgcut_info_t *imgcut_info)
{
    memset(&color_filter_model_info,0,
           sizeof(area_info_t)+sizeof(imgcut_Point_t)+sizeof(algorithm_info_t)+
           sizeof(int)*2+sizeof(bool));//初始化

    qDebug() <<"是否存在数据库中::" << imgcut_info->database_flag;
    database_flag = imgcut_info->database_flag;

    qDebug() <<"原图宽::" << imgcut_info->imgcut_parms.org_width;
    color_filter_model_info.imgcut_info.org_width= imgcut_info->imgcut_parms.org_width;

    qDebug() <<"原图长::" << imgcut_info->imgcut_parms.org_height;
    color_filter_model_info.imgcut_info.org_height= imgcut_info->imgcut_parms.org_height;

    qDebug() <<"截图横坐标::" << imgcut_info->imgcut_parms.x;
    color_filter_model_info.imgcut_info.x = imgcut_info->imgcut_parms.x;

    qDebug() <<"截图纵坐标::" << imgcut_info->imgcut_parms.y;
     color_filter_model_info.imgcut_info.y = imgcut_info->imgcut_parms.y;

    qDebug() <<"截图宽::"    << imgcut_info->imgcut_parms.width;
     color_filter_model_info.imgcut_info.width = imgcut_info->imgcut_parms.width;

    qDebug() <<"截图高::"    << imgcut_info->imgcut_parms.height;
     color_filter_model_info.imgcut_info.height = imgcut_info->imgcut_parms.height;

    qDebug() <<"客户端名字::"  <<QString((char*)imgcut_info->client_name);
    strcpy((char*)color_filter_model_info.algorithmInfo.client_name,(char*)imgcut_info->client_name);
    qDebug() <<"客户端uuid::"  <<QString((char*)imgcut_info->client_uuid);
    strcpy((char*)color_filter_model_info.algorithmInfo.clinet_uuid,(char*)imgcut_info->client_uuid);

    qDebug() <<"相机名字::"   << QString((char*)imgcut_info->camera_device_name);
    strcpy((char*)color_filter_model_info.algorithmInfo.camera_device_name,(char*)imgcut_info->camera_device_name);
    qDebug() <<"相机id::"  << imgcut_info->camera_id;
    color_filter_model_info.algorithmInfo.camera_id = imgcut_info->camera_id;

    qDebug() <<"算法名字::"   << QString((char*)imgcut_info->algorithm_name);
    strcpy((char*)color_filter_model_info.algorithmInfo.algorithm_name,(char*)imgcut_info->algorithm_name);
    qDebug() <<"算法版本号::" << QString((char*)imgcut_info->algorithm_version);
    strcpy((char*)color_filter_model_info.algorithmInfo.algorithm_version,(char*)imgcut_info->algorithm_version);
    qDebug() <<"算法id::"  << imgcut_info->algorithm_id;
    color_filter_model_info.algorithmInfo.algorithm_id = imgcut_info->algorithm_id;

    qDebug() <<"保存路径::" << QString((char*)imgcut_info->save_path);
    img_cut_path = QString((char*)imgcut_info->save_path);

    qDebug() <<"原图路径::" << QString((char*)imgcut_info->org_path);
    img_org_path = QString((char*)imgcut_info->org_path);


    qDebug() << "是否需要对图像进行缩放操作：" << imgcut_info->is_resize;
    color_filter_model_info.is_resize = imgcut_info->is_resize;

    ui->ui_colorFilter_clientname->setText(QString((char*)imgcut_info->client_name));
    ui->ui_colorFilter_clientname->setReadOnly(true);
    ui->ui_colorFilter_cameraname->setText(QString((char*)imgcut_info->camera_device_name));
    ui->ui_colorFilter_cameraname->setReadOnly(true);
    ui->ui_colorFilter_algorithm_name->setText(QString((char*)imgcut_info->algorithm_name));
    ui->ui_colorFilter_algorithm_name->setReadOnly(true);
    ui->ui_colorFilter_algorithm_version->setText(QString((char*)imgcut_info->algorithm_version));
    ui->ui_colorFilter_algorithm_version->setReadOnly(true);

    if(database_flag == true) {
        ui->ui_colorFilter_color_cmbx->setEnabled(false);
        ui->ui_colorFilter_open_btn->setEnabled(false);
        ui->ui_colorFilter_close_btn->setEnabled(false);
        ui->ui_colorFilter_add_btn_2->setEnabled(false);
        ui->ui_colorFilter_match_add_btn->setEnabled(false);
        ui->ui_colorFilter_cancel_btn->setEnabled(false);
        ui->ui_colorFilter_match_cancel_btn->setEnabled(false);
        ui->ui_colorFilter_area_min_edt->setReadOnly(true);
        ui->ui_colorFilter_area_max_edt->setReadOnly(true);
        ui->ui_colorFilter_time_text_edt->setReadOnly(true);

    }
}

void ui_algorithm_colorFilter::slot_clear_model_info()
{
    colorinfoList.clear();
    lampCompareList.clear();
    color_filter_model_info.hsv_color_infoList.clear();
}



//删除HSV参数
void ui_algorithm_colorFilter::on_ui_colorFilter_cancel_btn_clicked()
{
    QItemSelectionModel *selmodel = ui->ui_colorFilter_listview->selectionModel();
    QModelIndexList modelIndexlist = selmodel->selectedIndexes();
    for(int i=0;i<modelIndexlist.size();i++) {
        QString str =modelIndexlist.at(i).data().toString();
        ui->ui_colorFilter_listview->model()->removeRow(modelIndexlist.at(i).row());
        for(int j =0;j<colorinfoList.size();j++) {
            if(colorinfoList[j].section(',', 0, 0).trimmed() == str.section(',',0,0).trimmed()) {
             colorinfoList.removeAt(i);
            }
     }

    }
}


void ui_algorithm_colorFilter::on_ui_colorFilter_algorithm_compare_train_btn_clicked()//测试不同的图片 并执行比较算法 判断这两张图像是否相同
{
    //先判断算法是否训练通过
    if(color_filter_model_info.algorithmInfo.algorithm_train_res ==0 && database_flag ==0) {
         QMessageBox::information(NULL, "About!!", "算法模型未训练通过!!");
         return;
    }

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

    QList<lamp_info_out_t*> lamp_info_list1;
    lamp_info_list1 = color_filter_model_run(imgfrompath);
    print_model_out_info(lamp_info_list1,color_filter_model_info.color_num);

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
    QList<lamp_info_out_t*> lamp_info_list2;
    lamp_info_list2 = color_filter_model_run(imgfrompath2);
    print_model_out_info(lamp_info_list1,color_filter_model_info.color_num);

    //比较两张图的特征向量  不同按钮颜色数量比较
    bool is_same_color = true;
    for(int i=0;i<lamp_info_list1.size();i++) {
        qDebug() << "i:::" << i;
        if(!strcmp(lamp_info_list1.at(i)->lamp_color,lamp_info_list2.at(i)->lamp_color)) {
            //qDebug() << "lamp_info_list1.at(i)->lamp_color_num" << lamp_info_list1.at(i)->lamp_color_num;
            //qDebug() << "lamp_info_list2.at(i)->lamp_color_num" << lamp_info_list2.at(i)->lamp_color_num;

            if(lamp_info_list1.at(i)->lamp_color_num != lamp_info_list2.at(i)->lamp_color_num) {
                is_same_color = false;
                break;
            }
        }
    }
    if(is_same_color == false) {
        QMessageBox::information(NULL, "Warning!!", "两张图像颜色信息不一样!!");
    }
    else {
        QMessageBox::information(NULL, "Warning!!", "两张图像颜色信息一样!!");
        is_same_color = true;

        //比较相同按钮位置的颜色信息
        bool is_same_location = false;
        int  count_color = 0;
        for(int i=0;i<lamp_info_list1.size();i++) {
            int count_button = 0;
             for(int j=0;j<lamp_info_list1.at(i)->button_info_List.size();j++)//比较相同按钮位置的颜色信息
             {
                 for(int k=0;k<lamp_info_list2.at(i)->button_info_List.size();k++) {
                     if(lamp_info_list1.at(i)->button_info_List.at(j)->location_index == lamp_info_list2.at(i)->button_info_List.at(k)->location_index) {
                        count_button++;
                        break;
                     }
                 }
             }
             //按钮位置比较结束 继续查看下一个颜色的按钮
             if(count_button == lamp_info_list1.at(i)->button_info_List.size()) {
                 count_color++;
             }
             else {
                 is_same_location = false;
                 QMessageBox::information(NULL, "Warning!!", "两张图像按钮位置信息不一样!!");
             }
       }

       if(count_color == lamp_info_list1.size()) {
           is_same_location = true;
            QMessageBox::information(NULL, "Warning!!", "两张图像按钮位置信息一样!!");
       }
    }

    //QList 释放内存 元素为指针类型
    foreach(lamp_info_out_t *lamp_info_out,lamp_info_list1)
    {
        if(lamp_info_out)
        {
            lamp_info_list1.removeOne(lamp_info_out);
            delete lamp_info_out;
            lamp_info_out = nullptr;
        }
    }

    foreach(lamp_info_out_t *lamp_info_out,lamp_info_list2)
    {
        if(lamp_info_out)
        {
            lamp_info_list2.removeOne(lamp_info_out);
            delete lamp_info_out;
            lamp_info_out = nullptr;
        }
    }
}


void ui_algorithm_colorFilter::on_ui_colorFilter_model_sync_client_btn_clicked()
{
    if(color_filter_model_info.algorithmInfo.algorithm_train_res == 0) {
        QMessageBox::information(NULL, "Warning!!", "没有从数据库取得数据，请先点击\"算法训练\"!!");
        return;
    }
    int client_index = 0;
    for(int i=0;i<clientList.size();i++) {
        if(!strcmp((char*)clientList.at(i)->client_info.client_uuid,(char*)color_filter_model_info.algorithmInfo.clinet_uuid)) {
            client_index = i;
        }
    }
    //组织成json字符串  发送给对应的客户端
    cJSON* root = cJSON_CreateObject();
    cJSON_AddItemToObject(root,"cmd",cJSON_CreateString(CMD_SERVER_TRAIN_COLOR_ALGORITHM));
    cJSON_AddItemToObject(root,"server_uuid",cJSON_CreateString(server_info.server_uuid));
    cJSON_AddItemToObject(root,"client_uuid",cJSON_CreateString((char*)color_filter_model_info.algorithmInfo.clinet_uuid));
    cJSON_AddItemToObject(root,"camera_name",cJSON_CreateString((char*)color_filter_model_info.algorithmInfo.camera_device_name));
    cJSON_AddItemToObject(root,"is_resize",cJSON_CreateNumber(color_filter_model_info.is_resize));
    cJSON_AddItemToObject(root,"imgorg_width",cJSON_CreateNumber(color_filter_model_info.imgcut_info.org_width));
    cJSON_AddItemToObject(root,"imgorg_height",cJSON_CreateNumber(color_filter_model_info.imgcut_info.org_height));
    cJSON_AddItemToObject(root,"imgcut_info_x",cJSON_CreateNumber(color_filter_model_info.imgcut_info.x));
    cJSON_AddItemToObject(root,"imgcut_info_y",cJSON_CreateNumber(color_filter_model_info.imgcut_info.y));
    cJSON_AddItemToObject(root,"imgcut_info_width",cJSON_CreateNumber(color_filter_model_info.imgcut_info.width));
    cJSON_AddItemToObject(root,"imgcut_info_height",cJSON_CreateNumber(color_filter_model_info.imgcut_info.height));
    cJSON_AddItemToObject(root,"color_num",cJSON_CreateNumber(color_filter_model_info.color_num));
    cJSON_AddItemToObject(root,"area_min",cJSON_CreateNumber(color_filter_model_info.area_info.area_min));
    cJSON_AddItemToObject(root,"area_max",cJSON_CreateNumber(color_filter_model_info.area_info.area_max));
    cJSON_AddItemToObject(root,"time_offset",cJSON_CreateNumber(color_filter_model_info.time_offset));

    cJSON* color_arr = cJSON_CreateArray();

    for(int i=0;i<color_filter_model_info.color_num;i++) {
        cJSON* arr_item = cJSON_CreateObject();
        cJSON_AddItemToObject(arr_item,"color",cJSON_CreateString(color_filter_model_info.hsv_color_infoList.at(i)->lamp_color));
        cJSON_AddItemToObject(arr_item,"hmin",cJSON_CreateNumber(color_filter_model_info.hsv_color_infoList.at(i)->hmin));
        cJSON_AddItemToObject(arr_item,"smin",cJSON_CreateNumber(color_filter_model_info.hsv_color_infoList.at(i)->smin));
        cJSON_AddItemToObject(arr_item,"vmin",cJSON_CreateNumber(color_filter_model_info.hsv_color_infoList.at(i)->vmin));
        cJSON_AddItemToObject(arr_item,"hmax",cJSON_CreateNumber(color_filter_model_info.hsv_color_infoList.at(i)->hmax));
        cJSON_AddItemToObject(arr_item,"smax",cJSON_CreateNumber(color_filter_model_info.hsv_color_infoList.at(i)->smax));
        cJSON_AddItemToObject(arr_item,"vmax",cJSON_CreateNumber(color_filter_model_info.hsv_color_infoList.at(i)->vmax));
        cJSON_AddItemToArray(color_arr, arr_item);
    }
    cJSON_AddItemToObject(root, "hsv_info", color_arr);

    char* json_paras_info = cJSON_Print(root);
    qDebug() << "json::" << QString(json_paras_info);

    TcpSocketList.at(client_index)->TcpClientSocket->write(json_paras_info);
    QMessageBox::information(NULL, "About!!", "发送到客户端成功!!");
}

