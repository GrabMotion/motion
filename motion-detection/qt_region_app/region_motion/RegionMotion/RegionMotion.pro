#-------------------------------------------------
#
# Project created by QtCreator 2001-01-01T03:37:53
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = RegionMotion
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    drawing/mouse_coordinates.cpp

HEADERS  += mainwindow.h \
    drawing/mouse_coordinates.h

FORMS    += mainwindow.ui


INCLUDEPATH += "/usr/local/include" \

LIBS += -L/usr/local/lib \
     -lopencv_core \
     -lopencv_imgproc \
     -lopencv_features2d \
     -lopencv_highgui
