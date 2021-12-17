#include "protocol_cjson.h"
#include <QString>
#include <QDebug>

extern int fd_ec20;
PROTOCOL_CJSON::PROTOCOL_CJSON()
{

}

char *PROTOCOL_CJSON::create_clientinfo_json(client_all_info_t *client_all_info)
{
    //打印客户端信息
    printf("client uuid = %s\n",client_all_info->client_info.client_uuid);
    printf("client name = %s\n",client_all_info->client_info.client_name);
    printf("client online = %d\n",client_all_info->client_info.client_online);
    printf("client version = %s\n",client_all_info->client_info.client_version);

    printf("room name = %s\n",client_all_info->room_info.room_name);
    printf("substation name = %s\n",client_all_info->room_info.substation_name);

    printf("camera num = %d\n",client_all_info->camera_num);


    for(int i=0;i<client_all_info->camera_num;i++) {
          printf("camera_num[%d] camera_device_name = %s\n",i,client_all_info->camera_device_info[i].camera_device_name);

          printf("camera_num[%d] camera_device_name = %s\n",i,client_all_info->camera_device_info[i].camera_device_name);
          printf("camera_num[%d] camera_width = %d\n",i,client_all_info->camera_device_info[i].camera_paras_info.width);
          printf("camera_num[%d] camera_height = %d\n",i,client_all_info->camera_device_info[i].camera_paras_info.height);
          printf("camera_num[%d] camera_fps = %d\n",i,client_all_info->camera_device_info[i].camera_paras_info.fps);
          printf("camera_num[%d] camera_video_coding_fmt = %d\n",i,client_all_info->camera_device_info[i].camera_paras_info.video_coding_fmt);

          printf("camera_num[%d] camera_val_white_balance_temp = %d\n",i,client_all_info->camera_device_info[i].camera_paras_info.camera_ctrl_paras.val_white_balance_temp);
          printf("camera_num[%d] camera_val_brightness = %d\n",i,client_all_info->camera_device_info[i].camera_paras_info.camera_ctrl_paras.val_brightness);
          printf("camera_num[%d] camera_val_contrast = %d\n",i,client_all_info->camera_device_info[i].camera_paras_info.camera_ctrl_paras.val_contrast);
          printf("camera_num[%d] camera_val_saturation = %d\n",i,client_all_info->camera_device_info[i].camera_paras_info.camera_ctrl_paras.val_saturation);
          printf("camera_num[%d] camera_val_sharpness = %d\n",i,client_all_info->camera_device_info[i].camera_paras_info.camera_ctrl_paras.val_sharpness);
          printf("camera_num[%d] camera_val_exposure = %d\n",i,client_all_info->camera_device_info[i].camera_paras_info.camera_ctrl_paras.val_exposure);

          printf("camera_num[%d] lamp_device_name = %s\n",i,client_all_info->lamp_device_info->lamp_device_name);

    }


    cJSON* root = cJSON_CreateObject();
    cJSON* client_info = cJSON_CreateObject();
    cJSON* camera_array = cJSON_CreateArray();

    cJSON_AddItemToObject(root,"cmd",cJSON_CreateString(CMD_CLIENT_CONN));
    cJSON_AddItemToObject(root,"client_info",client_info);

    cJSON_AddItemToObject(client_info,"client_uuid",cJSON_CreateString((const char*)client_all_info->client_info.client_uuid));
    cJSON_AddItemToObject(client_info,"client_name",cJSON_CreateString((const char*)client_all_info->client_info.client_name));
    cJSON_AddItemToObject(client_info,"client_online",cJSON_CreateNumber(client_all_info->client_info.client_online));
    cJSON_AddItemToObject(client_info,"client_version",cJSON_CreateString((const char*)client_all_info->client_info.client_version));
    cJSON_AddItemToObject(client_info,"room_name",cJSON_CreateString((const char*)client_all_info->room_info.room_name));
    cJSON_AddItemToObject(client_info,"substation_name",cJSON_CreateString((const char*)client_all_info->room_info.substation_name));
    cJSON_AddItemToObject(client_info,"camera_num",cJSON_CreateNumber(client_all_info->camera_num));
    
    cJSON_AddItemToObject(client_info,"camera_list",camera_array);
    for(int i=0;i<client_all_info->camera_num;i++) {
        cJSON* array_obj = cJSON_CreateObject();
        cJSON* camera_parms = cJSON_CreateObject();
        cJSON_AddItemToArray(camera_array,array_obj);

        char id[10] = {0};
        sprintf(id,"100%d",i+1);
        cJSON_AddItemToObject(array_obj,"camera_id",cJSON_CreateString(id));
        cJSON_AddItemToObject(array_obj,"camera_device_name",cJSON_CreateString((const char*)client_all_info->camera_device_info[i].camera_device_name));
        cJSON_AddItemToObject(array_obj,"lamp_id",cJSON_CreateString(id));
        cJSON_AddItemToObject(array_obj,"lamp_device_name",cJSON_CreateString((const char*)client_all_info->lamp_device_info[i].lamp_device_name));

        cJSON_AddItemToObject(array_obj,"camera_parms",camera_parms);
        cJSON_AddItemToObject(camera_parms,"width",cJSON_CreateNumber(client_all_info->camera_device_info[i].camera_paras_info.width));
        cJSON_AddItemToObject(camera_parms,"height",cJSON_CreateNumber(client_all_info->camera_device_info[i].camera_paras_info.height));
        cJSON_AddItemToObject(camera_parms,"fps",cJSON_CreateNumber(client_all_info->camera_device_info[i].camera_paras_info.fps));
        cJSON_AddItemToObject(camera_parms,"white_balance_temp",cJSON_CreateNumber(client_all_info->camera_device_info[i].camera_paras_info.camera_ctrl_paras.val_white_balance_temp));
        cJSON_AddItemToObject(camera_parms,"brightness",cJSON_CreateNumber(client_all_info->camera_device_info[i].camera_paras_info.camera_ctrl_paras.val_brightness));
        cJSON_AddItemToObject(camera_parms,"contrast",cJSON_CreateNumber(client_all_info->camera_device_info[i].camera_paras_info.camera_ctrl_paras.val_contrast));
        cJSON_AddItemToObject(camera_parms,"saturation",cJSON_CreateNumber(client_all_info->camera_device_info[i].camera_paras_info.camera_ctrl_paras.val_saturation));
        cJSON_AddItemToObject(camera_parms,"sharpness",cJSON_CreateNumber(client_all_info->camera_device_info[i].camera_paras_info.camera_ctrl_paras.val_sharpness));
        cJSON_AddItemToObject(camera_parms,"exposure",cJSON_CreateNumber(client_all_info->camera_device_info[i].camera_paras_info.camera_ctrl_paras.val_exposure));
        cJSON_AddItemToObject(camera_parms,"video_coding_fmt",cJSON_CreateNumber(client_all_info->camera_device_info[i].camera_paras_info.video_coding_fmt));


    }

    char* temp = cJSON_Print(root);

    printf("data of json = %s\n",temp);

    QString str(temp);
    qDebug() << "str:" << str <<endl;

    //转换为16进制
    QByteArray sendData_hex = str.toLatin1();
    qDebug() << "sendData_hex = " << sendData_hex.toHex() << endl;
    qDebug() << "sendData_hex sizd = " << sendData_hex.size() << endl;
    char send_buff[20000] = {0};
    char recv_buff[20000] = {0};
    int timeout_ms = 20000;//20s
    char send_data[20000] = {0};
    strcpy(send_data,"7b0a0922636d64223a0922636c69656e745f636f6e6e222c0a0922636c69656e745f696e666f223a097b0a090922636c69656e745f75756964223a092234366565356437312d333333352d343534372d384c414d505f5245434f474e4954494f4e5f434c\
49454e5431222c0a090922636c69656e745f6e616d65223a09\
224c414d505f5245434f474e4954494f4e5f434c49454e5431\
222c0a090922636c69656e745f6f6e6c696e65223a09312c0a\
090922636c69656e745f76657273696f6e223a0922765f636c");
    sprintf(send_buff,"AT+QISENDEX=%d,\"%s\"\r\n",0,send_data);
    printf("send_buff = %s\n",send_buff);

    int ret = serial_send(fd_ec20,send_buff,strlen(send_buff));
    if(ret < 0) {
        printf("send failed\n");
    }
    printf("send ok\n");
    delayms(10);

    if(serial_data_available(fd_ec20,timeout_ms)) {//have a data
        ret= serial_receive(fd_ec20,recv_buff,sizeof(recv_buff));
        if(ret < 0) {
            printf("recv failed\n");
        }
    }

    return temp;
}
