#ifndef EVENTBASE_H
#define EVENTBASE_H
#include <QString>
#include <QStringList>
#include <QTcpSocket>

/* 定义客户端信息 服务端信息 一些全局结构体数据*/
#define ONLINE_STATUS_ON  1  //在线状态
#define ONLINE_STATUS_OFF 0  //离线状态
#define MAXVIDEONUM 20      //一个客户端拥有的相机数量


/* 客户端和服务端交互的一些事件，事件类型，命令定义 */

#define EVENT_CLIENT_CONN          "客户端连接事件"
#define EVENT_APP_BIND             "app绑定事件"
#define EVENT_SERVER_IMAGE_CAP     "服务器图片抓拍事件"
#define EVENT_APP_IMAGE_CAP        "app图片抓拍事件"
#define EVENT_CLIENT_HEART         "客户端心跳包"
#define EVENT_APP_HEART            "app心跳包"
#define EVENT_SERVER_CLIENT_HEART  "服务器向客户端心跳包"
#define EVENT_SERVER_APP_HEART     "服务器向app端心跳包"
#define EVENT_CLIENT_ALARM         "客户端报警事件"
#define EVENT_SERVER_ALARM_CHECK   "服务器每日定时报警信息检查事件"
#define EVENT_SERVER_CAMERA_PARMS_CHANGE "服务器端修改相机参数"
#define EVENT_APP_CAMERA_PARMS_CHANGE    "app端修改相机参数"
#define EVENT_CLIENT_DISCONN             "客户端断开"
#define EVENT_SERVER_DISCONN             "服务器断开"
#define EVENT_SERVER_START_RESTART_SYNC  "服务器启动或重启同步事件"
#define EVENT_SERVER_CLIENT_UPGRADE      "客户端版本升级"
#define EVENT_SERVER_UPGRADE             "服务器升级"
#define EVENT_SERVER_UPGRADE_AUTOCHECK   "app升级自动检查事件"
#define EVENT_APP_UPGRADE                "app升级"
#define EVENT_CLIENT_GET_VIDEO           "客户端视频监控事件"
#define EVENT_SERVER_SAVE_VIDEO          "服务器视频封装保存事件"
#define EVENT_SERVER_REGISTER            "用户注册事件"
#define EVENT_SERVER_LOGIN               "用户登录事件"
#define EVENT_SERVER_PASSWD_RESET        "用户重置密码事件"

enum EVENT_TYPE {
    EVENT_TYPE_CLIENT_CONN          = 1001,//客户端连接事件  ok
    EVENT_TYPE_APP_BIND             = 1002,//app绑定事件
    EVENT_TYPE_SERVER_IMAGE_CAP     = 1003,//服务器图片抓拍事件  ok
    EVENT_TYPE_APP_IMAGE_CAP        = 1004,//app图片抓拍事件
    EVENT_TYPE_CLIENT_HEART         = 1005,//客户端心跳包  ok
    EVENT_TYPE_APP_HEART            = 1006,//app心跳包
    EVENT_TYPE_SERVER_CLIENT_HEART  = 1007,//服务器向客户端心跳包  ok
    EVENT_TYPE_SERVER_APP_HEART     = 1008,//服务器向app端心跳包
    EVENT_TYPE_CLIENT_ALARM         = 1009,//客户端报警事件  ok
    EVENT_TYPE_SERVER_ALARM_CHECK   = 1010,//服务器每日定时报警信息检查事件   ok
    EVENT_TYPE_SERVER_CAMERA_PARMS_CHANGE = 1011,//服务器端修改相机参数  ok
    EVENT_TYPE_APP_CAMERA_PARMS_CHANGE    = 1012,//app端修改相机参数
    EVENT_TYPE_CLIENT_DISCONN             = 1013,//客户端断开
    EVENT_TYPE_SERVER_DISCONN             = 1014,//服务器断开
    EVENT_TYPE_SERVER_START_RESTART_SYNC  = 1015,//服务器启动或重启同步事件
    EVENT_TYPE_SERVER_CLIENT_UPGRADE      = 1016,//客户端版本升级
    EVENT_TYPE_SERVER_UPGRADE             = 1017,//服务器升级
    EVENT_TYPE_SERVER_UPGRADE_AUTOCHECK   = 1018,//app升级自动检查事件
    EVENT_TYPE_APP_UPGRADE                = 1019,//app升级
    EVENT_TYPE_CLIENT_GET_VIDEO           = 1020,//客户端视频监控事件
    EVENT_TYPE_SERVER_SAVE_VIDEO          = 1021,//服务器视频封装保存事件
    EVENT_TYPE_SERVER_REGISTER            = 1022,//用户注册事件
    EVENT_TYPE_SERVER_LOGIN               = 1023,//用户登录事件
    EVENT_TYPE_SERVER_PASSWD_RESET        = 1024,//用户重置密码事件
    EVENT_TYPE_SERVER_TRAIN_ALGORITHM     = 1025//服务器算法训练
};

