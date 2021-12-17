#include "ui_algorithm_setting.h"
#include "ui_ui_algorithm_setting.h"

#include <fstream>
#include <iostream>
#include <QFile>
#include <QDebug>
#include <QMessageBox>
#include <QStringList>
#include <QList>
#include <QFileDialog>
#include <QSettings>
#include "sql/sqlbase.h"

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/opencv.hpp>
using namespace cv;
using namespace std;

extern dbConnect_info_t m_dbconnect_info; //数据库信息
extern QList<client_all_info_t*> clientList;//管理客户端的列表
extern taskThread_Result_info_t taskResult_info;//管理服务端处理客户端消息的业务逻辑结果

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


//将Mat类型转换为 QImage类型
QImage cvMat2QImage(const Mat &mat)
{
    // 8-bits unsigned, NO. OF CHANNELS = 1
    if(mat.type() == CV_8UC1)
    {
        QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
        // Set the color table (used to translate colour indexes to qRgb values)
        image.setColorCount(256);
        for(int i = 0; i < 256; i++)
        {
            image.setColor(i, qRgb(i, i, i));
        }
        // Copy input Mat
        uchar *pSrc = mat.data;
        for(int row = 0; row < mat.rows; row ++)
        {
            uchar *pDest = image.scanLine(row);
            memcpy(pDest, pSrc, mat.cols);
            pSrc += mat.step;
        }
        return image;
    }
    // 8-bits unsigned, NO. OF CHANNELS = 3
    else if(mat.type() == CV_8UC3)
    {
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return image.rgbSwapped();
    }
    else if(mat.type() == CV_8UC4)
    {
        qDebug() << "CV_8UC4";
        // Copy input Mat
        const uchar *pSrc = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
        return image.copy();
    }
    else
    {
        qDebug() << "ERROR: Mat could not be converted to QImage.";
        return QImage();
    }
}

ui_algorithm_setting::ui_algorithm_setting(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ui_algorithm_setting)
{
    ui->setupUi(this);
    algorithm_colorFilter = NULL;
    algorithm_brightnessfilter = NULL;
    m_database_flag = false;
    m_client_uuid = "";
    m_camera_id = 0;
    m_algorithm_id = 0;
    m_is_resize = false;
    m_img_org_width = 0;
    m_img_org_height = 0;
    m_org_path = "";
    m_save_path = "";

    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(slot_reshow_widget()));
    timer->start(200);


    //读取算法配置文件信息
    QSettings scfg(":/resource/config/algorithm_version.ini",QSettings::IniFormat);
    scfg.beginGroup("algorithm_model_num");
    int num = scfg.value("algorithm1_num").toInt();
    scfg.endGroup();
    qDebug() << "algorithm1 num:" << num <<Qt::endl;

    //增加到客户端信息到列表中
    for(int i=0;i<clientList.size();i++) {//客户端索引
        //增加客户端
        ui->ui_algorithm_setting_client_cmbx->addItem(QString((char*)clientList.at(i)->client_info.client_name));
        //qDebug() <<"client_all_info["<< i << "].client_info.camera_num:" << clientList.at(i)->camera_num << Qt::endl;
        //增加相机
        ui->ui_algorithm_setting_camera_cmbx->clear();
        for(int j=0;j<clientList.at(i)->camera_num;j++) {//摄像机索引
            ui->ui_algorithm_setting_camera_cmbx->addItem(QString((char*)clientList.at(0)->camera_device_info[j].camera_device_name));
      }
    }

    //增加到列表中 算法信息
    for(int i = 1;i <= num;i++) {
        QString group = QString("%1%2").arg("algorithm_type").arg(i);
        //qDebug() << "group:" << group << Qt::endl;
        scfg.beginGroup(group);
        QString algorithm_name = scfg.value("algorithm_name").toString();
        QString algorithm_version = scfg.value("algorithm_version").toString();
        //qDebug() << "algorithm_name:" << algorithm_name  << "algorithm_version:" << algorithm_version <<Qt::endl;
        ui->ui_algorithm_setting_algorithm_type_cmbx->addItem(algorithm_name);
        ui->ui_algorithm_setting_algorithm_version_cmbx->addItem(algorithm_version);
        scfg.endGroup();

        //同步算法信息到数据库中
        sqlBase sqlbase;
        bool is_algorithminfo_in_database =sqlbase.is_algorithminfo_in_database(m_dbconnect_info.dbconn,algorithm_name);
        if(is_algorithminfo_in_database == false ) {
            QString sql = "SELECT MAX(algorithm_info.algorithm_id) FROM algorithm_info;";
            int algorithm_id = sqlbase.get_maxid(m_dbconnect_info.dbconn,sql) + 1;
            int ret = sqlbase.insert_to_algorithm_info(m_dbconnect_info.dbconn,algorithm_id,algorithm_name,algorithm_version);
            if(ret > 0) {
                qDebug()<<"algorithm info数据库插入成功!"<<Qt::endl;
                QMessageBox::information(0,"提示", "algorithm info数据库插入成功!");
            }
            else {
                 qDebug()<<"algorithm info数据库插入失败!"<<Qt::endl;
                  QMessageBox::information(0,"提示", "algorithm info数据库插入失败!");
                return;
            }
        }
    }

}

