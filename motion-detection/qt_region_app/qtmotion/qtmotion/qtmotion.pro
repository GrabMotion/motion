#-------------------------------------------------
#
# Project created by QtCreator 2015-08-27T18:53:28
#
#-------------------------------------------------

QT       += core gui network script

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DigiMotion
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    drawing/mouse_coordinates.cpp \
    spinner/QtWaitingSpinner.cpp \
    image/mat2qimage.cpp \
    socket/PracticalSocket.cpp \
    threads/broadcastthread.cpp \
    threads/mountthread.cpp \
    threads/streamingthread.cpp \
    tinyxml/tinystr.cpp \
    tinyxml/tinyxml.cpp \
    tinyxml/tinyxmlerror.cpp \
    tinyxml/tinyxmlparser.cpp \
    threads/tcpechothread.cpp \
    socket/socketlistener.cpp \
    protobuffer/motion.pb.cc \
    b64/base64.cpp \
    socket/streamlistener.cpp \
    socket/streamsender.cpp \
    socket/udpserver.cpp \
    socket/matlistener.cpp \
    login/login.cpp

HEADERS  += mainwindow.h \
    drawing/mouse_coordinates.h \
    threads/broadcastthread.h \
    threads/streamingthread.h \
    threads/mountthread.h \
    image/mat2qimage.h \
    socket/PracticalSocket.h \
    spinner/QtWaitingSpinner.h \
    tinyxml/tinystr.h \
    tinyxml/tinyxml.h \
    tinyxml/tinystr.h \
    tinyxml/tinyxml.h \
    threads/tcpechothread.h \
    socket/socketlistener.h \
    protobuffer/motion.pb.h \
    b64/base64.h \
    socket/streamlistener.h \
    socket/streamsender.h \
    socket/matlistener.h \
    socket/udpserver.h \
    login/login.h

FORMS    += mainwindow.ui

CONFIG   += c++11

INCLUDEPATH += "/usr/local/include" \

LIBS += -L /usr/local/lib \
     -lopencv_core \
     -lopencv_imgproc \
     -lopencv_features2d \
     -lopencv_highgui \
     -lopencv_imgcodecs \
     -lprotobuf
     -pthread

QMAKE_CXXFLAGS += -std=c++0x -pthread

DESTDIR     = ../macdeployqt
OBJECTS_DIR = ../macdeployqt
MOC_DIR     = ../macdeployqt

QMAKE_MAC_SDK = macosx10.11
