#include <stdio.h>
#include <stdlib.h>
#include "v4l2_camera.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

int cam_FdList[MAXVIDEONUM] = {0};   //all camera device fd
int number_frame = 0;                //all the video frame

static struct VideoBuffer* buffers[MAXVIDEONUM];   //mmap to user spaces buffe
client_all_info_t client_all_info;

/* get camera(video) device info for client
*  camera number,camera device name,lamp device name
*/
int v4l2_get_camera_device_info()
{
    /* init info */
    char devicename[PATHLEN] = {0};
    char videoname[MAXVIDEONUM][PATHLEN];
    memset(videoname,0,sizeof(videoname));
    client_all_info.camera_num = 0;

    /* open /dev dir;then read the file in this dir */
    DIR* dir = opendir("/dev");
    struct dirent *ptr;

    if(dir == NULL) {
        printf("open /dev failed\n");
        return -1;
    }

    int count = 0;
    while((ptr = readdir(dir)) != NULL)
    {
        if(ptr->d_type == _DT_CHR)
        {
            for(int i = 0; i < MAXVIDEONUM; i++) {
                sprintf(devicename,"video%d",i);
                /* find video device videoname = /dev/video0 /dev/video1*/
                if(!strcmp(ptr->d_name,devicename)) {
                    sprintf(devicename,"/dev/%s",ptr->d_name);
                    strcpy(videoname[count],devicename);
                    count++;
                }
            }
        }
    }

    /* becase of firefly camera driver bug(when using usb insert into a camera device ,then show two device file)
     * /dev/video0  /dev/video1,so should test which one is working
   */
    struct v4l2_format fmt;
     //set default video capture parameters設置默認的攝像頭捕獲參數
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.width  = 1280;
    fmt.fmt.pix.height = 720;
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

    /* check all video device file in /dev dir*/
    for(int i = 0;i< count;i++)
    {
        printf("video name = %s\n",videoname[i]);
        int fd = open(videoname[i],O_RDONLY);
        if(fd < 0) {
            printf("open %s failed\n",videoname[i]);
            close(fd);
        }
        else {
            printf("open %s success\n",videoname[i]);
            if(ioctl(fd,VIDIOC_S_FMT,&fmt) != -1) {
                strcpy((char*)client_all_info.camera_device_info[client_all_info.camera_num].camera_device_name,videoname[i]);
                strcpy((char*)client_all_info.lamp_device_info[client_all_info.camera_num].lamp_device_name,lamp_device_name[i]);
                client_all_info.camera_num++;
            }
            close(fd);
        }
    }

    printf("in this client ,having %d devices\n",client_all_info.camera_num);
    for(int i =0;i< client_all_info.camera_num;i++) {
        printf("in this client,camera[%d] is [%s]\n",i,client_all_info.camera_device_info[i].camera_device_name);
        printf("in this client,lamp[%d] is [%s]\n",i,client_all_info.lamp_device_info[i].lamp_device_name);
    }

    return 0;
}


/* query camera control parameters */
int v4l2_queryctrl_value(int cam_fd,int V4L2_cid)
{
	struct v4l2_queryctrl queryctrl;
	queryctrl.id = V4L2_cid;
	int ret = ioctl(cam_fd, VIDIOC_QUERYCTRL, &queryctrl);
	if(ret < 0) {
		perror("ioctl VIDIOC_QUERYCTRL");
		return -1;
	}

	printf("v4l2_queryctrl.mini = %d v4l2_queryctrl.max = %d v4l2_queryctrl.step = %d\n",queryctrl.minimum,queryctrl.maximum,queryctrl.step);
	return 0;
}

/* get camera control parameters value by v4l2_cid;for example brightness contrast ...*/
int v4l2_get_ctrl_value(int cam_fd,int V4L2_cid)
{
	struct v4l2_control ctrl;
	ctrl.id = V4L2_cid;
	int ret = ioctl(cam_fd, VIDIOC_G_CTRL, &ctrl);
	if(ret < 0) {
		perror("ioctl VIDIOC_G_CTRL");
		return -1;
	}

	printf("VALUE IS %d\n",ctrl.value);
	
	return ctrl.value;
}

/* set white balance temperature cvalue 2800 ~ 6500，default = 4600 */
int v4l2_set_white_balance(int cam_fd,int val_white_balance_temp)
{
	struct v4l2_control ctrl;
    ctrl.id = V4L2_CID_AUTO_WHITE_BALANCE;
    ctrl.value = V4L2_WHITE_BALANCE_MANUAL ;
    if(ioctl(cam_fd,VIDIOC_S_CTRL,&ctrl)==-1)
    {
        perror("ioctl VIDIOC_G_CTRL for V4L2_CID_AUTO_WHITE_BALANCE");
        return V4L2_UTILS_SET_WHITE_BALANCE_AUTO_TYPE_ERR;
    }

    ctrl.id = V4L2_CID_WHITE_BALANCE_TEMPERATURE;
    ctrl.value = val_white_balance_temp;
     if(ioctl(cam_fd,VIDIOC_S_CTRL,&ctrl)==-1)
    {
        perror("ioctl VIDIOC_S_CTRL for V4L2_CID_WHITE_BALANCE_TEMPERATURE");
        return V4L2_UTILS_SET_WHITE_BALANCE_ERR;
    }

	 printf("set white balance ok\n");
	return 0;
}

