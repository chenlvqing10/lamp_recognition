#include <stdio.h>
#include <stdlib.h>
#include "ec20_serial.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h> //for set serial
#include <string.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>
#include "base/include/delay.h"
#include <pthread.h>

#ifndef CMSPAR
#define CMSPAR   010000000000
#endif


int fd_ec20;
static char cops_info[30];
static at_qicsgp_t csgp_info;

int serial_open(const char* device_name)
{
    int fd = open(device_name,O_RDWR | O_NOCTTY);
    if(fd <0) {
        printf("open device name %s failed\n",device_name);
        return -1;
    }

    return fd;
}

/*
 * convert baud_rate to speed_t
 */
static speed_t  get_speed(unsigned int baud_rate)
{
    switch (baud_rate) {
        case 0:
            return B0;
        case 50:
            return B50;
        case 75:
            return B75;
        case 110:
            return B110;
        case 150:
            return B150;
        case 200:
            return B200;
        case 300:
            return B300;
        case 600:
            return B600;
        case 1200:
            return B1200;
        case 1800:
            return B1800;
        case 2400:
            return B2400;
        case 4800:
            return B4800;
        case 9600:
            return B9600;
        case 19200:
            return B19200;
        case 38400:
            return B38400;
        case 57600:
            return B57600;
        case 115200:
            return B115200;
        case 230400:
            return B230400;
        case 460800:
            return B460800;
        case 500000:
            return B500000;
        case 576000:
            return B576000;
        case 921600:
            return B921600;
        case 1000000:
            return B1000000;
        case 1152000:
            return B1152000;
        case 1500000:
            return B1500000;
        case 2000000:
            return B2000000;
        case 2500000:
            return B2500000;
        case 3000000:
            return B3000000;
        case 3500000:
            return B3500000;
        case 4000000:
            return B4000000;
        default:
            //unsupported baud rate
            return 0;
    }
}

/**
 * set baud rate of  serial port
 *
 * @param file_descriptor file descriptor of serial device
 * @param baud_rate baud rate
 *
 * @return 1 if success
 *  return negative error code if fail
 */
int serial_set_baud_rate(int file_descriptor, int baud_rate) {
    struct termios termio;
    speed_t speed;
    int fd = file_descriptor;

    if ( fd < 0 ) {
        return SERIAL_INVALID_FILE;
    }

    memset(&termio, 0, sizeof(termio));

    //get old attribute
    if (tcgetattr(fd, &termio)) {
        return SERIAL_INVALID_RESOURCE;
    }

    //calculate baud rate
    speed = get_speed(baud_rate);
    if (speed == 0) {
        return SERIAL_ERROR_BAUDRATE;
    }
    cfsetispeed(&termio, speed);
    cfsetospeed(&termio, speed);

    // set baud rate
    if (tcsetattr(fd, TCSAFLUSH, &termio) < 0) {
        return SERIAL_ERROR_BAUDRATE;
    }

    return 1;
}

/**
 * set serial attributes
 *
 * @param file_descriptor file descriptor of serial device
 * @param flow_ctrl
 * @param data_bits how many bits per data, could be 7, 8, 5
 * @param parity which parity method, could be PARITY_NONE, PARITY_ODD, PARITY_EVEN
 * @param stop_bits  how many stop bits
 *
 * @return 1 if success.
 *  return negative error code if fail.
 */
