#ifndef VIDEOCAPTURETHREAD_H
#define VIDEOCAPTURETHREAD_H

#include <QObject>
#include <QThread>


class VideoCaptureThread : public QThread
{
    Q_OBJECT
public:
    explicit VideoCaptureThread(QObject *parent = 0);
    virtual void run();
    void stop(int server_index);

public slots:
    void slot_videoCapture(int);


};

#endif // VIDEOCAPTURETHREAD_H
