#include "mytask.h"
#include <QByteArray>
#include <QMessageBox>
#include "base/eventbase.h"
#include <QDateTime>
#include <QPixmap>
#include <QDir>

#include <opencv2/highgui.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/core.hpp>

using namespace std;
using namespace cv;

extern QList<client_net_info_t*>TcpSocketList;      //管理客户端socket ip port
extern QList<client_all_info_t*> clientList;         //管理客户端的列表
extern taskThread_Result_info_t taskResult_info;    //服务端处理客户端消息的业务逻辑结果
extern dbConnect_info_t m_dbconnect_info;           //数据库信息
extern cap_record_info_t cap_record_info;//抓拍记录 用于数据库

static int number_frame[MAXVIDEONUM] = {0};         //标记相机持续过来的图像流
static int number_imgcap[MAXVIDEONUM] = {0};        //标记相机按照服务端指令过来的抓拍图像

extern QString username_login;                      //系统登录用户名
extern QString userid_login;                        //系统登录用户ID

myTask::myTask()
{

}

void myTask::run()
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
    qDebug() << "当前客户端的索引:" << taskResult_info.current_client;

    //业务代码  处理不同客户端过来的数据
    QByteArray ba = taskResult_info.string_RecivedData.toLatin1();
    //qDebug() << "the size of ba is: " << ba.length();
    cJSON* json = cJSON_Parse(ba.data());
    dataAnalysis(json,taskResult_info.current_client);
    emit Result(&taskResult_info);
}

void myTask::dataAnalysis(cJSON *json, int client_index)
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
    cJSON* item_cmd = cJSON_GetObjectItem(json,"cmd");
    if(!item_cmd)
        return;

    //客户端连接事件
    if(!strcmp(item_cmd->valuestring,CMD_CLIENT_CONN)) {
        qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
        taskResult_info.cmd = CMD_CLIENT_CONN;

        //解析json数据 获取客户端相关信息
        int ret = get_client_all_info(json,client_index);
        if(ret < 0)
            return;

        //更新数据库  客户端信息表  房间信息表  指示灯信息表 相机信息表 系统日志表
        set_client_all_info_database(&m_dbconnect_info,clientList.at(client_index));
    }

    //客户端获取视频 连续图像事件
    if(!strcmp(item_cmd->valuestring,CMD_CLIENT_GET_VIDEO)) {
        qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
        taskResult_info.cmd = CMD_CLIENT_GET_VIDEO;

        //解析json数据 得到一个客户端的所有相机采集的图
        get_video_path(json,client_index);
    }

    //服务端图片抓拍回复事件 来自客户端
    if(!strcmp(item_cmd->valuestring,CMD_CLIENT_SERVER_IMG_CAP_ACK)) {
        qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
        taskResult_info.cmd = CMD_CLIENT_SERVER_IMG_CAP_ACK;

        //解析json数据  得到一个客户端某一个相机采集到的图片
        get_imgcap(json,client_index);
    }
}