int serial_set_attr(int file_descriptor, int data_bits, char parity, int stop_bits, int flow_ctrl)
{
    struct termios termio;
    int fd = file_descriptor;

    if ( fd < 0 ) {
        return SERIAL_INVALID_FILE;
    }

    memset(&termio, 0, sizeof(termio));

    if (tcgetattr(fd, &termio)) {
        return SERIAL_INVALID_RESOURCE;
    }

    //set flow control
    switch(flow_ctrl) {
        case FLOW_CONTROL_NONE :
            termio.c_cflag &= ~CRTSCTS;
            break;
        case FLOW_CONTROL_HARDWARE :
            termio.c_cflag |= CRTSCTS;
            break;
        case FLOW_CONTROL_SOFTWARE :
            termio.c_cflag |= IXON | IXOFF | IXANY;
            break;
    }

    //set data bit
    termio.c_cflag &= ~CSIZE;
    switch (data_bits) {
        case 8:
            termio.c_cflag |= CS8;
            break;
        case 7:
            termio.c_cflag |= CS7;
            break;
        case 6:
            termio.c_cflag |= CS6;
            break;
        case 5:
            termio.c_cflag |= CS5;
            break;
        default:
            termio.c_cflag |= CS8;
            break;
    }

    //set stop bits
    switch (stop_bits) {
        case 1:
            termio.c_cflag &= ~CSTOPB;
            break;
        case 2:
            termio.c_cflag |= CSTOPB;
            break;
        default:
            break;
    }

    //set parity bit
    switch (parity) {
        case PARITY_NONE:
            termio.c_cflag &= ~(PARENB | PARODD);
            break;
        case PARITY_EVEN:
            termio.c_cflag |= PARENB;
            termio.c_cflag &= ~PARODD;
            break;
        case PARITY_ODD:
            termio.c_cflag |= PARENB | PARODD;
            break;
        case PARITY_MARK:
            termio.c_cflag |= PARENB | CMSPAR | PARODD;
            break;
        case PARITY_SPACE:
            termio.c_cflag |= PARENB | CMSPAR;
            termio.c_cflag &= ~PARODD;
            break;
    }

    if (tcsetattr(fd, TCSAFLUSH, &termio) < 0) {
        return SERIAL_ERROR_SETTING;
    }

    return 1;
}

/**
 * close serial port
 *
 * @param file_descriptor file descriptor of serial port device file
 *
 * @return 0 if success.
 *  return -1 if failed, and errno is set.
 */
int serial_close(int file_descriptor) {
    return close(file_descriptor);
}


/**
 * The function waits until all queued output to the terminal filedes has been transmitted.
 *
 * tcdrain() is called.

 * @param file_descriptor file descriptor of serial port device file
 *
 * @return 0 if success.
 *  return -1 if fail, and errno is set.
 */
int serial_flush(int file_descriptor)
{
    return tcdrain(file_descriptor);
}


/**
 * whether data is available
 *
 * @param file_descriptor file descriptor of serial port device file
 * @param timeout_millisec timeout in millisecond
 *
 * @return 1 if data is available
 *  return 0 if data is not available
 */
int serial_data_available(int file_descriptor, unsigned int timeout_millisec)
{
    int fd = file_descriptor;
    struct timeval timeout;
    fd_set readfds;

    if (fd < 0) {
        return SERIAL_INVALID_FILE;
    }

    if (timeout_millisec == 0) {
        // no waiting
        timeout.tv_sec = 0;
        timeout.tv_usec = 0;
    } else {
        timeout.tv_sec = timeout_millisec / 1000;
        timeout.tv_usec = (timeout_millisec % 1000) * 1000;
    }

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    if (select(fd + 1, &readfds, NULL, NULL, &timeout) > 0) {
        return 1; // data is ready
    } else {
        return 0; // no data
    }
}

/**
 * send data
 *
 * @param file_descriptor file descriptor of serial port device file
 * @param buffer   buffer of data to send
 * @param data_len data length
 *
 * @return positive integer of bytes sent if success.
 *  return -1 if failed.
 */
int serial_send(int file_descriptor, char *buffer, size_t data_len)
{
    size_t len = 0;

    len = write(file_descriptor, buffer, data_len);
    if ( len == data_len ) {
        return len;
    } else {
        tcflush(file_descriptor, TCOFLUSH);
        return -1;
    }

}


/**
 * receive data
 *
 * @param file_descriptor file descriptor of serial port device file
 * @param buffer   buffer to receive data
 * @param data_len max data length
 *
 * @return positive integer of bytes received if success.
 *  return -1 if failed.
 */