/* set exposure 300us~9900us default = auto exposure mode */
int v4l2_set_exposure(int cam_fd,int val_exposure)
{
	int ret;
	struct v4l2_control ctrl;

	ctrl.id = V4L2_CID_EXPOSURE_AUTO;
	ctrl.value = V4L2_EXPOSURE_MANUAL;//手动曝光模式
	ret = ioctl(cam_fd, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0)
	{
		printf("Get exposure auto Type failed\n");
		return V4L2_UTILS_SET_EXPSURE_AUTO_TYPE_ERR;
	}

	//设置曝光绝对值
	ctrl.id = V4L2_CID_EXPOSURE_ABSOLUTE;
	ctrl.value = val_exposure;  //单位100us
	ret = ioctl(cam_fd, VIDIOC_S_CTRL, &ctrl);
	if (ret < 0)
	{
		printf("Set exposure failed (%d)\n", ret);
		return V4L2_UTILS_SET_EXPSURE_ERR;
	}
	
	printf("set exposure ok\n");
	return 0;
}

/* set camera control parameters value */
int v4l2_set_ctrl_value(int cam_fd,int V4L2_cid,int V4L2_value)
{
	struct v4l2_control ctrl;
	 ctrl.id= V4L2_cid;
	 ctrl.value = V4L2_value;
	 if(ioctl(cam_fd,VIDIOC_S_CTRL,&ctrl)==-1)
     {
        perror("ioctl VIDIOC_S_CTRL error");
        return V4L2_UTILS_SET_BRIGHTNESS_ERR;
     }

	printf("set VIDIOC_S_CTRL ok\n");
	return 0;
}

/* set v4l2_format(resolution for width * height) */
int v4l2_set_fmtd(int cam_fd,int width,int height)
{
	struct v4l2_format fmt;
	//set default video capture parameters設置默認的攝像頭捕獲參數
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.height = height;
	fmt.fmt.pix.width = width;
	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

	if(ioctl(cam_fd,VIDIOC_S_FMT,&fmt) == -1)
	{
		printf("Unable to set format\n");
		return VIDIOC_S_FMT_ERROR;
	}

	//check having set video capture parameters
	if(ioctl(cam_fd,VIDIOC_G_FMT,&fmt) == -1 ) {
		printf("Unable to get format\n");
		return VIDIOC_G_FMT_ERROR;
	}
	
	printf("set v4l2_format ok\n");
	return 0;
}

/* set v4l2_streamparm for fps */
int v4l2_set_streamparm(int cam_fd,int fps)
{
	int ret;
	//set fps
	struct v4l2_streamparm stream_parm;
	stream_parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	stream_parm.parm.capture.timeperframe.numerator = 1;
	stream_parm.parm.capture.timeperframe.denominator = fps;
	ret = ioctl(cam_fd,VIDIOC_S_PARM,&stream_parm);
	if(ret < 0) {
		printf("Unable to set stream_parm\n");
		return VIDIOC_S_PARM_ERROR;
	}
	
	//check fps
	if(ioctl(cam_fd,VIDIOC_G_PARM,&stream_parm) == -1 ) {
		printf("Unable to get stream_parm\n");
		return VIDIOC_G_PARM_ERROR;
	}

	printf("set v4l2_streamparm ok\n");
	
	return 0;
} 

