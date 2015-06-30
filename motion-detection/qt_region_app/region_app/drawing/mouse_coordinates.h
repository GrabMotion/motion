#ifndef MOUSE_COORDINATES_H
#define MOUSE_COORDINATES_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include <QPoint>
#include <QPaintEvent>
#include <QVector>

#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <fstream>

#include "mainwindow.h"

typedef struct myLine{
    QPoint startPoint;
    QPoint endPoint;
} myLine;

typedef struct pLine{
    QPoint startPoint;
    QPoint endPoint;
} pLine;

class mouse_coordinates : public QLabel
{
    Q_OBJECT
public:
    explicit mouse_coordinates(QWidget * parent=0 );
    ~mouse_coordinates();

    void mouseMoveEvent(QMouseEvent * evt);
    void mousePressEvent(QMouseEvent *evt);
    void mouseReleaseEvent(QMouseEvent *evt);

    int x,y;

    // Continued drawing
    QPoint startPoint;
    QPoint endPoint;
    bool isPressed;
    QVector<myLine*> lines;

    QVector<pLine*> plines;
    QMouseEvent* event;

    QPainterPath path;
    myLine *line;
    bool start_region;

    std::vector<cv::Point2f> coor;

    void drawLinesSlot(std::vector<cv::Point2f> lines);
    vector<Point2f> stringToVectorPoint2f(std::string storedcoord);

    void SaveRegion();

protected:
    void paintEvent(QPaintEvent *);

signals:
    //void Mouse_Pressed(std::vector<cv::Point2f>&);
    //void Mouse_Pressed_Right_Click(std::vector<cv::Point2f>&);
    void Mouse_Pos();
    //void Mouse_Left();
    void sendMousePosition(QPoint&);
    void savedRegionResutl(QString);

    //void discoverMouseCoordinates(mouse_coordinates&);


//private slots:
    //void drawLinesSlot(std::vector<cv::Point2f> lines);

};

#endif