int serial_receive(int file_descriptor, char *buffer,size_t data_len)
{
    int len,fs_sel;
    fd_set fs_read;
    struct timeval time;
    int fd = file_descriptor;

    FD_ZERO(&fs_read);
    FD_SET(fd, &fs_read);

    time.tv_sec = 10;
    time.tv_usec = 0;

    //use select() to achieve multiple channel communication
    fs_sel = select(fd + 1, &fs_read, NULL, NULL, &time);
    if ( fs_sel ) {
        len = read(fd, buffer, data_len);
        return len;
    } else {
        return -1;
    }
}


/* send AT Command and recieve AT Command funtion */
int AT_Data_Send_Recv(int fd,char* send_buff,size_t send_len,char* recv_buff,size_t recv_len,int timeout_ms)
{
    int ret;
    printf("----------------------send AT COMMAND ----------------------------------\n");
    printf("send_buff  = %s\n",send_buff);
    ret = serial_send(fd,send_buff,send_len);
    if(ret < 0) {
        printf("send failed\n");
        return AT_SEND_ERROR;
    }
    printf("send ok\n");
    //delayms(500);

    if(serial_data_available(fd,timeout_ms)) {//have a data
        ret= serial_receive(fd,recv_buff,recv_len);
        if(ret < 0) {
            printf("recv failed\n");
            return AT_RECV_ERROR;
        }
    }
    else { //no respond
        return AT_NO_RESPOND;
    }
    printf("recv ok!  ");
    printf("----------------------receive AT Respond ----------------------------------\n");
    printf("recv_buff = %s len = %ld\n",recv_buff,strlen(recv_buff));
    printf("--------------------------------------------------------- ----------------------------------\n");
    return 0;
}

/* get SIM card Operator */
int AT_get_COPS(int fd)
{
    int ret;
    char send_buff[50] = {0};
    int timeout_ms = 20000;//20s
    char recv_buff[50] = {0};
    memset(cops_info,0,sizeof(cops_info));

    //send data and receive data
    strcpy(send_buff,"AT+COPS?\r\n");
    ret = AT_Data_Send_Recv(fd,send_buff,strlen(send_buff),recv_buff,sizeof(recv_buff),timeout_ms);
    if(ret < 0) {
        return ret;
    }

    //analysis the recv buffer;get Operator info
    char* ptr = recv_buff;
    int count = 0;
    int len = strlen("\r\n+COPS: 0,0,\"");
    if((int)strlen(recv_buff) < len) {
        printf("AT+COPS? command error\n");
        ptr = NULL;
        free(ptr);
        return AT_LENGTH_ERROR;
    }
    else
        ptr += len;

    while(*ptr != '\0') {
        cops_info[count++] = *(ptr++);
        if(*ptr == '\"')
            break;
    }
    printf("cops_info = %s\n",cops_info);
    ptr = NULL;
    free(ptr);

    return 0;
}

