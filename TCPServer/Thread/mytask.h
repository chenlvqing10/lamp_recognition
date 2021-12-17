#ifndef MYTASK_H
#define MYTASK_H

#include <QObject>
#include <QDebug>
#include <QRunnable>
#include "base/serverbase.h"
#include "base/eventbase.h"
#include "sql/sqlbase.h"

#ifdef __cplusplus
extern "C" {
#include "base/cJSON.h"
}
#endif


//QRunnable类 业务逻辑分离
class myTask : public QObject,public QRunnable
{
    Q_OBJECT
public:
    myTask();
signals:
    void Result(taskThread_Result_info_t*);
protected:
    void run();
private:
    void dataAnalysis(cJSON* json,int client_index);
    int get_client_all_info(cJSON *json, int client_index);
    void set_client_all_info_database(dbConnect_info_t* m_dbconnect_info,client_all_info_t* client_all_info);
    void  get_video_path(cJSON *json, int client_index);
    void get_imgcap(cJSON *json, int client_index);
};

#endif // MYTASK_H
