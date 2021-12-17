QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    base/src/base64.c \
    base/src/cJSON.c \
    base/src/delay.c \
    base/src/file_ops.c \
    base/src/thread_base.cpp \
    main.cpp \
    protocol/protocol_cjson.cpp \
    qt_ui/ui_lamp_mainwindow.cpp \
    servers/Thread/communicationthread.cpp \
    servers/Thread/connecttoserverthread.cpp \
    servers/Thread/taskthread.cpp \
    servers/Thread/videocapturethread.cpp \
    servers/camera/v4l2_camera.c \
    servers/serial/ec20_serial.c

HEADERS += \
    base/include/base64.h \
    base/include/cJSON.h \
    base/include/delay.h \
    base/include/file_ops.h \
    base/include/thread_base.h \
    protocol/protocol_cjson.h \
    qt_ui/ui_lamp_mainwindow.h \
    servers/Thread/communicationthread.h \
    servers/Thread/connecttoserverthread.h \
    servers/Thread/taskthread.h \
    servers/Thread/videocapturethread.h \
    servers/camera/v4l2_camera.h \
    servers/serial/ec20_serial.h



FORMS += \
    qt_ui/ui_lamp_mainwindow.ui \

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += /usr/local/opencv_5.15.1_arm64/include/
LIBS += -L/usr/local/opencv_5.15.1_arm64/lib -lopencv_core
LIBS += -L/usr/local/opencv_5.15.1_arm64/lib -lopencv_imgcodecs
LIBS += -L/usr/local/opencv_5.15.1_arm64/lib -lopencv_imgproc
LIBS += -L/usr/local/opencv_5.15.1_arm64/lib -lopencv_highgui
LIBS += -L/usr/local/opencv_5.15.1_arm64/lib -lopencv_features2d
LIBS += -L/usr/local/opencv_5.15.1_arm64/lib -lopencv_xfeatures2d


