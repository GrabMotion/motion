#include "drawing/mouse_coordinates.h"

#include <QString>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QMessageBox>

mouse_coordinates::mouse_coordinates(QWidget *parent) : QLabel(parent)
{
    this->setMouseTracking(true);
    this->isPressed=false;
}

mouse_coordinates::~mouse_coordinates() {}

bool isEventInBound(QMouseEvent * mouse_event, QLabel * label)
{
    QPoint mouse_pos = mouse_event->pos();
    if ( mouse_pos.x() <= label->size().width() && mouse_pos.y() <= label->size().height()){
        if(mouse_pos.x() > 0 && mouse_pos.y()  >0)
        {
            return true;
        }
    }
    return false;
}

void mouse_coordinates::mouseMoveEvent(QMouseEvent* evt)
{
    if (isEventInBound(evt, this))
    {
       QPoint mouse_pos = evt->pos();
       emit sendMousePosition(mouse_pos);
    }
}

void mouse_coordinates::mousePressEvent(QMouseEvent* evt)
{
    if(evt->button() == Qt::RightButton)
    {
        emit Mouse_Pressed_Right_Click(coor);
    }
    else
        if (isEventInBound(evt, this))
        {
            emit Mouse_Pressed(coor);
        }
}

void mouse_coordinates::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen;
    pen.setColor(Qt::red);
    pen.setWidth(1);
    painter.setPen(pen);

    for(int i=0;i<lines.size();i++)
    {
        myLine *line=lines[i];
        painter.drawLine(line->startPoint,line->endPoint);
    }
}

void mouse_coordinates::mouseReleaseEvent(QMouseEvent* evt)
{
    setCursor(Qt::ArrowCursor);

    cv::Point2f p(evt->x(), evt->y());
    coor.push_back(p);

    if (!start_region)
    {
        startPoint=evt->pos();
        start_region = true;
    }

    endPoint = evt->pos();
    myLine *line=new myLine;
    line->startPoint=startPoint;
    line->endPoint=endPoint;
    this->lines.push_back(line);

    update();
    startPoint=endPoint;

    //emit Mouse_Pressed_Right_Click(coor);
}



