#include "drawing/mouse_coordinates.h"


#include <QString>
#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QMessageBox>

struct cool_operator
{
    cool_operator(int _n = 0, bool b = true) : first(b), n(_n) {}

    bool first;

    bool operator <(int x) const
    {
        return first && (n < x);
    }

    int n;
};

cool_operator operator <(int x, cool_operator const &rhs)
{
    return cool_operator(rhs.n, x < rhs.n);
}

mouse_coordinates::mouse_coordinates(QWidget *parent) : QLabel(parent)
{
    this->setMouseTracking(true);
    this->isPressed=false;
}

mouse_coordinates::~mouse_coordinates() {}

bool isEventInBound(QMouseEvent * mouse_event, QLabel * label)
{
    QPoint mouse_pos = mouse_event->pos();
    if ( mouse_pos.x() <= label->size().width() && mouse_pos.y() <= label->size().height())
    {
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
        //emit Mouse_Pressed_Right_Click(coor);
    }
    else
        if (isEventInBound(evt, this))
        {
            //emit Mouse_Pressed(coor);
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

    if (coor.size()>4)
    {
        cv::Point2f fistp = coor.at(0);
        cool_operator cx (fistp.x);
        cool_operator cy (fistp.y);

        if ((std::cout << std::boolalpha << (p.x - 5 < cx < p.x + 5)) &&
            (std::cout << std::boolalpha << (p.y - 5 < cy < p.y + 5)))
        {
            mouse_coordinates::SaveRegion();
        }
    }
    update();
    startPoint=endPoint;
}

vector<Point2f> mouse_coordinates::stringToVectorPoint2f(std::string storedcoord)
{

    vector<Point2f> coordinates;
    std::stringstream ss(storedcoord);
    string d;
    int c=0, x=0, y=0, t=0;
    while (ss >> d)
    {
        d = d.erase( d.size() - 1 );
        bool fi = d.find("[");
        if (!fi)
        {
            d = d.erase(0 , 1);
        }

        float cor =  atof (d.c_str());
        if (c==0)
        {
            x = cor;
            c++;
         } else
        {
            y = cor;
            cv::Point2f p(x, y);
            coordinates.push_back(p);
            c=0;x=0;y=0;
        }
        t++;
    }
    return coordinates;
}

void mouse_coordinates::drawLinesSlot(std::vector<cv::Point2f> lines_array)
{
    start_region = false;
    myLine *line=new myLine;
    if (lines_array.size()>0)
    {
        int size = lines_array.size();
        for (int i=0; i<size;i++)
        {
            cv::Point2f p = lines_array.at(i);
            QPoint point = QPoint(p.x, p.y);

            if (!start_region)
            {
                startPoint=point;
                start_region = true;
            }

            endPoint = point;
            myLine *line = new myLine;
            line->startPoint=startPoint;
            line->endPoint=endPoint;
            this->lines.push_back(line);

            update();
            startPoint=endPoint;
        }
     }
     update();
     start_region=false;
}

void mouse_coordinates::SaveRegion()
{

    update();

    //this->setStyleSheet("QLabel { background-color: black }");

    std::stringstream rr;
    rr << coor;

    std::string storedcoord = rr.str();

    //std::string basefile = "/jose/repos/motion/region_1.txt";
    //std::ofstream out;
    //out.open (basefile.c_str());
    //out << storedcoord << "\n";
    //out.close();

    startPoint = QPoint();
    endPoint = QPoint();
    coor.clear();
    lines.clear();

    emit savedRegionResutl(QString::fromStdString(storedcoord));

}

void mouse_coordinates::ClearRegion()
{
    isPressed = false;
    startPoint = QPoint();
    endPoint = QPoint();
    coor.clear();
    lines.clear();
    update();
}