int ui_algorithm_setting::check_model_in_database(QString client_name,QString camera_name,QString algorithm_name,QString algorithm_version,int index)
{
    sqlBase sqlbase;
    m_client_uuid   = get_client_uuid(client_name);
    m_camera_id     = get_camera_id(camera_name,m_client_uuid);
    m_algorithm_id  = get_algorithm_id(algorithm_name,algorithm_version);

    bool database_flag = false;
    database_flag = sqlbase.is_lamp_template_no_in_database(m_dbconnect_info.dbconn,
                                                                           m_client_uuid,
                                                                           m_camera_id,
                                                                           m_algorithm_id,
                                                                           index);
    return database_flag;
}

void ui_algorithm_setting::slot_reshow_widget()
{
     if(taskResult_info.client_cancel_flag == true) {
        qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
        this->close();
    }
}

ui_algorithm_setting::~ui_algorithm_setting()
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;

    delete ui;
    if(algorithm_colorFilter) {
        delete algorithm_colorFilter;
        algorithm_colorFilter = NULL;
    }

    if(algorithm_brightnessfilter) {
        delete algorithm_brightnessfilter;
        algorithm_brightnessfilter = NULL;
    }
    delete timer;
    timer = NULL;
}




static Mat imgTemplate;//模板图图像
void ui_algorithm_setting::on_ui_algorithm_setting_template_open_btn_clicked()
{
    Mat imgfrompath;
    //打开本地图片 从客户端抓拍过来的图片
    QString strPath = QFileDialog::getOpenFileName(this, "打开图片", tr("c:\\"), tr("Images (*.png *.xpm *.jpg)"));
    //qDebug("strPath = [%s]", strPath.toUtf8().data());

    QFile file(strPath);
    if(!file.open(QFile::ReadOnly))
    {
        QMessageBox::information(NULL, "Warning!!", "图片未选择!!");
        return;
    }else{
        m_org_path = strPath;
        QByteArray ba = file.readAll();
        imgfrompath = imdecode(vector<char>(ba.begin(), ba.end()), 1);
    }

    //qDebug() << "image.width::" << imgfrompath.cols << Qt::endl;//列宽  图像的宽
    //qDebug() << "image.height::"<< imgfrompath.rows << Qt::endl;//行高  图像的高
    m_img_org_width     = imgfrompath.cols;
    m_img_org_height    = imgfrompath.rows;

    if(imgfrompath.cols > 800 ) {//图像的宽过大 缩放显示
        m_is_resize = true;
        cv::resize(imgfrompath,imgTemplate,Size(),0.5,0.5);
        QImage image = cvMat2QImage(imgTemplate);
        QPixmap map = QPixmap::fromImage(image);
        ui->ui_algorithm_setting_template_show_lab->setPixmap(map);
    }
    else
    {
        m_is_resize = false;
        imgfrompath.copyTo(imgTemplate);

        QPixmap map(strPath);
        ui->ui_algorithm_setting_template_show_lab->setMinimumSize(ui->ui_algorithm_setting_template_show_lab->size());
        map.scaled(ui->ui_algorithm_setting_template_show_lab->size(), Qt::KeepAspectRatio);
        ui->ui_algorithm_setting_template_show_lab->setScaledContents(true);
        ui->ui_algorithm_setting_template_show_lab->setPixmap(map);
    }
}


