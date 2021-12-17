QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Thread/myclient.cpp \
    Thread/myserver.cpp \
    Thread/mytask.cpp \
    base/Qtmainwindow.cpp \
    base/cJSON.c \
    base/serverbase.cpp \
    base/tablestyle.cpp \
    base/timebase.cpp \
    base/ui_rotate_management.cpp \
    base/wins_management.cpp \
    main.cpp \
    qt_ui/mainwindow.cpp \
    qt_ui/ui_algorithm_brightnessfilter.cpp \
    qt_ui/ui_algorithm_colorfilter.cpp \
    qt_ui/ui_algorithm_setting.cpp \
    qt_ui/ui_image_capture.cpp \
    qt_ui/ui_imgcap_log.cpp \
    qt_ui/ui_log_show.cpp \
    qt_ui/ui_login.cpp \
    qt_ui/ui_register.cpp \
    qt_ui/ui_reset_passwd.cpp \
    qt_ui/ui_system_setting.cpp \
    sql/mysqltable.cpp \
    sql/sqlbase.cpp \
    sql/sqltable_imgcaplog.cpp


HEADERS += \
    Thread/myclient.h \
    Thread/myserver.h \
    Thread/mytask.h \
    base/Qtmainwindow.h \
    base/cJSON.h \
    base/eventbase.h \
    base/serverbase.h \
    base/tablestyle.h \
    base/timebase.h \
    base/ui_rotate_management.h \
    base/wins_management.h \
    qt_ui/mainwindow.h \
    qt_ui/ui_algorithm_brightnessfilter.h \
    qt_ui/ui_algorithm_colorfilter.h \
    qt_ui/ui_algorithm_setting.h \
    qt_ui/ui_image_capture.h \
    qt_ui/ui_imgcap_log.h \
    qt_ui/ui_log_show.h \
    qt_ui/ui_login.h \
    qt_ui/ui_register.h \
    qt_ui/ui_reset_passwd.h \
    qt_ui/ui_system_setting.h \
    sql/mysqltable.h \
    sql/sqlbase.h \
    sql/sqltable_imgcaplog.h



FORMS += \
    qt_ui/mainwindow.ui \
    qt_ui/ui_algorithm_brightnessfilter.ui \
    qt_ui/ui_algorithm_colorfilter.ui \
    qt_ui/ui_algorithm_setting.ui \
    qt_ui/ui_image_capture.ui \
    qt_ui/ui_imgcap_log.ui \
    qt_ui/ui_log_show.ui \
    qt_ui/ui_login.ui \
    qt_ui/ui_register.ui \
    qt_ui/ui_reset_passwd.ui \
    qt_ui/ui_system_setting.ui




INCLUDEPATH +=F:\opencv4.5_mingw64\opencv\build\x64\mingw\install\include
LIBS += F:\opencv4.5_mingw64\opencv\build\x64\mingw\install\x64\mingw\bin\libopencv_*.dll

#INCLUDEPATH +=G:\OpenCV_4.5.4_Build\install\include
#LIBS += G:\OpenCV_4.5.4_Build\install\x64\mingw\bin\libopencv_*.dll

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

DISTFILES +=
