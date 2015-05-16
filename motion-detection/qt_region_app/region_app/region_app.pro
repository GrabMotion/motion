#-------------------------------------------------
#
# Project created by QtCreator 2015-05-02T02:20:06
#
#-------------------------------------------------

QT      += core gui webkitwidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET      = region
TEMPLATE    = app

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
    threads/tcpechothread.cpp

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
    threads/tcpechothread.h

FORMS    += mainwindow.ui

CONFIG   += c++11

INCLUDEPATH += "/usr/local/include" \

LIBS += -L/usr/local/lib \
     -lopencv_core \
     -lopencv_imgproc \
     -lopencv_features2d \
     -lopencv_highgui

DISTFILES += \
    img/pause.png \
    img/play.png \
    img/playstatic.png

CONFIG += mobility
MOBILITY = 
