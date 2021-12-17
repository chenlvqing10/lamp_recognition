#ifndef PROTOCOL_CJSON_H
#define PROTOCOL_CJSON_H
#include <QObject>

#ifdef __cplusplus
extern "C" {
#include "servers/camera/v4l2_camera.h"
#include "base/include/cJSON.h"
#include "servers/serial/ec20_serial.h"
#include "base/include/delay.h"
}
#endif


#define CMD_CLIENT_CONN   "client_conn"


class PROTOCOL_CJSON
{
public:
    PROTOCOL_CJSON();
public:
    char* create_clientinfo_json(client_all_info_t* client_all_info);
};

#endif // PROTOCOL_CJSON_H