int myTask::get_client_all_info(cJSON *json, int client_index)
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;
    cJSON* item_client_info = cJSON_GetObjectItem(json,"client_info");
    cJSON* item_client_client_uuid = cJSON_GetObjectItem(item_client_info,"client_uuid");
    if(clientList.size() > 0) {
        for(int i=0;i<clientList.size();i++) {
            if(!strcmp((char*)clientList.at(i)->client_info.client_uuid,item_client_client_uuid->valuestring)) {
                qDebug() << "现有客户端已经存在，请检查客户端UUID号";
                cJSON_free(item_client_info);
                return -1;
            }
        }
    }

    client_all_info* client_all_info = new client_all_info_t;//分配客户端信息内存
    strcpy((char*)client_all_info->client_info.client_uuid,item_client_client_uuid->valuestring);
    qDebug() <<"client_all_info->client_info.client_uuid:" << QString((char*)client_all_info->client_info.client_uuid) << Qt::endl;

    cJSON* item_client_name = cJSON_GetObjectItem(item_client_info,"client_name");
    strcpy((char*)client_all_info->client_info.client_name,item_client_name->valuestring);
    qDebug() <<"client_all_info->client_info.client_name:" << QString((char*)client_all_info->client_info.client_name) << Qt::endl;

    QByteArray ba = TcpSocketList.at(client_index)->client_ip.toLatin1();
    strcpy((char*)client_all_info->client_info.client_ip,ba.data());
    qDebug() <<"client_all_info->client_info.client_ip:" << client_all_info->client_info.client_ip << Qt::endl;

    client_all_info->client_info.client_port = TcpSocketList.at(client_index)->client_port;
    qDebug() <<"client_all_info->client_info.client_port:" << client_all_info->client_info.client_port << Qt::endl;

    client_all_info->client_info.client_online = 1;
    qDebug() <<"client_all_info->client_info.client_onlline:" << client_all_info->client_info.client_online << Qt::endl;

    cJSON* item_client_version = cJSON_GetObjectItem(item_client_info,"client_version");
    strcpy((char*)client_all_info->client_info.client_version,item_client_version->valuestring);
    qDebug() <<"client_all_info->client_info.client_version:" << QString((char*)client_all_info->client_info.client_version) << Qt::endl;


    cJSON* item_room_name = cJSON_GetObjectItem(item_client_info,"room_name");
    strcpy((char*)client_all_info->room_info.room_name,item_room_name->valuestring);
    qDebug() <<"client_all_info->room_info.room_name:" << QString((char*)client_all_info->room_info.room_name) << Qt::endl;

    cJSON* item_substation_name = cJSON_GetObjectItem(item_client_info,"substation_name");
    strcpy((char*)client_all_info->room_info.substation_name,item_substation_name->valuestring);
    qDebug() <<"client_all_info->room_info.substation_name:" << QString((char*)client_all_info->room_info.substation_name) << Qt::endl;

    cJSON* item_camera_num = cJSON_GetObjectItem(item_client_info,"camera_num");
    client_all_info->camera_num = item_camera_num->valueint;
    qDebug() <<"client_all_info->camera_num" << client_all_info->camera_num << Qt::endl;

    //解析json数组
    cJSON* item_camera_list = cJSON_GetObjectItem(item_client_info,"camera_list");
    int count = 0;
    if(NULL != item_camera_list) {
        cJSON* array_camera_info = item_camera_list->child;
        while(array_camera_info != NULL) {//相机索引
            cJSON* item_camera_device_name = cJSON_GetObjectItem(array_camera_info,"camera_device_name");
            strcpy((char*)client_all_info->camera_device_info[count].camera_device_name,item_camera_device_name->valuestring);
            qDebug() <<"client_all_info->camera_device_info[count].camera_device_name:" << QString((char*)client_all_info->camera_device_info[count].camera_device_name) << Qt::endl;

            cJSON* item_lamp_device_name = cJSON_GetObjectItem(array_camera_info,"lamp_device_name");
            strcpy((char*)client_all_info->lamp_device_info[count].lamp_device_name,item_lamp_device_name->valuestring);
            qDebug() <<"client_all_info->lamp_device_info[count].lamp_device_name:" << QString((char*)client_all_info->lamp_device_info[count].lamp_device_name) << Qt::endl;


            cJSON* item_camera_parms = cJSON_GetObjectItem(array_camera_info,"camera_parms");
            cJSON* item_width = cJSON_GetObjectItem(item_camera_parms,"width");
            client_all_info->camera_device_info[count].camera_paras_info.width = item_width->valueint;
            qDebug() <<"width:" << client_all_info->camera_device_info[count].camera_paras_info.width  << Qt::endl;


            cJSON* item_height = cJSON_GetObjectItem(item_camera_parms,"height");
            client_all_info->camera_device_info[count].camera_paras_info.height = item_height->valueint;
            qDebug() <<"height:" << client_all_info->camera_device_info[count].camera_paras_info.height  << Qt::endl;

            cJSON* item_fps = cJSON_GetObjectItem(item_camera_parms,"fps");
            client_all_info->camera_device_info[count].camera_paras_info.fps = item_fps->valueint;
            qDebug() <<"fps:" << client_all_info->camera_device_info[count].camera_paras_info.fps  << Qt::endl;


            cJSON* item_white_balance_temp = cJSON_GetObjectItem(item_camera_parms,"white_balance_temp");
            client_all_info->camera_device_info[count].camera_paras_info.camera_ctrl_paras.val_white_balance_temp  = item_white_balance_temp->valueint;
            qDebug() <<"val_white_balance_temp:" << client_all_info->camera_device_info[count].camera_paras_info.camera_ctrl_paras.val_white_balance_temp  << Qt::endl;

            cJSON* item_brightness = cJSON_GetObjectItem(item_camera_parms,"brightness");
            client_all_info->camera_device_info[count].camera_paras_info.camera_ctrl_paras.val_brightness  = item_brightness->valueint;
            qDebug() <<"val_brightness:" << client_all_info->camera_device_info[count].camera_paras_info.camera_ctrl_paras.val_brightness  << Qt::endl;

            cJSON* item_contrast = cJSON_GetObjectItem(item_camera_parms,"contrast");
            client_all_info->camera_device_info[count].camera_paras_info.camera_ctrl_paras.val_contrast  = item_contrast->valueint;
            qDebug() <<"val_contrast:" << client_all_info->camera_device_info[count].camera_paras_info.camera_ctrl_paras.val_contrast  << Qt::endl;

            cJSON* item_saturation = cJSON_GetObjectItem(item_camera_parms,"saturation");
            client_all_info->camera_device_info[count].camera_paras_info.camera_ctrl_paras.val_saturation  = item_saturation->valueint;
            qDebug() <<"val_saturation:" << client_all_info->camera_device_info[count].camera_paras_info.camera_ctrl_paras.val_saturation  << Qt::endl;

            cJSON* item_sharpness = cJSON_GetObjectItem(item_camera_parms,"sharpness");
            client_all_info->camera_device_info[count].camera_paras_info.camera_ctrl_paras.val_sharpness  = item_sharpness->valueint;
            qDebug() <<"val_sharpness:" << client_all_info->camera_device_info[count].camera_paras_info.camera_ctrl_paras.val_sharpness  << Qt::endl;

            cJSON* item_exposure = cJSON_GetObjectItem(item_camera_parms,"exposure");
            client_all_info->camera_device_info[count].camera_paras_info.camera_ctrl_paras.val_exposure  = item_exposure->valueint;
            qDebug() <<"val_exposure:" << client_all_info->camera_device_info[count].camera_paras_info.camera_ctrl_paras.val_exposure  << Qt::endl;

            cJSON* item_video_coding_fmt = cJSON_GetObjectItem(item_camera_parms,"video_coding_fmt");
            client_all_info->camera_device_info[count].camera_paras_info.video_coding_fmt = item_video_coding_fmt->valueint;
            qDebug() <<"video_coding_fmt:" << client_all_info->camera_device_info[count].camera_paras_info.video_coding_fmt  << Qt::endl;

            array_camera_info = array_camera_info->next;
            count++;

        }
    }


    clientList.append(client_all_info);  //地址不变
    cJSON_free(item_client_info);//释放内存

    return 0;
}