static Mat imgOrg;         //源图像
static Mat imgClone;       //拷贝图像 保存原图
static Mat imgTmp;         //临时图像
static Mat imgDst;         //通过鼠标事件截的图
static roiPoint_t roiDst;  //截图坐标
//鼠标回调函数
void ui_algorithm_setting::on_mouse(int event, int x, int y, int flags, void* userdata)
{
    //定义坐标点
    static Point pre_pt(-1,-1);
    static Point cur_pt(-1,-1);

    //截图框的颜色
    Scalar color(12, 255, 200);//BGR三通道

    char temp[16];
    if (event == EVENT_LBUTTONDOWN)//按下左键
    {
        //imgClone中保留原始图像的内容  每次按下左键恢复图像到原始图像
        imgClone.copyTo(imgOrg);

        //获取截图框起始坐标
        pre_pt = Point(x, y);
        roiDst.PointStart.x = pre_pt.x;
        roiDst.PointStart.y = pre_pt.y;

        //显示图像
        imshow("imgOrg", imgOrg);
        imgOrg.copyTo(imgTmp);
    }
    else if (event == EVENT_MOUSEMOVE && !(flags & EVENT_FLAG_LBUTTON))//鼠标移动时显示坐标
    {
        imgTmp.copyTo(imgOrg);
        sprintf(temp, "(%d,%d)", x, y);
        cur_pt = Point(x, y);
        putText(imgOrg, temp, cur_pt, FONT_HERSHEY_SIMPLEX, 0.4, color, 1, LINE_8, false);
        imshow("imgOrg", imgOrg);
    }
    else if (event == EVENT_MOUSEMOVE && (flags & EVENT_FLAG_LBUTTON))//左键拖曳
    {
        //获取截图框结束坐标
        imgTmp.copyTo(imgOrg);
        sprintf(temp, "(%d,%d)", x, y);
        cur_pt = Point(x, y);
        roiDst.PointEnd.x = cur_pt.x;
        roiDst.PointEnd.x = cur_pt.y;

        //判断得到截图框截图起始坐标
        if (pre_pt.x < cur_pt.x && pre_pt.y < cur_pt.y)
        {
            roiDst.PointRoi.x = pre_pt.x;
            roiDst.PointRoi.y = pre_pt.y;
        }
        else if (pre_pt.x > cur_pt.x && pre_pt.y < cur_pt.y)
        {
            roiDst.PointRoi.x = cur_pt.x;
            roiDst.PointRoi.y = pre_pt.y;
        }
        else if (pre_pt.x > cur_pt.x && pre_pt.y > cur_pt.y)
        {
            roiDst.PointRoi.x = cur_pt.x;
            roiDst.PointRoi.y = cur_pt.y;
        }
        else if (pre_pt.x<cur_pt.x && pre_pt.y>cur_pt.y)
        {
            roiDst.PointRoi.x = pre_pt.x;
            roiDst.PointRoi.y = cur_pt.y;
        }

        //计算得到截图框的宽和高
        roiDst.width  = abs(pre_pt.x - cur_pt.x);
        roiDst.height = abs(pre_pt.y - cur_pt.y);
        roiDst.ButtonAread = roiDst.width * roiDst.height;
        qDebug() << "长:" << roiDst.width << "宽:" << roiDst.height << "小矩形面积:" << roiDst.ButtonAread << Qt::endl;

        //cout << "x1 = " << pre_pt.x << "y1 = " << pre_pt.y << endl;
        //cout << "x2 = " << cur_pt.x << "y2 = " << cur_pt.y << endl;
        //cout << "x2-x1 = " <<  cur_pt.x - pre_pt.x << "y2 - y1= " << cur_pt.y - pre_pt.y << endl;

        Rect rect(pre_pt.x,pre_pt.y, roiDst.width, roiDst.height);
        putText(imgOrg, temp, cur_pt, FONT_HERSHEY_SIMPLEX, 0.4, color, 1,LINE_8,false);
        //rectangle(imgOrg, rect,color, 1, LINE_8, 0);//只能单一方向绘制矩形
        rectangle(imgOrg, pre_pt, cur_pt, color, 1, LINE_8, 0);//可以上下左右绘制矩形
        imshow("imgOrg", imgOrg);
    }
    else if (event == EVENT_LBUTTONUP)//放开左键
    {
        if(roiDst.PointRoi.x ==0 ) {
             QMessageBox::information(NULL, "Warning!!", "截图错误!!");
            return;
        }

        imgTmp.copyTo(imgOrg);

        sprintf(temp, "(%d,%d)", x, y);
        cur_pt = Point(x, y);
        putText(imgOrg, temp, cur_pt, FONT_HERSHEY_SIMPLEX, 0.4, color, 1, LINE_8, false);
        circle(imgOrg, cur_pt, 1, Scalar(255, 0, 0), FILLED, LINE_8, 0);
        Rect rect(pre_pt.x, pre_pt.y, cur_pt.x - pre_pt.x, cur_pt.y - pre_pt.y);
        rectangle(imgOrg, rect, color, 1, LINE_8, 0);
        imshow("imgOrg", imgOrg);

        //得到截图
        int width = abs(pre_pt.x - cur_pt.x);
        int height = abs(pre_pt.y - cur_pt.y);


        if (width == 0 || height == 0)
        {
            destroyWindow("dst");
            return;
        }

        //截图并显示
        Rect rectRoi(roiDst.PointRoi.x, roiDst.PointRoi.y, roiDst.width, roiDst.height);
        imgDst = imgClone(rectRoi);
        imshow("dst",imgDst);

        //将截图保存到指定路径
        QString fileName(tr("imgcut.jpg")) ;
        QString dir = QFileDialog::getSaveFileName(this,tr("另存为"),fileName,"Text files (*.jpg)");
        if (!dir.isEmpty())
        {
          qDebug() << dir;
          QByteArray ba = dir.toLatin1();
          m_save_path = dir;
          imwrite(ba.data(), imgDst);
        }
        else {
             QMessageBox::information(NULL, "Warning!!", "路径非法!!");
        }

        if (imgDst.empty() == false) {
            qDebug() << "截图起始坐标:(" << roiDst.PointRoi.x << "," << roiDst.PointRoi.y << " ） " << Qt::endl;
            qDebug() << "截图长:" << roiDst.width << "截图宽：" << roiDst.height << Qt::endl;
            setimgPoint(roiDst.PointRoi.x,roiDst.PointRoi.y, roiDst.width, roiDst.height,dir);
            destroyAllWindows();
        }
    }
}

