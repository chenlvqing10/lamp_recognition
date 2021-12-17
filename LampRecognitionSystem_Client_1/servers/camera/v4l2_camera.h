#ifndef __V4L2_CAMERA_H
#define __V4L2_CAMERA_H

#include <linux/videodev2.h>
#include <linux/v4l2-controls.h>
#include <stdio.h>

#define _DT_CHR     2

//define return error value
#define CAMERA_GET_DEVICE_INFO_ERROE -1
#define CAMERA_OPEN_ERROR  -2
#define VIDIOC_QUERYCAP_ERROR -3
#define VIDIOC_S_FMT_ERROR    -4
#define VIDIOC_G_FMT_ERROR    -5
#define VIDIOC_S_PARM_ERROR    -6
#define VIDIOC_G_PARM_ERROR    -7
#define VIDIOC_REQBUFS_ERROR    -8
#define MALLOC_ERROR            -9
#define VIDIOC_QUERYBUF_ERROR   -10
#define MMP_ERROR -11
#define VIDIOC_QBUF_ERROR  -12
#define VIDIOC_STREAMON_ERROR  -13
#define VIDIOC_DQBUF_ERROR   -14
#define UMMMP_ERROR -15
#define VIDIOC_STREAMOFF_ERROR -16
#define V4L2_UTILS_SET_EXPSURE_AUTO_TYPE_ERR  -55
#define V4L2_UTILS_SET_EXPSURE_ERR            -54
#define V4L2_UTILS_SET_WHITE_BALANCE_AUTO_TYPE_ERR  -57
#define V4L2_UTILS_SET_WHITE_BALANCE_ERR    -58
#define V4L2_UTILS_SET_BRIGHTNESS_ERR    -59
#define MAXVIDEONUM 20
#define PATHLEN     300

//define defult resolution width * height 
#define IMAGEWIDTH   1280
#define IMAGEHEIGHT  720
#define FPS          30
#define EXPOSURE_DEFULT 300
#define WHITE_BALANCE_DEFULT    4600
#define BRIGHTNESS_DEFULT       0
#define CONTRAST_DEFULT         32
#define SATURATION_DEFULT       64
#define SHARPNESS_DEFULT        50
#define VIDEO_CODING_FORMAT_H264 1

#define CLIENTNAME "LAMP_RECOGNITION_CLIENT1"
#define ONLINE_STATUS_ON  1
#define ONLINE_STATUS_OFF 0


const char lamp_device_name[20][20] = {"Lamp_Device_1","Lamp_Device_2","Lamp_Device_3","Lamp_Device_4","Lamp_Device_5",
                                       "Lamp_Device_6","Lamp_Device_7","Lamp_Device_8","Lamp_Device_9","Lamp_Device_10",
                                       "Lamp_Device_11","Lamp_Device_12","Lamp_Device_13","Lamp_Device_14","Lamp_Device_15",
                                       "Lamp_Device_16","Lamp_Device_17","Lamp_Device_18","Lamp_Device_19","Lamp_Device_20"};


//客户端信息
typedef struct client_info{
    unsigned char client_uuid[20];
    unsigned char client_name[50];
    unsigned int  client_online;
    unsigned char client_version[50];
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



/* mmap to user spaces buffer */
struct VideoBuffer {
    void* start;//mmap start addr
    size_t length;
};

int v4l2_get_camera_device_info();
int v4l2_queryctrl_value(int cam_fd,int V4L2_cid);
int v4l2_get_ctrl_value(int cam_fd,int V4L2_cid);
int v4l2_set_white_balance(int cam_fd,int val_white_balance_temp);
int v4l2_set_exposure(int cam_fd,int val_exposure);
int v4l2_set_ctrl_value(int cam_fd,int V4L2_cid,int V4L2_value);
int v4l2_set_fmtd(int cam_fd,int width,int height);
int v4l2_set_streamparm(int cam_fd,int fps);

int v4l2_init_camera(void);
int v4l2_camera_reqbuff(int cam_fd,int camera_index,int frame_count);
int v4l2_start_camera(FILE* file,int cam_fd,int camera_index,int frame_count);
int v4l2_stop_camera(int cam_fd,int camera_index,int frame_count);

#endif 