void myTask::set_client_all_info_database(dbConnect_info_t *m_dbconnect_info, client_all_info_t *client_all_info)
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;

    int ret;
    sqlBase sqlbase;
    QString client_uuid = QString((char*)client_all_info->client_info.client_uuid);

    bool is_client_in_database = sqlbase.is_client_in_database(m_dbconnect_info->dbconn,client_uuid);
    if(is_client_in_database == false) { //没有存在
        ret = sqlbase.insert_to_client_info(m_dbconnect_info->dbconn,client_all_info);
        if(ret > 0) {
            qDebug()<<"client_info数据库插入成功!"<<Qt::endl;
            //QMessageBox::information(0,"提示", "client_info数据库插入成功!");
        }
        else {
             qDebug()<<"client_info数据库插入失败!"<<Qt::endl;
            //QMessageBox::information(0,"提示", "client_info数据库插入失败!");
            return;
        }
    }
    else { //服务器存在该客户端信息  更新其在线状态
        ret = sqlbase.update_to_client_info(m_dbconnect_info->dbconn,client_all_info->client_info.client_online,client_uuid);
        if(ret > 0) {
            qDebug()<<"client_online 数据库更新成功!"<<Qt::endl;
            //QMessageBox::information(0,"提示", "client_online 数据库更新成功!");
        }
        else {
              qDebug()<<"client_online 数据库更新失败"<<Qt::endl;
            //QMessageBox::information(0,"提示", "client_online 数据库更新失败!");
            return;
        }
    }

    QString room_name = QString((char*)client_all_info->room_info.room_name);
    QString substation_name = QString((char*)client_all_info->room_info.substation_name);
    bool is_roominfo_in_database = sqlbase.is_roominfo_in_database(m_dbconnect_info->dbconn,room_name,substation_name);
    if(is_roominfo_in_database == false) {
        ret = sqlbase.insert_to_room_info(m_dbconnect_info->dbconn,client_all_info);
        if(ret > 0) {
            //QMessageBox::information(0,"提示", "room_info数据库插入成功!");
             qDebug()<<"room_info数据库插入成功!"<<Qt::endl;
        }
        else {
            //QMessageBox::information(0,"提示", "room_info数据库插入失败!");
            qDebug()<<"room_info数据库插入失败!"<<Qt::endl;
            return;
        }
    }

    for(int i=0;i<client_all_info->camera_num;i++) {

        QString camera_device_name = QString((char*)client_all_info->camera_device_info[i].camera_device_name);
        bool is_camerainfo_in_database = sqlbase.is_camerainfo_in_database(m_dbconnect_info->dbconn,client_uuid,camera_device_name);
        if(is_camerainfo_in_database == false) {
            ret = sqlbase.insert_to_camera_info(m_dbconnect_info->dbconn,client_all_info,i);
            if(ret > 0) {
                //QMessageBox::information(0,"提示", "camera_info数据库插入成功!");
                 qDebug()<<"camera_info数据库插入成功!"<<Qt::endl;
            }
            else {
                //QMessageBox::information(0,"提示", "camera_info数据库插入失败!");
                qDebug()<<"camera_info数据库插入失败!"<<Qt::endl;
                return;
            }
        }

        bool is_lampinfo_in_database = sqlbase.is_lampinfo_in_database(m_dbconnect_info->dbconn,client_uuid,camera_device_name);
        if(is_lampinfo_in_database == false) {
            ret = sqlbase.insert_to_lamp_info(m_dbconnect_info->dbconn,client_all_info,i);
            if(ret > 0) {
                qDebug()<<"lamp_info数据库插入成功!"<<Qt::endl;
                //QMessageBox::information(0,"提示", "lamp_info数据库插入成功!");
            }
            else {
                //QMessageBox::information(0,"提示", "lamp_info数据库插入失败!");
                qDebug()<<"lamp_info数据库插入失败!"<<Qt::endl;
                return;
            }
        }
    }
}