int AT_set_QICSGP(int fd,int contextID)
{
    int ret;
    char send_buff[50] = {0};
    int timeout_ms = 150000;//150s
    char recv_buff[50] = {0};

    //send data and receive data
    sprintf(send_buff,"AT+QICSGP=%d\r\n",contextID);
    ret = AT_Data_Send_Recv(fd,send_buff,strlen(send_buff),recv_buff,sizeof(recv_buff),timeout_ms);
    if(ret < 0) {
        return ret;
    }

    //analysis the recv buffer;get context_type and APN
    memset(&csgp_info,0,sizeof(at_qicsgp_t));
    csgp_info.context_id = contextID;
    char* ptr = recv_buff;
    int count = 0;
    if(strlen(recv_buff) < strlen("\r\n+QICSGP: ")) {
        printf("QICSGP command error\n");
        ptr = NULL;
        free(ptr);
        return AT_LENGTH_ERROR;
    }
    else
        ptr += strlen("\r\n+QICSGP: ");
    csgp_info.context_type = *ptr - '0';
    printf("context_type = %d\n",csgp_info.context_type);

    ptr += strlen("1,\"");
    while(*ptr != '\0') {
        csgp_info.APN[count++] = *(ptr++);
        if(*ptr == '\"')
            break;
    }
    csgp_info.APN[count] = '\0';
    printf("APN = %s\n",csgp_info.APN);

    ptr = NULL;
    free(ptr);
    printf("analysis ok\n");

    //if APN = "" then compare to Network Operator
    if(csgp_info.APN[0] == '\"') {
        printf("contextID %d's APN is empty\n",contextID);
        if(!strcmp(cops_info,OPERATOR_CM))
            strcpy((char*)csgp_info.APN,"CMNET");

        if(!strcmp(cops_info,OPERATOR_CT))
            strcpy((char*)csgp_info.APN,"CHN_CT");

        if(!strcmp(cops_info,OPERATOR_CT))
            strcpy((char*)csgp_info.APN,"CHN_UNICOM");
    }

    //write contextID context_type APN
    memset(send_buff,0,sizeof(send_buff));
    memset(recv_buff,0,sizeof(recv_buff));
    sprintf(send_buff,"AT+QICSGP=%d,%d,\"%s\"\r\n",contextID,csgp_info.context_type,csgp_info.APN);
    ret = AT_set_command(fd,send_buff,strlen(send_buff),recv_buff,sizeof(recv_buff),timeout_ms,AT_COMMAND_RESPOND_OK);
    if(ret < 0) {
        return AT_SET_COMMAND_FAILED;
    }

    printf("set QICSGP OK\n");
    return AT_SET_COMMAND_SUCCESS;
}


/* init the EC20 4G module Serial device;and set baut rate,data bits,parity, stop bits,flow control
* return < 0,open ec20 module serial init failed
*/

int serial_init(const char* device_name,int baut_rate,short data_bits,char parity,short stop_bits,short flow_control)
{
    int ret;

    //1.open the ttyUSB2 serial for EC20 4G Module
    int fd = serial_open(device_name);
    if(fd < 0) {
        perror("open ttyUSB2");
        return SERIAL_ERROR_OPEN;
    }
    //printf("open serial %s for EC20 4G module ok\n",device_name);

    //2.set serial proprities
    ret = serial_set_baud_rate(fd,baut_rate);
    if(ret < 0) {
        printf("set baut rate failed\n");
        return SERIAL_ERROR_BAUDRATE;
    }
    //printf("set serial baut rate ok\n");

    //3.set data bits;set stop bit;set flow control;set parity
    ret = serial_set_attr(fd,data_bits,parity,stop_bits,flow_control);
    if(ret < 0) {
        printf("set data bits/stop bit/flow control/parity failed\n");
        return SERIAL_ERROR_SETTING;
    }
    //printf("set serial attr ok\n");


    serial_flush(fd);

    fd_ec20 = fd;
    return 0;
}

/* Function:set AT command of no return parameters to EC20;then compare the res with res_expect
 * Parameters:
 * fd:file descriptor for 4g ec20 serial
 * cmd:AT command
 * cmd len:len of AT command
 * res:AT command respond result
 * res_len:len of AT command respond result
 * timeout:recv data timeout time
 * res_expect:AT command expected respond result
 *Return:error(<0) success(>0)
 * */
int AT_set_command(int fd,char* cmd,size_t cmd_len,char* res,size_t res_len,int timeout_ms,const char* res_expect )
{
    int ret = AT_Data_Send_Recv(fd,cmd,cmd_len,res,res_len,timeout_ms);
    if(ret < 0) {
        printf("send or receive data to serial failed\n");
        return AT_SET_COMMAND_FAILED;
    }

    if(!strcmp(res,res_expect)) {
        printf("compare ok for cmd %s\n",cmd);
        printf("----------------------------------------------------------------------------\n");
        return AT_SET_COMMAND_SUCCESS;
    }
    else {
        printf("compare nok for cmd %s\n",cmd);
        printf("----------------------------------------------------------------------------\n");
        return AT_SET_COMMAND_FAILED;
    }
}