void ui_algorithm_setting::setimgPoint(double x,double y,double w,double h,QString& dir)
{
  ui->ui_algorithm_setting_template_cut_x_lab->setText(QString::number(x));
  ui->ui_algorithm_setting_template_cut_y_lab->setText(QString::number(y));
  ui->ui_algorithm_setting_template_cut_width_lab->setText(QString::number(w)) ;
  ui->ui_algorithm_setting_template_cut_height_lab->setText(QString::number(h));

  if(m_is_resize == true) {
      QPixmap map(dir);
      ui->ui_algorithm_setting_template_cut_show_lab->setPixmap(map);
  }
  else {
      QPixmap map(dir);
      ui->ui_algorithm_setting_template_cut_show_lab->setMinimumSize(ui->ui_algorithm_setting_template_cut_show_lab->size());
      map.scaled(ui->ui_algorithm_setting_template_cut_show_lab->size(), Qt::KeepAspectRatio);
      ui->ui_algorithm_setting_template_cut_show_lab->setScaledContents(true);
      ui->ui_algorithm_setting_template_cut_show_lab->setPixmap(map);
  }

}


void mouseWrapper( int event, int x, int y, int flags, void* param )
{
    ui_algorithm_setting* mainWn = (ui_algorithm_setting *)(param);
    mainWn->on_mouse(event,x,y,flags,param);
}

//得到截图图像 和截图坐标
void ui_algorithm_setting::on_ui_algorithm_setting_template_cut_btn_clicked()
{
    if(imgTemplate.empty()) {
        QMessageBox::information(NULL, "Warning!!", "模板图未选择!!");
        return;
    }

    //复制图像
    imgClone = imgTemplate.clone();
    imgTmp = imgTemplate.clone();

     namedWindow("imgOrg", (640, 200)); // Create Window
     imshow("imgOrg", imgTemplate);

     //鼠标回调函数
    setMouseCallback("imgOrg",mouseWrapper,this);
    waitKey(0);
}