enum EVENT_DIRECTION {
    EVENT_DIRECTION_SERVER_TO_CLIENT = 1,
    EVENT_DIRECTION_CLIENT_TO_SERVER = 2,
    EVENT_DIRECTION_SERVER_TO_APP    = 3,
    EVENT_DIRECTION_APP_TO_SERVER    = 4,
    EVENT_DIRECTION_SERVER_ONLY      = 5,
    EVENT_DIRECTION_CLIENT_ONLY      = 6,
    EVENT_DIRECTION_APP_ONLY         = 7
};

#define CMD_CLIENT_CONN                             "client_conn"
#define CMD_CLIENT_GET_VIDEO                        "client_get_video"
#define CMD_SERVER_PARAS_CHG                        "ser_parms_chg"
#define CMD_SERVER_HEART                            "ser_heart"
#define CMD_SERVER_IMG_CAP                          "ser_img_cap"
#define CMD_SERVER_TRAIN_COLOR_ALGORITHM            "ser_train_color_algorithm"
#define CMD_SERVER_TRAIN_BRIGHTNESS_ALGORITHM       "ser_train_brightness_algorithm"
#define CMD_CLIENT_SERVER_IMG_CAP_ACK               "client_ser_img_cap_ack"


//服务器信息
typedef struct server_info {
    char server_uuid[40];
    char server_name[30];
    char server_mac[20];
    char server_ip[20];
    char server_outip[20];
    int  server_port;//监听端口号
    char server_version[20];
}server_info_t;


//客户端信息
typedef struct client_info{
    unsigned char client_uuid[20];
    unsigned char client_name[50];
    unsigned int  client_online;
    unsigned char client_version[50];
    unsigned char client_ip[50];
    unsigned short client_port;
}client_info_t;

//摄像头控制参数信息
typedef struct camera_contrl_paras {
    unsigned short val_white_balance_temp;  //white balance temperature value
    unsigned short val_brightness; //brightness
    unsigned short val_contrast;
    unsigned short val_saturation;
    unsigned short val_sharpness;
    unsigned short val_exposure;
}camera_contrl_paras_t;

//摄像头整体参数信息
typedef struct camera_paras{
    unsigned short width;           //resolution width
    unsigned short height;          //resolution height
    unsigned short fps;             //frame per second
    unsigned short video_coding_fmt;//video coding format
    camera_contrl_paras_t camera_ctrl_paras;
}camera_paras_t;

//单个摄像头信息
typedef struct camera_device_info{
    unsigned char camera_id[20];
    unsigned char camera_device_name[20];  //one camera have a device name  /dev/video0
    camera_paras_t camera_paras_info; //one camera have it's parameters
}camera_device_info_t;

//单个房间信息 变电站信息
typedef  struct room_info{
   unsigned char room_name[50];
   unsigned char substation_name[50];
}room_info_t;

//单个指示灯信息(图片范围)
typedef struct lamp_device_info{
    unsigned char lamp_id[20];
    unsigned char lamp_device_name[20];
}lamp_device_info_t;