void myTask::get_video_path(cJSON *json, int client_index)
{
    qDebug() << "FUN:" << __func__ << "LINE:" << __LINE__  << "FILE:" << __FILE__ << Qt::endl;

    //客户端uuid
    cJSON *item_client_uuid = cJSON_GetObjectItem(json,"client_uuid");

    //解析数组
    cJSON* item_camera_image_info = cJSON_GetObjectItem(json,"camera_image_info");
    int camera_count = 0;
    if(NULL != item_camera_image_info) {
        cJSON* array_camera_image_info = item_camera_image_info->child;
        while(array_camera_image_info != NULL) { //相机数量
            //相机名称
            cJSON* item_camera_device_name = cJSON_GetObjectItem(array_camera_image_info,"camera_device_name");
            //相机图片
            cJSON *item_image = cJSON_GetObjectItem(array_camera_image_info,"image");
            cJSON *item_alarm_flag = cJSON_GetObjectItem(array_camera_image_info,"alarm_flag");
            //将base64图片数据转为QPixmap类型  已经经过处理标记好的图像
            QString str(item_image->valuestring);
            qDebug() << "image str len" <<  str.length() <<Qt::endl;
            QPixmap map;
            map.loadFromData(QByteArray::fromBase64(str.toLocal8Bit()));

 /*           //转为QImage类型
            QImage image = map.toImage();
            //转为Mat类型
            Mat img;
            switch (image.format())
            {
                case QImage::Format_ARGB32:
                case QImage::Format_RGB32:
                case QImage::Format_ARGB32_Premultiplied:
                img = cv::Mat(image.height(), image.width(), CV_8UC4, (void*)image.constBits(), image.bytesPerLine());
                break;
                case QImage::Format_RGB888:
                img = cv::Mat(image.height(), image.width(), CV_8UC3, (void*)image.constBits(), image.bytesPerLine());
                cv::cvtColor(img, img, COLOR_BGR2RGB);
                break;
                case QImage::Format_Indexed8:
                img = cv::Mat(image.height(), image.width(), CV_8UC1, (void*)image.constBits(), image.bytesPerLine());
                break;
                default:
                break;
            }
*/
            //自动创建文件夹 当前
            QString fileName = QDir::currentPath();
            fileName += "/result/image/client";
            QString image_path = QString("%1%2%3%4%5%6%7").arg(fileName).arg(client_index + 1).arg("/camera").arg(camera_count).arg("_image").arg(number_frame[camera_count]).arg(".jpg");
            qDebug() << "image_path:" <<  image_path <<Qt::endl;

            QByteArray ba_image_path = image_path.toLatin1();
           // vector<int> compression_params;
            //compression_params.push_back( IMWRITE_JPEG_QUALITY);
           // compression_params.push_back(100);
            //imwrite(ba_image_path.data(),img,compression_params);
            map.save(ba_image_path.data(),Q_NULLPTR,100);
            taskResult_info.number_frame[camera_count] = number_frame[camera_count];

            //帧数加一
            number_frame[camera_count]++;
            qDebug() << "image num for camera[" << camera_count << "] is "  << number_frame[camera_count];
            camera_count ++;
            array_camera_image_info = array_camera_image_info->next;//下一个数组元素

            taskResult_info.VideoImgPathList.append(image_path);


            /* 是否报警 */
            if(item_alarm_flag->valueint == 1) {
               qDebug() << "指示灯告警 指示灯告警 指示灯告警 指示灯告警" << Qt::endl;
            }
/*
            //界面显示
            switch (client_index) {
                case 0:
                    map = QPixmap(ba_image_path.data());
                    if(camera_count == 0)
                        ui->ui_client1_show_lab1 ->setPixmap(map);
                    if(camera_count == 1)
                        ui->ui_client1_show_lab2->setPixmap(map);
                    if(camera_count == 2)
                        ui->ui_client1_show_lab3->setPixmap(map);
                    if(camera_count == 3)
                        ui->ui_client1_show_lab4->setPixmap(map);
                    break;
                case 1:
                    break;
                default:
                    break;
            }
*/

        }//end while
    }//end if

    emit Result(&taskResult_info);
}