void ui_algorithm_setting::on_ui_algorithm_setting_algorithm_test_btn_clicked()
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
     imgcut_info_t  imgcut_info;
     QByteArray ba_temp;
     sqlBase sqlbase;
     memset(&imgcut_info,0,sizeof(imgcut_info_t));

     bool is_templateinfo_in_database  = false;
     if(ui->ui_algorithm_setting_algorithm_type_cmbx->currentIndex() == COLOR_FILTER_TEMPLATE)
         is_templateinfo_in_database =   check_model_in_database(ui->ui_algorithm_setting_client_cmbx->currentText(),
                                                                 ui->ui_algorithm_setting_camera_cmbx->currentText(),
                                                                 ui->ui_algorithm_setting_algorithm_type_cmbx->currentText(),
                                                                 ui->ui_algorithm_setting_algorithm_version_cmbx->currentText(),
                                                                 0);
     else if(ui->ui_algorithm_setting_algorithm_type_cmbx->currentIndex() == BRIGHTNESS_FILTER_TEMPLATE)
         is_templateinfo_in_database =   check_model_in_database(ui->ui_algorithm_setting_client_cmbx->currentText(),
                                                                 ui->ui_algorithm_setting_camera_cmbx->currentText(),
                                                                 ui->ui_algorithm_setting_algorithm_type_cmbx->currentText(),
                                                                 ui->ui_algorithm_setting_algorithm_version_cmbx->currentText(),
                                                                 1);

     ba_temp = m_client_uuid.toLatin1();
     memcpy(imgcut_info.client_uuid,ba_temp.data(),sizeof(imgcut_info.client_uuid));
     imgcut_info.camera_id = m_camera_id;
     imgcut_info.algorithm_id = m_algorithm_id;

     //设置客户端参数 算法参数
     ba_temp =  ui->ui_algorithm_setting_client_cmbx->currentText().toLatin1();
     memcpy(imgcut_info.client_name,ba_temp.data(),sizeof(imgcut_info.client_name));
     ba_temp =  ui->ui_algorithm_setting_camera_cmbx->currentText().toLatin1();
     memcpy(imgcut_info.camera_device_name,ba_temp.data(),sizeof(imgcut_info.camera_device_name));
     ba_temp =  ui->ui_algorithm_setting_algorithm_type_cmbx->currentText().toLatin1();
     memcpy(imgcut_info.algorithm_name,ba_temp.data(),sizeof(imgcut_info.algorithm_name));
     ba_temp =  ui->ui_algorithm_setting_algorithm_version_cmbx->currentText().toLatin1();
     memcpy(imgcut_info.algorithm_version,ba_temp.data(),sizeof(imgcut_info.algorithm_version));

    //判断该客户端所对应的相机拍摄的指示灯图片是否训练

    if(is_templateinfo_in_database == false) {
        imgcut_info.database_flag = false;
        if(imgTemplate.empty()) {
            QMessageBox::information(NULL, "Warning!!", "模板图未选择!!");
            return;
        }

        if(imgDst.empty()) {
            QMessageBox::information(NULL, "Warning!!", "截图未选择!!");
            return;
        }

        imgcut_info.imgcut_parms.org_width  = m_img_org_width;
        imgcut_info.imgcut_parms.org_height = m_img_org_height;
        imgcut_info.imgcut_parms.x      = roiDst.PointRoi.x;
        imgcut_info.imgcut_parms.y      = roiDst.PointRoi.y;
        imgcut_info.imgcut_parms.width  = roiDst.width;
        imgcut_info.imgcut_parms.height = roiDst.height;
        imgcut_info.is_resize = m_is_resize;

        ba_temp = m_org_path.toLatin1();
        memcpy(imgcut_info.org_path,ba_temp.data(),sizeof(imgcut_info.org_path));

        ba_temp = m_save_path.toLatin1();
        memcpy(imgcut_info.save_path,ba_temp.data(),sizeof(imgcut_info.save_path));
    }
    else {

        QSqlQuery query(m_dbconnect_info.dbconn);
        QString org_path,cut_path;
        if(ui->ui_algorithm_setting_algorithm_type_cmbx->currentIndex() == COLOR_FILTER_TEMPLATE)
            query.prepare("SELECT lamp_color_template_info.template_image_org_path,lamp_color_template_info.template_image_cut_path \
                          FROM lamp_color_template_info \
                          WHERE lamp_color_template_info.client_uuid = ? \
                          AND lamp_color_template_info.camera_id = ? \
                          AND lamp_color_template_info.algorithm_id = ?;");
        else if(ui->ui_algorithm_setting_algorithm_type_cmbx->currentIndex() == BRIGHTNESS_FILTER_TEMPLATE)
            query.prepare("SELECT brightness_template_info.template_image_org_path,brightness_template_info.template_image_cut_path \
                          FROM brightness_template_info \
                          WHERE brightness_template_info.client_uuid = ? \
                          AND brightness_template_info.camera_id = ? \
                          AND brightness_template_info.algorithm_id = ?;");

        query.addBindValue(m_client_uuid);
        query.addBindValue(m_camera_id);
        query.addBindValue(m_algorithm_id);

        if(query.exec()) {
            while(query.next()) {
                org_path = query.value(0).toString();
                cut_path = query.value(1).toString();
            }
        }//end while

        //加载图像
        QPixmap org_img_map(org_path);
        QPixmap cut_img_map(cut_path);
        ui->ui_algorithm_setting_template_show_lab->setMinimumSize(ui->ui_algorithm_setting_template_show_lab->size());
        ui->ui_algorithm_setting_template_cut_show_lab->setMinimumSize(ui->ui_algorithm_setting_template_cut_show_lab->size());
        org_img_map.scaled(ui->ui_algorithm_setting_template_show_lab->size(), Qt::KeepAspectRatio);
        cut_img_map.scaled(ui->ui_algorithm_setting_template_cut_show_lab->size(), Qt::KeepAspectRatio);
        ui->ui_algorithm_setting_template_show_lab->setScaledContents(true);
         ui->ui_algorithm_setting_template_cut_show_lab->setScaledContents(true);
        ui->ui_algorithm_setting_template_show_lab->setPixmap(org_img_map);
        ui->ui_algorithm_setting_template_cut_show_lab->setPixmap(cut_img_map);

        imgcut_info.database_flag = true;
        QMessageBox::information(NULL, "About", "该客户端模型已经生成，如果需要重新训练，请点击【清空算法模型】!!");
        imgcut_info.imgcut_parms.org_width  = 0;
        imgcut_info.imgcut_parms.org_height = 0;
        imgcut_info.imgcut_parms.x      = 0;
        imgcut_info.imgcut_parms.y      = 0;
        imgcut_info.imgcut_parms.width  = 0;
        imgcut_info.imgcut_parms.height = 0;
        imgcut_info.is_resize = 0;

        strcpy((char*)imgcut_info.save_path,"");
    }

    if(ui->ui_algorithm_setting_algorithm_type_cmbx->currentIndex() == 0) {
        algorithm_colorFilter = new ui_algorithm_colorFilter();
        connect(this,SIGNAL(sig_sendto_algorithm_colorFilte(imgcut_info_t*)),algorithm_colorFilter,SLOT(slot_sendto_algorithm_colorFilte(imgcut_info_t*)));
        emit sig_sendto_algorithm_colorFilte(&imgcut_info);
        algorithm_colorFilter->show();
    }
    else if(ui->ui_algorithm_setting_algorithm_type_cmbx->currentIndex() == 1) {
        algorithm_brightnessfilter = new ui_algorithm_brightnessfilter();
        connect(this,SIGNAL(sig_sendto_algorithm_brightnessfilter(imgcut_info_t*)),algorithm_brightnessfilter,SLOT(slot_sendto_algorithm_brightnessfilter(imgcut_info_t*)));
        emit sig_sendto_algorithm_brightnessfilter(&imgcut_info);
        algorithm_brightnessfilter->show();
    }
}