int v4l2_init_camera(void)
{
	struct v4l2_capability cap;
	struct v4l2_fmtdesc fmtdesc;//视频流相关
	int ret;

	//get all camera device info
	ret = v4l2_get_camera_device_info();
	if(ret < 0) {
		printf("get all camera device error\n ");
		return CAMERA_GET_DEVICE_INFO_ERROE;
	}
    printf("get camera devoce info success\n");

    for(int i = 0; i < client_all_info.camera_num; i++) {
		//1.open all video device
        cam_FdList[i] = open((char*)client_all_info.camera_device_info[i].camera_device_name,O_RDWR);
		if(cam_FdList[i] == -1) {
            printf("open %s failed;err info = %s\n",client_all_info.camera_device_info[i].camera_device_name,strerror(errno));
			return CAMERA_OPEN_ERROR;
        }
        printf("camera fd is %d\n",cam_FdList[i]);

        //2.get the video info
        ret = ioctl(cam_FdList[i],VIDIOC_QUERYCAP,&cap);
        if(ret == -1) {
            printf("VIDIOC_QUERYCAP failed\n");
            return VIDIOC_QUERYCAP_ERROR;
        }

        //print video capabilities

        printf("driver:\t\t%s\n",cap.driver);
        printf("card:\t\t%s\n",cap.card);
        printf("bus_info:\t\t%s\n",cap.bus_info);
        printf("version:\t\t%d\n",cap.version);
        printf("capabilities:\t\t%x\n",cap.capabilities);
        if((cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) == V4L2_CAP_VIDEO_CAPTURE) {
            printf("Device %s : supports capture.\n",client_all_info.camera_device_info[i].camera_device_name);
        }

        if((cap.capabilities & V4L2_CAP_STREAMING) == V4L2_CAP_STREAMING) {
            printf("Device %s:supports streaming.\n",client_all_info.camera_device_info[i].camera_device_name);
        }

        //3.get vide0 pixel format
        fmtdesc.index = 0;
        fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        printf("Supports format:\n");
        while(ioctl(cam_FdList[i],VIDIOC_ENUM_FMT,&fmtdesc) != -1) {
            printf("\t%d.%s\n",fmtdesc.index + 1,fmtdesc.description);
            fmtdesc.index++;
        }

        //set resolution
        ret = v4l2_set_fmtd(cam_FdList[i],IMAGEWIDTH,IMAGEHEIGHT);
        if(ret < 0) {
            printf("set v4l2_format failed\n");
            return ret;
        }
        client_all_info.camera_device_info[i].camera_paras_info.width = IMAGEWIDTH;
        client_all_info.camera_device_info[i].camera_paras_info.height = IMAGEHEIGHT;

        //set fps
        ret = v4l2_set_streamparm(cam_FdList[i],FPS);
        if(ret < 0) {
            printf("set v4l2_streamparm failed\n");
            return ret;
        }
        client_all_info.camera_device_info[i].camera_paras_info.fps = FPS;

        client_all_info.camera_device_info->camera_paras_info.video_coding_fmt = VIDEO_CODING_FORMAT_H264;

        //摄像头参数默认数值设置
        v4l2_set_exposure(cam_FdList[i],EXPOSURE_DEFULT);               //曝光
        v4l2_set_white_balance(cam_FdList[i],WHITE_BALANCE_DEFULT);     //白平衡
        v4l2_set_ctrl_value(cam_FdList[i],V4L2_CID_BRIGHTNESS,BRIGHTNESS_DEFULT);      //亮度
        v4l2_set_ctrl_value(cam_FdList[i],V4L2_CID_CONTRAST,CONTRAST_DEFULT);        //对比度
        v4l2_set_ctrl_value(cam_FdList[i],V4L2_CID_SATURATION,SATURATION_DEFULT);      //饱和度
        v4l2_set_ctrl_value(cam_FdList[i],V4L2_CID_SHARPNESS,SHARPNESS_DEFULT);      //锐度

         //get video contrl info
        client_all_info.camera_device_info[i].camera_paras_info.camera_ctrl_paras.val_exposure = EXPOSURE_DEFULT;
        client_all_info.camera_device_info[i].camera_paras_info.camera_ctrl_paras.val_white_balance_temp = v4l2_get_ctrl_value(cam_FdList[i],V4L2_CID_WHITE_BALANCE_TEMPERATURE);
        client_all_info.camera_device_info[i].camera_paras_info.camera_ctrl_paras.val_brightness = v4l2_get_ctrl_value(cam_FdList[i],V4L2_CID_BRIGHTNESS);
        client_all_info.camera_device_info[i].camera_paras_info.camera_ctrl_paras.val_contrast = v4l2_get_ctrl_value(cam_FdList[i],V4L2_CID_CONTRAST);
        client_all_info.camera_device_info[i].camera_paras_info.camera_ctrl_paras.val_saturation = v4l2_get_ctrl_value(cam_FdList[i],V4L2_CID_SATURATION);
        client_all_info.camera_device_info[i].camera_paras_info.camera_ctrl_paras.val_sharpness = v4l2_get_ctrl_value(cam_FdList[i],V4L2_CID_SHARPNESS);
    }//end for
    return 0;
}

