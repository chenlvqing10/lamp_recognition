#include "videocapturethread.h"
#include "base/include/thread_base.h"
#include <QDebug>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/core.hpp>
#include <QDir>

#ifdef __cplusplus
extern "C" {
#include "servers/camera/v4l2_camera.h"
#include "base/include/base64.h"
}
#endif


using namespace std;
using namespace cv;

static int _index;
extern taskthread_info_t taskthreadInfo[MAXSERVERNUMBER];
extern client_all_info_t client_all_info;//客户端信息
extern int cam_FdList[MAXVIDEONUM];   //all camera device fd
extern int number_frame;                //all the video frame
VideoCaptureThread::VideoCaptureThread(QObject *parent) : QThread(parent)
{

}

void VideoCaptureThread::run()
{
    QMutex mutex;
    while(taskthreadInfo[_index].videocaptureThread_runable) {
       mutex.lock();
       taskthreadInfo[_index].videocaptureThreadID = this->currentThreadId();
       qDebug() << " start video capture thread   " << "thread_id::" << this->currentThreadId() << endl;


       const int framesize = IMAGEHEIGHT * IMAGEWIDTH * 2;
       for(int i = 0;i < (int)client_all_info.camera_num;i++) {
           unsigned char* pYuvBuf = new unsigned char[framesize]; //一帧数据大小

           //create a image file of yuv format,no data
           QString path = QDir::currentPath();
           QByteArray bi = path.toLatin1();
           char* img_yuv = new char[100];
           sprintf(img_yuv,"%s/image/camera%d_img_yuv.yuv",bi.data(),number_frame % 5);
           printf("img_yuv:%s\n",img_yuv);
           FILE* file_yuv = fopen(img_yuv,"wb");
           if(!file_yuv) {
               printf("open YUY2IMG failed\n");
               exit(-1);
           }
           else {
               printf("create yuyv file success!\n");
           }

           //start camera and wrrite data to the yuv image,one frame
           int ret = v4l2_start_camera(file_yuv,cam_FdList[i],i,5);
           if(ret < 0) {
               printf("cap image failed\n");
           }
           fclose(file_yuv);

           //open the yuv image ;read data to pYuvBuf
           FILE* fileIn = fopen(img_yuv, "rb+");
           if(!fileIn) {
               printf("open YUY2IMG failed\n");
               exit(-1);
           }
           else {
               printf("open yuyv file success!\n");
           }
           fread(pYuvBuf, framesize*sizeof(unsigned char), 1, fileIn);
           fclose(fileIn);

           //yuv to rgb
           printf("length = %ld\n",strlen((char*)pYuvBuf));
           cv::Mat yuvImg;
           cv::Mat rgbImg(IMAGEHEIGHT, IMAGEWIDTH, CV_8UC3);
           yuvImg.create(IMAGEHEIGHT, IMAGEWIDTH, CV_8UC2);
           memcpy(yuvImg.data, pYuvBuf, framesize);
           cv::cvtColor(yuvImg, rgbImg, COLOR_YUV2BGR_YUYV);

           /*save a image of jpg format */
              vector<int> compression_params;
           compression_params.push_back( IMWRITE_JPEG_QUALITY);
           compression_params.push_back(100);
           char* img_jpg = new char[100];
           sprintf(img_jpg,"%s/image/camera%d_img_jpg.jpg",bi.data(),number_frame % 5);
           imwrite(img_jpg,rgbImg,compression_params);

           /* change to base64 */
            Mat imgResize;
           cv::resize(rgbImg,imgResize,Size(),0.5,0.5);
           char* img_resize = new char[100];
           sprintf(img_resize,"%s/image/camera%d_imgResize_jpg.jpg",bi.data(),number_frame % 5);
           imwrite(img_resize,imgResize,compression_params);

           char* jpg_base64 = NULL;
           jpg_base64 = file2base(img_resize);
           printf("lens of jpg_base6i4 = %ld\n",strlen(jpg_base64));

           //show is error because of opencv and qt all use gtk lib ,will have a conflict
           //cv::imshow("img_resize",imgResize);

           //free
           free(pYuvBuf);
           free(img_yuv);
           free(img_jpg);
           free(img_resize);
           yuvImg.release();
           rgbImg.release();
           printf("-----------------------------------------------------------------------------------------------------------------\n");
           //waitKey(10);
       }//end for



       mutex.unlock();
   }

   qDebug() << "end video Capture thread" << endl;
}

void VideoCaptureThread::stop(int server_index)
{
   taskthreadInfo[server_index].videocaptureThread_runable = false;
}

void VideoCaptureThread::slot_videoCapture(int server_index)
{
    _index = server_index;
}