int AT_set_CREG_CGREG(int fd)
{
    //define local variable
    int ret;
    int timeout_ms = 60000;//60s
    char recv_buff[100] = {0};

    //set AT command "AT+CREG=0"
    ret = AT_set_command(fd,"AT+CREG=0\r\n",strlen("AT+CREG=0\r\n"),recv_buff,sizeof(recv_buff),timeout_ms,AT_COMMAND_RESPOND_OK);
    if(ret < 0)
        return AT_SET_COMMAND_FAILED;

    //set AT command "AT+CREG?"
    memset(recv_buff,0,sizeof(recv_buff));
    ret = AT_set_command(fd,"AT+CREG?\r\n",strlen("AT+CREG?\r\n"),recv_buff,sizeof(recv_buff),timeout_ms,AT_COMMAND_RESPOND_CREG);
    if(ret < 0)
        return AT_SET_COMMAND_FAILED;

    //set AT command "AT+CREG=0"
    memset(recv_buff,0,sizeof(recv_buff));
    ret = AT_set_command(fd,"AT+CGREG=0\r\n",strlen("AT+CGREG=0\r\n"),recv_buff,sizeof(recv_buff),timeout_ms,AT_COMMAND_RESPOND_OK);
    if(ret < 0)
        return AT_SET_COMMAND_FAILED;

    //set AT command "AT+CREG?"
    memset(recv_buff,0,sizeof(recv_buff));
    ret = AT_set_command(fd,"AT+CGREG?\r\n",strlen("AT+CGREG?\r\n"),recv_buff,sizeof(recv_buff),timeout_ms,AT_COMMAND_RESPOND_CGREG);
    if(ret < 0)
        return AT_SET_COMMAND_FAILED;

    return AT_SET_COMMAND_SUCCESS;

}

int AT_set_ACT(int fd,int contextID)
{
    //define local variable
    int ret;
    int timeout_ms = 20000;//20s
    char send_buff[50] = {0};
    char recv_buff[100] = {0};

    //set AT command "AT+QIDEACT=<contextID>"
    sprintf(send_buff,"AT+QIDEACT=%d\r\n",contextID);
    ret = AT_set_command(fd,send_buff,strlen(send_buff),recv_buff,sizeof(recv_buff),timeout_ms,AT_COMMAND_RESPOND_OK);
    if(ret < 0)
        return AT_SET_COMMAND_FAILED;

    //set AT command "AT+QIACT=<contextID>"
    memset(send_buff,0,sizeof(send_buff));
    memset(recv_buff,0,sizeof(recv_buff));
    sprintf(send_buff,"AT+QIACT=%d\r\n",contextID);
    ret = AT_set_command(fd,send_buff,strlen(send_buff),recv_buff,sizeof(recv_buff),timeout_ms,AT_COMMAND_RESPOND_OK);
    if(ret < 0)
        return AT_SET_COMMAND_FAILED;

    return AT_SET_COMMAND_SUCCESS;
}

int AT_check_SIM(int fd)
{
    int ret;
    int timeout_ms = 40000;//40s
    char recv_buff[50] = {0};
    ret = AT_set_command(fd,"AT+CPIN?\r\n",strlen("AT+CPIN?\r\n"),recv_buff,sizeof(recv_buff),timeout_ms,AT_COMMAND_RESPOND_SIM);
    //send data and receive data
    if(ret < 0) {
        return AT_NO_SIM;
    }

    return 0;
}

void AT_reboot(int fd)
{
    char recv_buff[50] = {0};
    AT_Data_Send_Recv(fd,"AT+CFUN=1,1\r\n",strlen("AT+CFUN=1,1\r\n"),recv_buff,sizeof(recv_buff),1000);
}