void ui_algorithm_setting::on_ui_algorithm_setting_algorithm_type_cmbx_currentIndexChanged(int index)
{
        ui->ui_algorithm_setting_algorithm_version_cmbx->setCurrentIndex(index);
}

void ui_algorithm_setting::on_ui_algorithm_setting_algorithm_version_cmbx_currentIndexChanged(int index)
{
     ui->ui_algorithm_setting_algorithm_type_cmbx->setCurrentIndex(index);
}

void ui_algorithm_setting::on_ui_algorithm_setting_client_cmbx_currentTextChanged(const QString &arg1)
{
    if(clientList.size() >= 2 && ui->ui_algorithm_setting_client_cmbx->count()==clientList.size()) { //有两个客户端及以上
        for(int i=0;i<clientList.size();i++) {//客户端索引
            if(arg1 == QString((char*)clientList.at(i)->client_info.client_name)) {
                ui->ui_algorithm_setting_camera_cmbx->clear();
                for(int j=0;j<clientList.at(i)->camera_num;j++) {//相机索引
                    ui->ui_algorithm_setting_camera_cmbx->addItem(QString((char*)clientList.at(i)->camera_device_info[j].camera_device_name));
                }//end camera index
            }//end if client name
        }//end client index
    }
}


void ui_algorithm_setting::on_ui_algorithm_setting_client_cmbx_currentIndexChanged(int index)
{
     ui->ui_algorithm_setting_client_cmbx->setCurrentIndex(index);
}


