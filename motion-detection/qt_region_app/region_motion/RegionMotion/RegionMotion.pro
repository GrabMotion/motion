#-------------------------------------------------
#
# Project created by QtCreator 2001-01-01T00:09:11
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RegionMotion
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    tinyxml/tinystr.cpp \
    tinyxml/tinyxml.cpp \
    tinyxml/tinyxmlerror.cpp \
    tinyxml/tinyxmlparser.cpp \
    spinner/QtWaitingSpinner.cpp \
    socket/PracticalSocket.cpp \
    image/mat2qimage.cpp

HEADERS  += mainwindow.h \
    tinyxml/tinystr.h \
    tinyxml/tinyxml.h \
    spinner/QtWaitingSpinner.h \
    socket/PracticalSocket.h \
    image/mat2qimage.h

FORMS    += mainwindow.ui

CONFIG   += c++11

INCLUDEPATH += "/usr/local/include" \

LIBS += -L/usr/local/lib \
     -lopencv_core \
     -lopencv_imgproc \
     -lopencv_features2d \
     -lopencv_highgui

CONFIG += mobility
