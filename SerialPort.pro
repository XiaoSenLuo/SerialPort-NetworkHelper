QT       += core gui serialport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

include($$PWD/TTKWidgetTools-2.2.0.0/TTKModule/TTKModule.pro)

TEMPLATE = app

TARGET = SerialPort

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += \
    main.cpp \
    mainwindow.cpp \
    serialhelper.cpp \
    serialthread.cpp \
    settingconfig.cpp \
    settingdialog.cpp \
    tcphelper.cpp \
    udphelper.cpp

HEADERS += \
    main.h \
    mainwindow.h \
    serialhelper.h \
    serialthread.h \
    settingconfig.h \
    settingdialog.h \
    tcphelper.h \
    udphelper.h

FORMS += \
    mainwindow.ui \
    settingdialog.ui

TRANSLATIONS += \
    SerialPort_zh_CN.ts

win32-msvc*:QMAKE_CXXFLAGS += /utf-8

# Default rules for deployment.
#qnx: target.path = $${PWD}/../release/$${TARGET}
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

release {
DESTDIR = $${PWD}/../release/$${TARGET}
}

debug{
DESTDIR = $${PWD}/../debug/$${TARGET}
}


RESOURCES += \
    res.qrc

win32:RC_FILE = SerialPort.rc

DISTFILES += \
    SerialPort.rc