void ui_algorithm_setting::on_ui_algorithm_setting_camera_cmbx_currentIndexChanged(int index)
{
    ui->ui_algorithm_setting_camera_cmbx->setCurrentIndex(index);
}


void ui_algorithm_setting::on_ui_algorithm_setting_algorithm_clear_btn_clicked()
{
    bool is_templateinfo_in_database  = false;
    if(ui->ui_algorithm_setting_algorithm_type_cmbx->currentIndex() == COLOR_FILTER_TEMPLATE)
        is_templateinfo_in_database =   check_model_in_database(ui->ui_algorithm_setting_client_cmbx->currentText(),
                                                                ui->ui_algorithm_setting_camera_cmbx->currentText(),
                                                                ui->ui_algorithm_setting_algorithm_type_cmbx->currentText(),
                                                                ui->ui_algorithm_setting_algorithm_version_cmbx->currentText(),
                                                                0);
    else if(ui->ui_algorithm_setting_algorithm_type_cmbx->currentIndex() == BRIGHTNESS_FILTER_TEMPLATE)
        is_templateinfo_in_database =   check_model_in_database(ui->ui_algorithm_setting_client_cmbx->currentText(),
                                                                ui->ui_algorithm_setting_camera_cmbx->currentText(),
                                                                ui->ui_algorithm_setting_algorithm_type_cmbx->currentText(),
                                                                ui->ui_algorithm_setting_algorithm_version_cmbx->currentText(),
                                                                1);

    if(is_templateinfo_in_database == false) {
         QMessageBox::information(NULL, "About", "该客户端模型没有生成,无法清除!!!");
         return;
    }

    bool ret = false;
    QSqlQuery query(m_dbconnect_info.dbconn);
    query.exec("BEGIN");

    if(ui->ui_algorithm_setting_algorithm_type_cmbx->currentIndex() == COLOR_FILTER_TEMPLATE)
        query.prepare("DELETE from lamp_color_template_info;");
    else if(ui->ui_algorithm_setting_algorithm_type_cmbx->currentIndex() == BRIGHTNESS_FILTER_TEMPLATE)
        query.prepare("DELETE from brightness_template_info;");

    if(query.exec()) {
        query.exec("COMMIT");
        ret = true;
    }//end while
    else
        ret = false;

    if(ret == true)
        QMessageBox::information(NULL, "About", "清空算法模型成功!!");
    else
        QMessageBox::information(NULL, "About", "清空算法模型失败!!");

    ui->ui_algorithm_setting_template_show_lab->clear();
    ui->ui_algorithm_setting_template_cut_show_lab->clear();
    imgTemplate.release();

    //使用信号槽传递清楚相关模型信息
    connect(this,SIGNAL(sig_clear_model_info_color()),algorithm_colorFilter,SLOT(slot_clear_model_info()));
    connect(this,SIGNAL(sig_clear_model_info_brightness()),algorithm_brightnessfilter,SLOT(slot_clear_model_info()));

    if(ui->ui_algorithm_setting_algorithm_type_cmbx->currentIndex() == COLOR_FILTER_TEMPLATE)
        emit sig_clear_model_info_color();
    else if(ui->ui_algorithm_setting_algorithm_type_cmbx->currentIndex() == BRIGHTNESS_FILTER_TEMPLATE)
        emit sig_clear_model_info_brightness();
}