void myTask::get_imgcap(cJSON *json, int client_index)
{
    cJSON* item_server_uuid = cJSON_GetObjectItem(json,"server_uuid");
    cJSON* item_client_uuid = cJSON_GetObjectItem(json,"client_uuid");
    cJSON* item_camera_name = cJSON_GetObjectItem(json,"camera_name");
    cJSON *item_image = cJSON_GetObjectItem(json,"image");

    //得到相机索引
    sqlBase sqlbase;
    int camera_index = 0;
    for(int i=0;i<clientList.size();i++) {//客户端索引
        for(int j=0;j<clientList.at(i)->camera_num;j++) {//相机索引
            if(QString(item_camera_name->valuestring) == QString((char*)clientList.at(i)->camera_device_info[j].camera_device_name))
                camera_index = j;
        }
    }
    cap_record_info.client_uuid = QString(item_client_uuid->valuestring);
    cap_record_info.camera_device_name = QString(item_camera_name->valuestring);
    //从数据库中得到number_imgcap
    number_imgcap[camera_index] = sqlbase.get_number_of_imgcap(m_dbconnect_info.dbconn,cap_record_info.client_uuid,cap_record_info.camera_device_name);

    //将base64图片数据转为QPixmap类型  已经经过处理标记好的图像
    QString str(item_image->valuestring);
    qDebug() << " lenth of str" <<  str.length() <<Qt::endl;
    if(str.length() <= 5) {
         qDebug()<<"图片抓取失败，请检查图片是否存在!"<<Qt::endl;
         //QMessageBox::warning(0,"提示", "图片抓取失败，请检查图片是否存在!");
         return;
    }

    //base64转QPixmap
    QPixmap map;
    map.loadFromData(QByteArray::fromBase64(str.toLocal8Bit()));
    //构造文件夹和文件名
    QString fileName = QDir::currentPath();
    fileName += "/result/imagecap/client";
    QString image_path = QString("%1%2%3%4%5%6%7").arg(fileName).arg(client_index + 1).arg("/camera").arg(camera_index).arg("/imagecap").arg(number_imgcap[camera_index]).arg(".jpg");
    qDebug() << "image_path:" <<  image_path <<Qt::endl;
    cap_record_info.cap_image_path = image_path;
    taskResult_info.img_cap_path = image_path;
    //保存到构造的文件夹中
    QByteArray ba_image_path = image_path.toLatin1();
    map.save(ba_image_path.data(),Q_NULLPTR,100);

    //图片数量增加1
    number_imgcap[camera_index]++;

    //数据库更新  抓拍记录表
    QString sql = "SELECT MAX(cap_record_info.cap_record_no) FROM cap_record_info;";
    int cap_record_no = sqlbase.get_maxid(m_dbconnect_info.dbconn,sql);
    qDebug() << "cap_record_no::" << cap_record_no ;
    cap_record_info.cap_record_no = cap_record_no + 1;

    QDateTime dateTime = QDateTime::currentDateTime();
    long long cap_timestamp = dateTime.toMSecsSinceEpoch();
    qDebug() << "cap_timestamp::" << cap_timestamp ;
    cap_record_info.cap_timestamp = cap_timestamp;

    cap_record_info.user_id = userid_login;

    int ret = sqlbase.insert_to_cap_record_info(m_dbconnect_info.dbconn,&cap_record_info);
    if(ret > 0) {
        qDebug()<<"cap_record_info数据库插入成功!"<<Qt::endl;
        //QMessageBox::information(0,"提示", "cap_record_info数据库插入成功!");
    }
    else {
        //QMessageBox::information(0,"提示", "cap_record_info数据库插入成功!");
        qDebug()<<"cap_record_info数据库插入成功!"<<Qt::endl;
        return;
    }
}