//一个客户端拥有的信息
typedef struct client_all_info{
    camera_device_info_t camera_device_info[MAXVIDEONUM];//多个摄像头
    lamp_device_info_t   lamp_device_info[MAXVIDEONUM];//多个摄像头拍摄的指示灯(设备图片)
    room_info_t room_info;//安装的房间信息
    client_info_t client_info;//客户端信息
    int camera_num;//摄像头的个数 = 指示灯个数(图片)
}client_all_info_t;

#define SERVERNAME  "LAMP_RECOGNITION_SERVER1"
#define COLOR_FILTER_TEMPLATE 0
#define BRIGHTNESS_FILTER_TEMPLATE 1


//数据交互业务逻辑处理结果和主界面UI控件交互的结构体
typedef  struct taskThread_Result_info{ //开头不能为类
    bool  client_add_flag;              //新的客户端连接 添加客户端界面
    bool  client_cancel_flag;           //客户端断开    重绘客户端界面
    bool  client_show_flag;             //客户端连续采集的图片信息  展示在客户端界面
    bool  client_show_imgcap;           //客户端响应服务端图片抓拍要求 展示在图片抓拍界面
    bool  newpwd_flag;

    int current_client;                 //客户端索引
    int   number_frame[MAXVIDEONUM];              //不同相机的拍摄图片帧数

    QString cmd;                        //命令
    QString string_RecivedData;         //接收数据
    QStringList VideoImgPathList;       //摄像头实时图像路径
    QString img_cap_path;               //抓拍图像路径

}taskThread_Result_info_t;

//处理QTcpSocket数据类型
typedef struct client_net_info {
    QTcpSocket* TcpClientSocket;
    QString     client_ip;
    int         client_port;
}client_net_info_t;

//图片抓拍数据 用于数据库
typedef struct cap_record_info {
    int cap_record_no;
    QString user_id;
    QString client_uuid;
    QString camera_device_name;
    long long cap_timestamp;
    QString cap_image_path;
}cap_record_info_t ;

typedef struct imgcut_parms{
    double org_width;     //原图宽
    double org_height;    //原图长
    double x;             //截图起始横坐标
    double y;             //截图起始坐坐标
    double width;         //截图框的宽度
    double height;        //截图框的长度
}imgcut_parms_t;

typedef struct imgcut_info {
    bool            database_flag;
    bool            is_resize;
    unsigned char   client_name[50];
    unsigned char   camera_device_name[50];
    unsigned char   algorithm_name[40];
    unsigned char   algorithm_version[20];
    unsigned char   save_path[150];
    unsigned char   org_path[150];
    unsigned char   client_uuid[50];
    int camera_id;
    int algorithm_id;
    imgcut_parms_t imgcut_parms;

}imgcut_info_t;



typedef struct lamp_color_template_info{
    int lamp_template_no;
    int camera_id;
    int algorithm_id;
    int is_resize;
    int area_min;
    int area_max;
    int color_num;

    int imgorg_width;
    int imgorg_height;
    int imgcut_x;
    int imgcut_y;
    int imgcut_width;
    int imgcut_height;
    int time_offset;

     QString template_image_org_path;
    QString template_image_cut_path;
    QString client_uuid;
    QStringList colorinfoList;

}lamp_color_template_info_t;

typedef struct brightness_template_info {
    int     brightness_template_no;
    int     camera_id;
    int     algorithm_id;
    int     is_resize;      //是否缩放
    int     imgorg_width;
    int     imgorg_height;
    int     imgcut_x;
    int     imgcut_y;
    int     imgcut_width;
    int     imgcut_height;

    int     maskVaule;      //二值化阈值
    int     num_row;        //行数
    int     num_cols;       //列数
    int     offset;         //位置偏移值
    int     off_x_offset;    //off标记x偏移值
    int     off_y_offset;    //off标y记偏移值
    int     time_offset;    //时间标记偏移值

    QString template_image_cut_path;
    QString template_image_org_path;
    QString client_uuid;
}brightness_template_info_t ;//模型 输入参数



#endif // EVENTBASE_H