/*
int AT_send_DATA(int fd,int connectID,char* data_hex,size_t data_len)
{
    int ret;
    char send_buff[1000] = {0};
    int timeout_ms = 20000;//20s
    char recv_buff[1000] = {0};

    while(1) {
        memset(send_buff,0,sizeof(send_buff));
        memset(recv_buff,0,sizeof(recv_buff));
        char hex_a[100] = "303132333435";
        sprintf(send_buff,"AT+QISENDEX=%d,\"%s\"\r\n",connectID,hex_a);
        printf("send_buff = %s\n",send_buff);
        AT_Data_Send_Recv(fd,send_buff,strlen(send_buff),recv_buff,sizeof(recv_buff),timeout_ms);
        sleep(4);

        printf("++++recieve data+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
        memset(send_buff,0,sizeof(send_buff));
        memset(recv_buff,0,sizeof(recv_buff));
        sprintf(send_buff,"AT+QIRD=%d,1500\r\n",connectID);
        printf("send_buff = %s\n",send_buff);
        AT_Data_Send_Recv(fd,send_buff,strlen(send_buff),recv_buff,sizeof(recv_buff),timeout_ms);
        if(!memcmp(recv_buff,"\r\n+QIRD: ",strlen("\r\n+QIRD: "))){
            printf("it is same \n");
        }

        if(!strcmp(recv_buff,AT_COMMAND_URC_RECV)) {
            printf("it is same1\n");

        }


        sleep(4);
        printf("---------------------------------------------------------------------------------------\n");
    }
}
*/



int AT_close_socket(int fd)
{
    int ret;
    char recv_buff[100] = {0};
    int timeout_ms = 40000;
    ret = AT_set_command(fd,"AT+QICLOSE=0\r\n",strlen("AT+QICLOSE=0\r\n"),recv_buff,sizeof(recv_buff),timeout_ms,AT_COMMAND_RESPOND_OK);
    if(ret < 0) {
        printf("socket close error\n");
        AT_reboot(fd);
        return AT_QICLOSE_ERROR;
        exit(EXIT_FAILURE);
    }
    return 0;
}


int AT_Command_int(int fd)
{
    int ret;
    char recv_buff[1000] = {0};
    int count =0;

    //2.set ATE0 to EC20 serial 3 times 20s/time
    ret = AT_Data_Send_Recv(fd,"ATE0\r\n",strlen("ATE0\r\n"),recv_buff,sizeof(recv_buff),20000);
    if(ret < 0) {
        printf("send or receive data to serial failed\n");
        return AT_COMMAND_ERROR;
    }
    else
        printf("ATE0 OK\n");


    //3.set AT to EC20 serial 5 times  20s/time if no respond "OK" then close fd_ec20
    do {
        ret = AT_set_command(fd,"AT\r\n",strlen("AT\r\n"),recv_buff,sizeof(recv_buff),20000,AT_COMMAND_RESPOND_OK);
        count ++;
    }while((ret < 0) && (count < 5));

    if(ret < 0) {
        printf("the EC20 is error,Please check or change it\n");
        return AT_COMMAND_ERROR;
    }
    else
        printf("AT OK\n");


    //4.check if having SIM card
    ret = AT_check_SIM(fd);
    if(ret == AT_NO_SIM) {
        printf("Please check if having SIM card\n");
        return AT_NO_SIM;
    }
    else
        printf("SIM Checked OK\n");

    //5. get Operator info
    ret = AT_get_COPS(fd);
    if(ret < 0) {
        printf("get Operator failed\n");
    }
    else {
        printf("get Operator success\n");
    }

    //6.check network
    ret= AT_set_CREG_CGREG(fd);
    if(ret < 0) {
        printf("set CREG and CGREG failed\n ");
        AT_reboot(fd);
        return  AT_NO_NETWORK;
    }
    else
        printf("set CREG and CGREG OK\n ");


    //7.configure context
    count =0;
    do {
        ret = AT_set_QICSGP(fd,CONTEXTID);
        count ++;
    }while((ret < 0) && (count <= 3));
    if(ret  < 0)
    {
        printf("set QICSGP failed,no local network\n");
        AT_reboot(fd);
        return  AT_SET_CONTEXT_ERROR;
    }

    //8.active ACT
    count =0;
    do {
        ret = AT_set_ACT(fd,1);
        count++;
    }while((ret < 0) && (count <= 3));
    if(ret < 0) {
        printf("set ACT failed\n");
        AT_reboot(fd);
        return  AT_ACTIVE_CONTEXT_ERROR;
    }else
         printf("set ACT OK\n");


    return AT_COMMAND_INIT_OK;
}