/* reqest frame buffer ;mmp into user space;put into output queue for a camera device
*  cam_fd:camera device fd
*  camera_index:camera device index  video0 video1  ...
*/
int v4l2_camera_reqbuff(int cam_fd,int camera_index,int frame_count)
{
    struct v4l2_requestbuffers reqBuffer;   //request frame buffer to kernel 内核缓存区
    struct v4l2_buffer buf[frame_count];          //用户缓存区 a struct v4l2_buffer meomery is the same to kernel buffer

    //1.request kernel buffers(the number of frames is buf,this program is 5)申请帧缓存区
    reqBuffer.count = frame_count;
	reqBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	reqBuffer.memory = V4L2_MEMORY_MMAP;
	if(ioctl(cam_fd,VIDIOC_REQBUFS,&reqBuffer) == -1) {
		printf("request for buffers failed\n");
		return VIDIOC_REQBUFS_ERROR;
	}
	printf("request buffers frame success!!\n");

    //2.get memory kernel address and length;
	buffers[camera_index] =(struct VideoBuffer*) malloc(reqBuffer.count * sizeof(struct VideoBuffer));
	if(!buffers[camera_index]) {
		printf("malloc memory error\n");
		return MALLOC_ERROR;
	}

    for(int i = 0;i < (int)reqBuffer.count;i++) {
		buf[i].type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf[i].memory = V4L2_MEMORY_MMAP;
		buf[i].index = i;

        //获取内核缓存区地址
		if(ioctl(cam_fd,VIDIOC_QUERYBUF,&buf[i]) == -1) {
			printf("query buffer error\n");
			return VIDIOC_QUERYBUF_ERROR;
		}
        //3.get buffers length;change to address;buf[COUNT]'s memory space is map to buffers[i]
        //mmap函数转换成应用程序中的绝对地址
		buffers[camera_index][i].length = buf[i].length;
		buffers[camera_index][i].start = mmap(NULL,buf[i].length,PROT_READ | PROT_WRITE,MAP_SHARED,cam_fd,buf[i].m.offset);
		
		if(buffers[camera_index][i].start == MAP_FAILED) {
			printf("buffer map error\n");
			return MMP_ERROR;
		}
		printf("Frame buffer %d   address = %p ,length = %ld \n",reqBuffer.count,buffers[camera_index][i].start,buffers[camera_index][i].length);

        //4.put into buffer queue 放入视频采集输出队列
		if(ioctl(cam_fd,VIDIOC_QBUF,&buf[i]) == -1) {
			printf("VIDIOC_QBUF put frame buffers into buffer queue\n");
			return VIDIOC_QBUF_ERROR;
		};
	}

	printf("request buffer ok for cemera %d\n",camera_index);
	return 0;
}

/* start video stream data captrue
*  FILE* file:save video yuv file
*  cam_fd:camera device fd
*  camera_index:camera device index  video0 video1  ...
*/
int v4l2_start_camera(FILE* file,int cam_fd,int camera_index,int frame_count)
{
    // 1.start cap video stream 开启视频流数据的采集
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	if(ioctl(cam_fd,VIDIOC_STREAMON,&type) == -1) {
		printf("start cap video stream failed\n");
		return VIDIOC_STREAMON_ERROR;
	}
	printf("cap vidieo stream success!\n");
  
	struct v4l2_buffer v4lbuf;
    memset(&v4lbuf,0, sizeof(v4lbuf));
    v4lbuf.type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4lbuf.memory= V4L2_MEMORY_MMAP;
    v4lbuf.index = number_frame % frame_count;
	printf("index = %d\n",v4lbuf.index);

    //2.get out frame buffer data from queue c从输出队列中取出已经含有采集视频数据的帧缓冲区
	if(ioctl(cam_fd,VIDIOC_DQBUF,&v4lbuf) == -1) {
		printf("VIDIOC_DQBUF get out frame buffer data from queue failed\n");

		return VIDIOC_DQBUF_ERROR;
	};
	printf("GRAB YUYV ok\n");

    //3.save yuyv image file保存原始的yuv数据  依次处理5个缓存区
	fwrite(buffers[camera_index][v4lbuf.index].start,buffers[camera_index][v4lbuf.index].length,1,file);
	printf("save yuyv ok\n");

    //4.将该帧缓存区重新排入输出队列 可以进行循环采集
	if (ioctl(cam_fd, VIDIOC_QBUF, &v4lbuf) == -1) {
		printf("VIDIOC_QBUF error\n");
		return VIDIOC_QBUF_ERROR;
	}
  	number_frame ++;
    printf("number_frame = %d\n",number_frame);//打印采集图像总帧数
  	return 0;
}


//停止视频采集
int v4l2_stop_camera(int cam_fd,int camera_index,int frame_count)
{
	//unmap
    for (int i = 0; i < frame_count; i++)
	{
		if (NULL != buffers[camera_index][i].start)
		{
			if (-1 == munmap(buffers[camera_index][i].start, buffers[camera_index][i].length))
			{
				printf("unmap failed\n");
				return UMMMP_ERROR;
			}
			buffers[camera_index][i].start = NULL;
		}
	}

	//stop camera and close fd
	enum v4l2_buf_type type;
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	ioctl(cam_fd,VIDIOC_STREAMOFF,&type);
	if(cam_fd != -1) {
		close(cam_fd);
		return 0;
	}
	else
		return VIDIOC_STREAMOFF_ERROR;
}
