#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPoint>
#include <QMouseEvent>
#include <QtCore>
#include <QString>
#include <QMessageBox>
#include <vector>
#include <QFileSystemModel>
#include <QObject>
#include <QModelIndex>
#include <QTreeWidgetItem>
#include <QGraphicsView>
#include <QTimeLine>
#include <string>
#include <qfile.h>

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <cstdio>

#include "tinyxml/tinystr.h"
#include "tinyxml/tinyxml.h"

#include <ctime>

#include <ifaddrs.h>
#include "./spinner/QtWaitingSpinner.h"

#include <pthread.h>

const int STARTED_NOT_ENGAGED = 5000;
const int ENGAGED_NO_INSTANCE = 5001;
const int ENGAGED_INSTANCE    = 5002;

using namespace google::protobuf::io;
using namespace std;
using namespace cv;

class ListItems
{
    public:
        std::string name;
        std::string file;
        std::string path;
};
Q_DECLARE_METATYPE(ListItems*)

Mat main_mat;
ListItems * items;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //set toogle to recgnition button.
    ui->start_recognition->setCheckable(true);

    //BroadCast Sockets:
    broadcast_thread = new BroadcastThread(this);
    connect(broadcast_thread, SIGNAL(BroadcastReceived(QString)), this, SLOT(BroadcastReceived(QString)));
    connect(broadcast_thread, SIGNAL(BroadcastTimeoutSocketException()), this, SLOT(broadcastTimeoutSocketException()));

    //Socket Listener Class
    //socket_listener = new SocketListener(this);
    //socket_listener->startListening(this);

    //Video Streamig

    //mat_listener = new MatListener(this);
    //mat_listener->startListening(this);

    //Mount Shares
    mount_thread = new MountThread(this);
    connect(mount_thread, SIGNAL(SharedMounted(QString)), this, SLOT(SharedMounted(QString)));

    //Mouse Operations
    connect(ui->qt_drawing_output, SIGNAL(sendMousePosition(QPoint&)), this, SLOT(showMousePosition(QPoint&)));
    connect(ui->qt_drawing_output, SIGNAL(Mouse_Pos()), this, SLOT(Mouse_current_pos()));
    connect(ui->qt_drawing_output, SIGNAL(savedRegionResutl(QString)), this, SLOT(savedRegionResutl(QString)));

    //Combos.
    connect(ui->motionday,SIGNAL(currentIndexChanged(const QString&)), this, SLOT(dayComboChange(const QString &)));
    //connect(ui->rec,SIGNAL(currentIndexChanged(const QString&)), this, SLOT(recComboChange(const QString &)));

    //EditText
    connect(ui->rec_name, SIGNAL(textChanged(const QString &)), this, SLOT(onRecNameTextChanged()));

    //ItemTreeClick
    //connect(ui->remote_directory, SIGNAL(itemClicked(QTreeWidgetItem*,int)),this, SLOT(MainWindow::on_treewidget_clicked(QTreeWidgetItem*, int)));

    enableDisableButtons(STARTED_NOT_ENGAGED);

    //Stream Listener Class -- Cannot connect received
    //stream_listener = new StreamListener(this);
    //stream_listener->startListening(this);

    //udp_server = new UDPServer();
    //udp_server->startListening(this);

    //Streaming Resutl
    //streaming_thread = new StreamingThread(this);
    //connect(streaming_thread, SIGNAL(StreamingUpdateLabelImage(std::string, Mat)), this, SLOT(StreamingUpdateLabelImage(std::string, Mat)));

    //Mouse Events
    //connect(ui->qt_drawing_output, SIGNAL(discoverMouseCoordinates(mouse_coordinates&)), this, SLOT(getMouseCoordinates(mouse_coordinates mc)));
    //connect(ui->qt_drawing_output, SIGNAL(Mouse_Pressed(std::vector<cv::Point2f>&)), this, SLOT(Mouse_pressed(std::vector<cv::Point2f>&)));
    //connect(ui->qt_drawing_output, SIGNAL(Mouse_Pressed_Right_Click(std::vector<cv::Point2f>&)), this, SLOT(Mouse_Pressed_Right_Click(std::vector<cv::Point2f>&)));
    //connect(ui->qt_drawing_output, SIGNAL(Mouse_Left()), this, SLOT(Mouse_left()));

    //Top Spinner
    m_spinner = new QtWaitingSpinner(this);
    QVBoxLayout *spinnerTopLayout = new QVBoxLayout;
    spinnerTopLayout->insertWidget(0, m_spinner);
    spinnerTopLayout->insertStretch(0);
    spinnerTopLayout->addStretch();
    m_spinner->setNumberOfLines(13);
    m_spinner->setLineLength(6);
    m_spinner->setLineWidth(1);
    m_spinner->setInnerRadius(4);
    ui->top_spinner->insertLayout(1, spinnerTopLayout);

    //Instance Spinner
    i_spinner = new QtWaitingSpinner(this);
    QVBoxLayout *spinnerInstanceLayout = new QVBoxLayout;
    spinnerInstanceLayout->insertWidget(0, i_spinner);
    spinnerInstanceLayout->insertStretch(0);
    spinnerInstanceLayout->addStretch();
    i_spinner->setNumberOfLines(13);
    i_spinner->setLineLength(6);
    i_spinner->setLineWidth(1);
    i_spinner->setInnerRadius(4);
    ui->instance_spinner->insertLayout(1, spinnerInstanceLayout);

    //Local Network
    MainWindow::getLocalNetwork();

    ui->mat_progress->setValue(0);
    ui->xml_progress->setValue(0);

    ui->delay->addItem(tr("1"));
    ui->delay->addItem(tr("2"));
    ui->delay->addItem(tr("3"));
    ui->delay->addItem(tr("4"));
    ui->delay->addItem(tr("5"));
    ui->delay->addItem(tr("6"));
    int indexd = ui->delay->findText("3");
    if (indexd != -1)
    {
        ui->delay->setCurrentIndex(indexd);
        ui->rec->setCurrentText("3");
    }

    ui->mapdrive->setEnabled(false);
    ui->codename->setText("prueba");

    ip_address = getIpAddress();

    ui->speedcombo->addItem(QString::number(motion::Message::SOCKET_BUFFER_MICRO_SIZE));
    ui->speedcombo->addItem(QString::number(motion::Message::SOCKET_BUFFER_SMALL_SIZE));
    ui->speedcombo->addItem(QString::number(motion::Message::SOCKET_BUFFER_REGULAR_SIZE));
    ui->speedcombo->addItem(QString::number(motion::Message::SOCKET_BUFFER_MEDIUM_SIZE));

    int indexs = ui->speedcombo->findText(QString::number(motion::Message::SOCKET_BUFFER_MEDIUM_SIZE));
    if (indexs != -1)
    {
        ui->speedcombo->setCurrentIndex(indexs);
        ui->rec->setCurrentText(QString::number(motion::Message::SOCKET_BUFFER_MEDIUM_SIZE));
    }

    ui->output->setStyleSheet("border: 1px solid grey");

    MainWindow::enableDisableButtons(STARTED_NOT_ENGAGED);

    //Start Serarch
    MainWindow::on_search_button_clicked();

}

void MainWindow::getLocalNetwork()
{
    std::string sech = getIpAddress();
    local_ip = sech;

    vector<string> ip_vector;
    split(local_ip, '.', ip_vector);

    for (int i=0; i<ip_vector.size(); i++)
    {
        if ( i==0 | i==1)
        {
             NETWORK_IP +=  ip_vector[i] + ".";
        }
        else if ( i==2 )
        {
             NETWORK_IP +=  ip_vector[i];
        }
    }
    QString ip = QString::fromUtf8(NETWORK_IP.c_str());
    ui->network_ip->setText(ip);
}

MainWindow::~MainWindow()
{
    delete ui;
}

std::string getGlobalIntToString(int id)
{
    std::stringstream strm;
    strm << id;
    return strm.str();
}

int getGlobalStringToInt(std::string id)
{
   return atoi( id.c_str() );
}



char* convert2char(QString input)
{
    return (char*)input.data();
}


void MainWindow::on_search_button_clicked ()
{
    broadcast_thread->start();
    m_spinner->start();
}

void MainWindow::BroadcastReceived(QString ip)
{
    m_spinner->stop();
    ui->ips_combo->addItem(ip);
    ui->engage_button->setEnabled(true);
    ui->mapdrive->setEnabled(true);
    ui->mapdrive->setChecked(true);
}

void MainWindow::broadcastTimeoutSocketException()
{
    m_spinner->stop();
    QMessageBox* msgBox 	= new QMessageBox();
    msgBox->setWindowTitle("Trying to connect with terminals");
    msgBox->setText("No terminal found on the network.");
    msgBox->setWindowFlags(Qt::WindowStaysOnTopHint);
    msgBox->show();
    ui->engage_button->setEnabled(false);
    ui->mapdrive->setChecked(false);
}


std::string MainWindow::getActiveTerminalIPString()
{
    QString qs = ui->ips_combo->currentText();
    std::string utf8_text = qs.toUtf8().constData();
    return qs.toLocal8Bit().constData();
}

void MainWindow::on_engage_button_clicked()
{
    QString none;
    MainWindow::on_engage(none);
}

void MainWindow::on_engage(QString rec)
{

    i_spinner->start();

    //Serialize proto NEXT response.
    motion::Message::ActionType reply;
    motion::Message mreply;
    reply =  motion::Message::ActionType::Message_ActionType_RESPONSE_NEXT;
    mreply.set_type(reply);

    QString qt_ip = ui->ips_combo->currentText();
    QByteArray ba_ip = qt_ip.toLatin1();
    std::string qt_ip_str = qt_ip.toUtf8().constData();
    char *c_str_ip = ba_ip.data();

    //Aca tomar activemat del combo y mandar campo obligatorio.
    QVariant id = ui->cameracombo->itemData(ui->cameracombo->currentIndex());
    cout << "id: " << id.toFloat() << endl;
    mreply.set_activecam(id.toFloat());

    //Initialize objects to serialize.
    int size = mreply.ByteSize();
    char dataresponse[size];
    string datastr;
    mreply.SerializeToArray(&dataresponse, size);
    reply_next_proto = base64_encode(reinterpret_cast<const unsigned char*>(dataresponse),sizeof(dataresponse));
    //Speed
    QString qt_packagesize = ui->speedcombo->currentText();
    cout << "qt_packagesize: " << qt_packagesize.toStdString() << endl;
    packagesize = qt_packagesize.toInt();

    motion::Message m;
    m.set_type(motion::Message::ActionType::Message_ActionType_ENGAGE);

    m.set_currday(getCurrentDayLabel());
    m.set_currmonth(getCurrentMonthLabel());
    m.set_activecam(id.toFloat());

    if (rec.size()>0)
    {
        m.set_recname(rec.toStdString());
    }

    setMessageBodyAndSend(m);
    google::protobuf::ShutdownProtobufLibrary();

}

void MainWindow::on_picture_clicked()
{

    ui->picture->setChecked(true);

    QString qt_ip = ui->ips_combo->currentText();
    QString share = getSharedFolder();

    string scree_shot = share.toStdString() + "/"  + qt_ip.toStdString() +  "/screenshots/screen.jpg"; //region + "region.xml";
    string screenshots = share.toStdString() + "/"  + qt_ip.toStdString() +  "/screenshots"; //region + "region.xml";

    QString scree_shot_file = QString::fromStdString(scree_shot);
    QString screenshots_file = QString::fromStdString(scree_shot);

    if (QFile(scree_shot_file).exists())
    {
        QString path = screenshots_file;
        QDir dir(path);
        dir.setNameFilters(QStringList() << "*.*");
        dir.setFilter(QDir::Files);
        foreach(QString dirFile, dir.entryList())
        {
            dir.remove(dirFile);
        }
    }

    QString sh = getSharedFolder();

    motion::Message m;
    m.set_type(motion::Message::ActionType::Message_ActionType_TAKE_PICTURE);

    int index = ui->cameracombo->currentIndex();
    QVariant number = ui->cameracombo->itemData(index);
    float n = number.toFloat();
    m.set_activecam(n);

    m.set_currday(getCurrentDayLabel());
    m.set_currmonth(getCurrentMonthLabel());

    setMessageBodyAndSend(m);
    google::protobuf::ShutdownProtobufLibrary();

}

void MainWindow::on_refresh_clicked()
{

    i_spinner->start();

    motion::Message m;
    m.set_type(motion::Message::ActionType::Message_ActionType_REFRESH);

    //Aca tomar activemat del combo y mandar campo obligatorio.
    QVariant id = ui->cameracombo->itemData(ui->cameracombo->currentIndex());
    m.set_activecam(id.toFloat());

    QString rec = ui->rec->currentText();
    m.set_data(rec.toStdString()); //recname

    m.set_currday(getCurrentDayLabel());
    m.set_currmonth(getCurrentMonthLabel());
    m.set_activecam(id.toFloat());

    setMessageBodyAndSend(m);
    google::protobuf::ShutdownProtobufLibrary();

}

std::string MainWindow::getCurrentDayLabel()
{
    struct timeval td;
    struct tm* ptd;
    char day_rasp[9];
    gettimeofday (&td, NULL);
    ptd = localtime (&td.tv_sec);
    const char * dir = "%d%h%Y";
    strftime (day_rasp, sizeof (day_rasp), dir, ptd);
    std::string _day(day_rasp, 9);
    return _day;
}

std::string MainWindow::getCurrentMonthLabel()
{
    struct timeval tm;
    struct tm* ptm;
    char month_rasp[3];
    gettimeofday (&tm, NULL);
    ptm = localtime (&tm.tv_sec);
    strftime (month_rasp, sizeof (month_rasp), "%h", ptm);
    std::string _month(month_rasp, 3);
    return _month;
}


void MainWindow::on_collapse_clicked()
{
    ui->remote_directory->collapseAll();
}


void MainWindow::on_expand_clicked()
{
    ui->remote_directory->expandAll();
}



void MainWindow::on_watch_video_clicked()
{
    i_spinner->start();
    motion::Message m;
    std::string _day = MainWindow::getCurrentDayLabel();

    std::string path = items->path;
    video_name = items->name;

    vector<string> splpath = MainWindow::splitString(path, "/");
    std::string remove = splpath.at(splpath.size()-1);

    std::string::size_type i = path.find(remove);

    if (i != std::string::npos)
        path.erase(i, path.size());

    m.set_videofilepath(path);
    std::string file = path + video_name;
    m.set_data(file);

    m.set_type(motion::Message::ActionType::Message_ActionType_GET_VIDEO);
    setMessageBodyAndSend(m);
    google::protobuf::ShutdownProtobufLibrary();
}


void MainWindow::on_getxml_clicked()
{
    i_spinner->start();
    motion::Message m;

    std::string _day = MainWindow::getCurrentDayLabel();
    m.set_currday(_day);

    std::string _month = MainWindow::getCurrentMonthLabel();
    m.set_currmonth(_month);

    m.set_type(motion::Message::ActionType::Message_ActionType_GET_XML);

    int index = ui->cameracombo->currentIndex();
    m.set_activecam(index);

    motion::Message::MotionCamera * mcamera = m.add_motioncamera();

    motion::Message::MotionCamera * mprotocam = PROTO.mutable_motioncamera(index);
    std::string xmlpath = mprotocam->xmlfilepath();
    mcamera->set_xmlfilepath(xmlpath);

    QString qt_name = ui->rec_name->text();
    mcamera->set_recname(qt_name.toStdString());

    setMessageBodyAndSend(m);
    google::protobuf::ShutdownProtobufLibrary();
}

void MainWindow::setMessageBodyAndSend(motion::Message m)
{
    //spinner_folders->start();
    QString qt_ip = ui->ips_combo->currentText();
    QByteArray ba_ip = qt_ip.toLatin1();
    std::string qt_ip_str = qt_ip.toUtf8().constData();
    c_str_ip = ba_ip.data();

    QString qt_packagesize = ui->speedcombo->currentText();
    int pz = qt_packagesize.toInt();
    m.set_packagesize(pz);

    //string data;
    m.set_serverip(ip_address);
    m.set_time(getTime());

    //Initialize objects to serialize.
    int size = m.ByteSize();
    char dataresponse[size];
    string datastr;
    m.SerializeToArray(&dataresponse, size);

    std:string encoded_proto = base64_encode(reinterpret_cast<const unsigned char*>(dataresponse),sizeof(dataresponse));
    sendSocket(packagesize, c_str_ip, encoded_proto);

    google::protobuf::ShutdownProtobufLibrary();

}

QString MainWindow::getSharedFolder()
{
    QDir rootDir(QApplication::applicationDirPath());
    //QDir rootDir;
    //rootDir = QDir::currentPath();
    rootDir.cdUp();
    rootDir.cdUp();
    rootDir.cdUp();
    QString roStr = rootDir.absolutePath();
    QString roo = roStr + "/" + "shares";

    QByteArray bsh = roo.toLatin1();
    const char *shares = bsh.data();
    if (!QDir(shares).exists())
    {
        QDir().mkdir(shares);
    }

    return roo;
}

void MainWindow::SharedMounted(QString folder)
{

    QString sh = getSharedFolder();
    share = sh;

    QString ip = ui->ips_combo->currentText();
    QString rip = share + "/" + ip;
    QByteArray baip = rip.toLatin1();
    const char *ipfile = baip.data();

    treeViewPath = share;
    ipPath = rip;

    QString region = rip + "/region/";
    QByteArray baregion = rip.toLatin1();
    const char *regionfile = baregion.data();

    QString shots = rip + "/" + "screenshots";

    QByteArray bts = shots.toLatin1();
    const char *shrsots = bts.data();
    if (!QDir(shrsots).exists())
    {
        QDir().mkdir(shrsots);
    }

    //removeDir(region);
    if (!QDir(regionfile).exists())
    {
        QDir().mkdir(regionfile);
    }

    QString region_file = region + "region.xml";

    QByteArray xml_region = region_file.toLatin1();
    const char *xmlfile = xml_region.data();

}

QString MainWindow::getShare()
{
    return share;
}

void MainWindow::ShareUmounted(){}

void MainWindow::on_stream_clicked()
{

}

void MainWindow::on_save_region_clicked()
{
    ui->qt_drawing_output->SaveRegion();
}

void MainWindow::on_clear_region_clicked()
{
    ui->qt_drawing_output->ClearRegion();
}

/*void MainWindow::on_disconnect_clicked()
{
    motion::Message m;
    m.set_type(motion::Message::ActionType::Message_ActionType_DISSCONNECT);
    setMessageBodyAndSend(m);
    google::protobuf::ShutdownProtobufLibrary();
}*/

void MainWindow::on_get_time_clicked()
{
    motion::Message m;
    m.set_type(motion::Message::ActionType::Message_ActionType_GET_TIME);
    setMessageBodyAndSend(m);
    google::protobuf::ShutdownProtobufLibrary();
}

void MainWindow::showMousePosition( QPoint & pos )
{
    ui->label_xy->setText("x: " + QString::number(pos.x()) + " y : "  + QString::number(pos.y()) );
}

void MainWindow::Mouse_current_pos(){}

void MainWindow::closeEvent (QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "MOTION",
                                                                tr("Are you sure?\n"),
                                                                QMessageBox::Cancel | QMessageBox::No | QMessageBox::Yes,
                                                                QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes) {
        event->ignore();
    } else {
        event->accept();
    }
}

std::string MainWindow::getIpAddress ()
{

    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr = NULL;
    std::string address;
    std::stringstream strm6;
    std::stringstream strm;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            //printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);

            strm << addressBuffer;
            address = strm.str();

        } else if (ifa->ifa_addr->sa_family == AF_INET6) { // check it is IP6
            // is a valid IP6 Address
            tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            //printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);

            strm6 << addressBuffer;
            address = strm.str();
        }
    }
    if (ifAddrStruct!=NULL)
       freeifaddrs(ifAddrStruct);

    address.erase (0,9);

    return address;
}


void MainWindow::split(const string& s, char c, vector<string>& v)
{
   string::size_type i = 0;
   string::size_type j = s.find(c);

   while (j != string::npos)
   {
      v.push_back(s.substr(i, j-i));
      i = ++j;
      j = s.find(c, j);

      if (j == string::npos)
         v.push_back(s.substr(i, s.length()));
   }
}

void MainWindow::on_start_recognition_toggled(bool checked)
{

    motion::Message m;

    int index = ui->cameracombo->currentIndex();
    m.set_activecam(index);

    if (!checked)
    {
        m.set_type(motion::Message::ActionType::Message_ActionType_REC_STOP);
    }
    else
    {
        m.set_type(motion::Message::ActionType::Message_ActionType_REC_START);
    }

    std::string _month = getCurrentMonthLabel();
    m.set_currday(_month);

    std::string _day = getCurrentDayLabel();
    m.set_currmonth(_day);

    //Speed
    QString qt_packagesize = ui->speedcombo->currentText();
    int pz = qt_packagesize.toInt();
    m.set_packagesize(pz);

    motion::Message::MotionCamera * mcamera = m.add_motioncamera();

    QVariant number = ui->cameracombo->itemData(index);
    float camnum = number.toFloat();
    mcamera->set_cameranumber(camnum);

    QString cameraname = ui->cameracombo->currentText();
    mcamera->set_cameraname(cameraname.toStdString());

    QString qt_name = ui->rec_name->text();
    mcamera->set_recname(qt_name.toStdString());

    motion::Message::MotionRecognition * mrec = mcamera->add_motionrec();
    mrec->set_name(qt_name.toStdString());

    QVariant dbname = ui->rec->itemData(ui->rec->currentIndex());
    google::protobuf::int32 nameid = static_cast<int>(dbname.toFloat());
    mrec->set_db_idrec(nameid);

    setMessageBodyAndSend(m);
}

void MainWindow::on_save_rec_clicked()
{

    motion::Message m;

    int index = ui->cameracombo->currentIndex();
    m.set_activecam(index);

    m.set_type(motion::Message::ActionType::Message_ActionType_SAVE);
    motion::Message::MotionCamera * mcamera = m.add_motioncamera();

    QString vspeed = ui->speedcombo->currentText();

    int speed = vspeed.toInt();
    mcamera->set_speed(speed);

    std::string _month = getCurrentMonthLabel();
    m.set_currmonth(_month);

    motion::Message::MotionMonth * pmonth = mcamera->add_motionmonth();
    pmonth->set_monthlabel(_month);

    std::string _day = getCurrentDayLabel();
    m.set_currday(_day);

    mcamera->set_hasregion(region);
    if (region)
    {
        std::string res = region_resutl;
        std::string resencoded = base64_encode(reinterpret_cast<const unsigned char*>(res.c_str()), res.length());
        mcamera->set_coordinates(resencoded.c_str());
    }

    QString cname = ui->codename->text();
    std::string code = cname.toStdString();
    cout << "CODENAME: " << code << endl;
    mcamera->set_codename(code);

    QString dlay = ui->delay->currentText();
    google::protobuf::uint32 delay = dlay.toInt();
    mcamera->set_delay(delay);

    mcamera->set_storevideo(true);

    if (ui->has_images->isChecked())
    {
        mcamera->set_storeimage(true);
    } else
    {
        mcamera->set_storeimage(false);
    }

    motion::Message::MotionCamera * mprotocam = PROTO.mutable_motioncamera(index);

    int db_idcamera = mprotocam->db_idcamera();
    mcamera->set_db_idcamera(db_idcamera);

    QString qt_camera = ui->cameracombo->currentText();
    mcamera->set_cameraname(qt_camera.toStdString());

    QVariant number = ui->cameracombo->itemData(index);
    float camnum = number.toFloat();
    mcamera->set_cameranumber(camnum);

    google::protobuf::uint32 mrows = mprotocam->matrows();
    mcamera->set_matrows(mrows);

    google::protobuf::uint32 mcols = mprotocam->matcols();
    mcamera->set_matcols(mcols);

    google::protobuf::uint32 mwidth = mprotocam->matheight();
    mcamera->set_matheight(mwidth);

    google::protobuf::uint32 mheight = mprotocam->matwidth();
    mcamera->set_matwidth(mheight);

    mcamera->set_fromdatabase(false);

    //Name
    QString name = ui->rec_name->text();
    mcamera->set_recname(name.toStdString());

    if (ui->between->isChecked())
    {

        mcamera->set_hascron(true);
        char spacer = ' ';

        //Times start-end
        QTime starttime = ui->time_from->time();
        QString times = starttime.toString();
        mcamera->set_timestart(times.toStdString());

        vector<string> startsplit = MainWindow::splitString(times.toStdString(), ":");
        int starthour   = atoi(startsplit.at(0).c_str());
        int startminute = atoi(startsplit.at(1).c_str());

        stringstream startcron;
        if (startminute>0)
        {
            startcron << startminute;
        } else
        {
            startcron << "* ";
        }
        startcron << starthour << " * * *";

        motion::Message::MotionCron * mcronstart = mcamera->add_motioncron();
        mcronstart->set_command(startcron.str());

        stringstream startprogram;
        startprogram                        <<
        "/home/pi/motion/motion-detection/motion_src/src/motion_detect/./motion_detect_raspberry -start " <<
        mcamera->db_idcamera()              << spacer << name.toStdString();
        mcronstart->set_program(startprogram.str());

        QTime endtime = ui->time_to->time();
        QString timee = endtime.toString();
        mcamera->set_timeend(timee.toStdString());

        vector<string> stopsplit = MainWindow::splitString(timee.toStdString(), ":");
        int endstarthour   = atoi(stopsplit.at(0).c_str());
        int endstartminute = atoi(stopsplit.at(1).c_str());

        stringstream endcron;
        if (endstartminute>0)
        {
            endcron << endstartminute;
        } else
        {
            endcron << "* ";
        }
        endcron << endstarthour << " * * *";

        motion::Message::MotionCron * mcronstop = mcamera->add_motioncron();
        mcronstop->set_command(endcron.str());
        stringstream stopprogram;
        stopprogram <<
        "/home/pi/motion/motion-detection/motion_src/src/motion_detect/./motion_detect_raspberry -stop "  <<
        mcamera->db_idcamera()              << spacer << name.toStdString();
        mcronstop->set_program(stopprogram.str());

    } else
    {
        mcamera->set_hascron(false);
    }

    int dbmat = mprotocam->db_idmat();
    mcamera->set_db_idmat(dbmat);



    bool startup = ui->startup->isChecked();
    if (startup)
    {
        mcamera->set_runatstartup(true);
    } else
    {
        mcamera->set_runatstartup(false);
    }

    if (mprotocam->has_lastinstance())
    {
        std::string last = mprotocam->lastinstance();
        mcamera->set_lastinstance(last);
    }

    setMessageBodyAndSend(m);

}

void MainWindow::on_rec_new_clicked()
{

    //Clear Coords
    ui->qt_drawing_output->ClearRegion();

    //Clear Image
    ui->output->clear();

    //Clear List and Combox
    ui->remote_directory->clear();
    ui->motionmonth->clear();
    ui->motionday->clear();
    ui->rec->clear();
    ui->rec_name->clear();

    //Clear START
    ui->start_recognition->setStyleSheet(styleSheet());

    std::string from = "7:00:00";
    QTime timestart(QTime::fromString(from.c_str(), "hh:mm:ss"));
    ui->time_from->setTime(timestart);

    std::string to = "21:00:00";
    QTime timestop(QTime::fromString(to.c_str(), "hh:mm:ss"));
    ui->time_to->setTime(timestop);

    ui->between->setChecked(false);
    ui->time_from->setEnabled(false);
    ui->time_to->setEnabled(false);

    ui->save_rec->setEnabled(false);

}

void MainWindow::on_set_time_clicked()
{

    struct timeval tv;
    struct tm* ptm;
    char time_string[40];

    gettimeofday (&tv, NULL);
    ptm = localtime (&tv.tv_sec);
    strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S %z", ptm);

    GOOGLE_PROTOBUF_VERIFY_VERSION;
    motion::Message m;
    m.set_type(motion::Message::ActionType::Message_ActionType_SET_TIME);
    m.set_time(time_string);
    setMessageBodyAndSend(m);

}

void MainWindow::SocketErrorMessage(QString &e)
{
    QMessageBox* msgBox 	= new QMessageBox();
    msgBox->setWindowTitle("Trying to connect with terminals");
    msgBox->setText(e);
    msgBox->setWindowFlags(Qt::WindowStaysOnTopHint);
    msgBox->show();
}

std::string MainWindow::get_file_contents(std::string filename)
{
    std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
    if (in)
    {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return(contents);
    }
    throw(errno);
}

std::string MainWindow::getTime()
{
    struct timeval tv;
    struct tm* ptm;
    char time_string[40];
    gettimeofday (&tv, NULL);
    ptm = localtime (&tv.tv_sec);
    strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S %z", ptm);
    return time_string;
}

void MainWindow::getTerminalFolder(motion::Message m)
{

    int qt_cam_number = ui->cameracombo->currentIndex();
    std::stringstream qt_camstream;
    qt_camstream << "camera" <<  qt_cam_number;
    QString qt_cam = qt_camstream.str().c_str();

    QString qt_ip = ui->ips_combo->currentText();
    QDir rootDir(QApplication::applicationDirPath());
    rootDir.cdUp();
    rootDir.cdUp();
    rootDir.cdUp();
    QString basedir = rootDir.absolutePath();

    QString day = m.currday().c_str();
    QString qt_name = ui->rec->currentText();
    QString rec = qt_name;
    bool hasrec;

    if (qt_name.toStdString().size()>0)
    {
        hasrec = true;
    }

    folder_paths.clear();

    /* DATA */
    //data
    QString data = basedir + "/" + "data";
    QByteArray da = data.toLatin1();
    char *dashrs = da.data();
    if (!QDir(dashrs).exists())
    {
        QDir().mkdir(dashrs);
    }
    //data/data
    QString datadata = QString(dashrs) + "/" + "data";
    QByteArray ddada = datadata.toLatin1();
    char *dadashrs = ddada.data();
    if (!QDir(dadashrs).exists())
    {
        QDir().mkdir(dadashrs);
    }
    //data/data/camera
    QString datadata_camera = QString(dadashrs) + '/' + qt_cam;
    QByteArray datadatacamera = datadata_camera.toLatin1();
    char *datadata_camerafile = datadatacamera.data();
    if (!QDir(datadata_camerafile).exists())
    {
        QDir().mkdir(datadata_camerafile);
    }
    //data/data/camera/ip
    QString datadata_ip = QString(datadata_camerafile) + '/' + qt_ip;
    QByteArray datadataip = datadata_ip.toLatin1();
    char *datadata_ipfile = datadataip.data();
    if (!QDir(datadata_ipfile).exists())
    {
        QDir().mkdir(datadata_ipfile);
    }
    //data/data/camera/ip/day
    QString data_dayt_ip = QString(datadata_ipfile) + '/' + day;
    QByteArray datadtip = data_dayt_ip.toLatin1();
    char *data_day_ipfile = datadtip.data();
    if (!QDir(data_day_ipfile).exists())
    {
        QDir().mkdir(data_day_ipfile);
    }
    //data/data/camera/ip/day/rec
    if (hasrec)
    {
        QString data_rec_day_ip = QString(data_day_ipfile) + '/' + rec;
        QByteArray datarec = data_rec_day_ip.toLatin1();
        char * data_rec_day_ipfile = datarec.data();
        if (!QDir(data_rec_day_ipfile).exists())
        {
            QDir().mkdir(data_rec_day_ipfile);
        }
        stringstream sr;
        sr << data_rec_day_ipfile;
        folder_paths.push_back(sr.str()); // 0
    } else
    {
        stringstream ss;
        ss << data_day_ipfile;
        folder_paths.push_back(ss.str()); // 0
    }

    /* MAT */
    //data/mats
    QString mats = QString(data) + "/" + "mats";
    QByteArray ma = mats.toLatin1();
    char *matshrs = ma.data();
    if (!QDir(matshrs).exists())
    {
        QDir().mkdir(matshrs);
    }
    //data/mats/camera
    QString datadata_camera_mat = QString(matshrs) + '/' + qt_cam;
    QByteArray datadatacameramat = datadata_camera_mat.toLatin1();
    char *datadata_cameramatfile = datadatacameramat.data();
    if (!QDir(datadata_cameramatfile).exists())
    {
        QDir().mkdir(datadata_cameramatfile);
    }
    //data/mats/camera/ip
    QString mat_ip = QString(datadata_cameramatfile) + '/' + qt_ip;
    QByteArray matip = mat_ip.toLatin1();
    char *mat_ipfile = matip.data();
    if (!QDir(mat_ipfile).exists())
    {
        QDir().mkdir(mat_ipfile);
    }
    //data/mats/camera/ip/day
    QString mat_dayt_ip = QString(mat_ipfile) + '/' + day;
    QByteArray matdtip = mat_dayt_ip.toLatin1();
    char *mat_day_ipfile = matdtip.data();
    if (!QDir(mat_day_ipfile).exists())
    {
        QDir().mkdir(mat_day_ipfile);
    }
    stringstream sm;
    sm << mat_day_ipfile;
    folder_paths.push_back(sm.str()); // 1

    /* XML */
    //data/xml
    QString xmls = QString(data) + "/" + "xmls";
    QByteArray xm = xmls.toLatin1();
    char *xmlshrs = xm.data();
    if (!QDir(xmlshrs).exists())
    {
        QDir().mkdir(xmlshrs);
    }
    //data/xml/camera
    QString datadata_camera_xml = QString(xmlshrs) + '/' + qt_cam;
    QByteArray datadatacameraxml = datadata_camera_xml.toLatin1();
    char *datadata_cameraxmlfile = datadatacameraxml.data();
    if (!QDir(datadata_cameraxmlfile).exists())
    {
        QDir().mkdir(datadata_cameraxmlfile);
    }
    //data/xml/camera/ip
    QString xml_ip = QString(datadata_cameraxmlfile) + '/' + qt_ip;
    QByteArray xmltip = xml_ip.toLatin1();
    char *xml_ipfile = xmltip.data();
    if (!QDir(xml_ipfile).exists())
    {
        QDir().mkdir(xml_ipfile);
    }
    //data/xml/camera/ip/day
    QString xml_dayt_ip = QString(xml_ipfile) + '/' + day;
    QByteArray xmldtip = xml_dayt_ip.toLatin1();
    char *xml_day_ipfile = xmldtip.data();
    if (!QDir(xml_day_ipfile).exists())
    {
        QDir().mkdir(xml_day_ipfile);
    }
    //data/xml/camera/ip/day/rec
    if (hasrec)
    {
        QString data_xml_day_rec = QString(xml_day_ipfile) + '/' + rec;
        QByteArray dataxmlrec = data_xml_day_rec.toLatin1();
        char * xml_day_rec_ipfile = dataxmlrec.data();
        if (!QDir(xml_day_rec_ipfile).exists())
        {
            QDir().mkdir(xml_day_rec_ipfile);
        }
        stringstream sxr;
        sxr << xml_day_rec_ipfile;
        folder_paths.push_back(sxr.str()); // 2
    } else
    {
        stringstream sx;
        sx << xml_day_ipfile;
        folder_paths.push_back(sx.str()); // 2
    }

    /* IMAGE */
    //data/image
    QString images = QString(data) + "/" + "images";
    QByteArray img = images.toLatin1();
    char *imgshrs = img.data();
    if (!QDir(imgshrs).exists())
    {
        QDir().mkdir(imgshrs);
    }
    //data/image/camera
    QString datadata_camera_image = QString(imgshrs) + '/' + qt_cam;
    QByteArray datadatacameraimage = datadata_camera_image.toLatin1();
    char *datadata_cameraimagefile = datadatacameraimage.data();
    if (!QDir(datadata_cameraimagefile).exists())
    {
        QDir().mkdir(datadata_cameraimagefile);
    }
    //data/images/camera/ip
    QString img_ip = QString(datadata_cameraimagefile) + '/' + qt_ip;
    QByteArray imgtip = img_ip.toLatin1();
    char *img_ipfile = imgtip.data();
    if (!QDir(img_ipfile).exists())
    {
        QDir().mkdir(img_ipfile);
    }
    //data/images/camera/ip/day
    QString img_dayt_ip = QString(img_ipfile) + '/' + day;
    QByteArray imgdtip = img_dayt_ip.toLatin1();
    char *img_day_ipfile = imgdtip.data();
    if (!QDir(img_day_ipfile).exists())
    {
        QDir().mkdir(img_day_ipfile);
    }
    //data/images/camera/ip/day/rec
    if (hasrec)
    {
        QString data_image_day_rec = QString(img_day_ipfile) + '/' + rec;
        QByteArray dataimagerec = data_image_day_rec.toLatin1();
        char *img_day_rec_ipfile = dataimagerec.data();
        if (!QDir(img_day_rec_ipfile).exists())
        {
            QDir().mkdir(img_day_rec_ipfile);
        }
        stringstream sir;
        sir << img_day_rec_ipfile;
        folder_paths.push_back(sir.str()); // 3
    } else
    {
        stringstream si;
        si << img_day_ipfile;
        folder_paths.push_back(si.str()); // 3
    }

    /* VIDEO */
    //data/video
    QString videos = QString(data) + "/" + "videos";
    QByteArray vm = videos.toLatin1();
    char *videoshrs = vm.data();
    if (!QDir(videoshrs).exists())
    {
        QDir().mkdir(videoshrs);
    }
    //data/video/camera
    QString datadata_camera_video = QString(videoshrs) + '/' + qt_cam;
    QByteArray datadatacameravideo = datadata_camera_video.toLatin1();
    char *datadata_cameravideofile = datadatacameravideo.data();
    if (!QDir(datadata_cameravideofile).exists())
    {
        QDir().mkdir(datadata_cameravideofile);
    }
    //data/video/camera/ip
    QString video_ip = QString(datadata_cameravideofile) + '/' + qt_ip;
    QByteArray videotip = video_ip.toLatin1();
    char *video_ipfile = videotip.data();
    if (!QDir(video_ipfile).exists())
    {
        QDir().mkdir(video_ipfile);
    }
    //data/video/camera/ip/day
    QString video_dayt_ip = QString(video_ipfile) + '/' + day;
    QByteArray videodtip = video_dayt_ip.toLatin1();
    char *video_day_ipfile = videodtip.data();
    if (!QDir(video_day_ipfile).exists())
    {
        QDir().mkdir(video_day_ipfile);
    }
    //data/video/camera/ip/day/rec
    if (hasrec)
    {
        QString data_video_day_rec = QString(video_day_ipfile) + '/' + rec;
        QByteArray datavideorec = data_video_day_rec.toLatin1();
        char * video_day_rec_ipfile = datavideorec.data();
        if (!QDir(video_day_rec_ipfile).exists())
        {
            QDir().mkdir(video_day_rec_ipfile);
        }
        stringstream svr;
        svr << video_day_rec_ipfile;
        folder_paths.push_back(svr.str()); // 4
    } else
    {
        stringstream sv;
        sv << video_day_ipfile;
        folder_paths.push_back(sv.str()); // 4
    }
}

void MainWindow::saveMat(std::string encodedmat, google::protobuf::uint32 file)
{

    QString qt_ip = ui->ips_combo->currentText();
    QByteArray ba_ip = qt_ip.toLatin1();
    std::string qt_ip_str = qt_ip.toUtf8().constData();
    std::string matfolder = folder_paths.at(1);

    std::string basefile = matfolder  + "/" + std::to_string(file) + ".mat";
    std::ofstream out;
    out.open (basefile.c_str());
    out << encodedmat << "\n";
    out.close();

}

void MainWindow::loadMat(google::protobuf::uint32 file)
{

    QString qt_ip = ui->ips_combo->currentText();
    QByteArray ba_ip = qt_ip.toLatin1();
    std::string qt_ip_str = qt_ip.toUtf8().constData();
    std::string matfolder = folder_paths.at(1);

    std::string basefile = matfolder + "/" + std::to_string(file) + ".mat";

    cout << "basefile: " << basefile << endl;

    string loadedmat = MainWindow::get_file_contents(basefile);

    main_mat = MainWindow::extractMat(loadedmat);

    //Render image.
    //imwrite("/jose/repos/image_2.jpg", main_mat);
    QImage frame = Mat2QImage(main_mat);
    ui->output->setPixmap(QPixmap::fromImage(frame));

}

Mat MainWindow::extractMat(string loadedmat)
{
    std::string oridecoded = base64_decode(loadedmat);

    stringstream decoded;
    decoded << oridecoded;

    // The data we need to deserialize.
    int width_d = 0;
    int height_d = 0;
    int type_d = 0;
    int size_d = 0;

    // Read the width, height, type and size of the buffer
    decoded.read((char*)(&width_d), sizeof(int));
    decoded.read((char*)(&height_d), sizeof(int));
    decoded.read((char*)(&type_d), sizeof(int));
    decoded.read((char*)(&size_d), sizeof(int));

    // Allocate a buffer for the pixels
    char* data_d = new char[size_d];
    // Read the pixels from the stringstream
    decoded.read(data_d, size_d);

    Mat extracted = cv::Mat(height_d, width_d, type_d, data_d).clone();

    // Delete our buffer
    delete[]data_d;

    return extracted;
}

void MainWindow::saveImage(std::string oridecoded, std::string file)
{

    stringstream decoded;
    decoded << oridecoded;

    // The data we need to deserialize.
    int width_d = 0;
    int height_d = 0;
    int type_d = 0;
    int size_d = 0;

    // Read the width, height, type and size of the buffer
    decoded.read((char*)(&width_d), sizeof(int));
    decoded.read((char*)(&height_d), sizeof(int));
    decoded.read((char*)(&type_d), sizeof(int));
    decoded.read((char*)(&size_d), sizeof(int));

    // Allocate a buffer for the pixels
    char* data_d = new char[size_d];
    // Read the pixels from the stringstream
    decoded.read(data_d, size_d);

    Mat image_mat = cv::Mat(height_d, width_d, type_d, data_d).clone();

    //Render image.
    imwrite(file, image_mat);

    // Delete our buffer
    delete[]data_d;

}

void MainWindow::saveProto(string encodedproto, std::string file)
{

    QString qt_ip = ui->ips_combo->currentText();
    QByteArray ba_ip = qt_ip.toLatin1();
    std::string qt_ip_str = qt_ip.toUtf8().constData();
    std::string protofolder = folder_paths.at(0);

    std::string basefile = protofolder  + '/' + file + '/' + '.mat';
    std::ofstream out;
    out.open (basefile.c_str());
    out << encodedproto << "\n";
    out.close();

 }


vector<Point2f> MainWindow::stringToVectorPoint2f(std::string storedcoord)
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

void MainWindow::savedRegionResutl(QString result)
{
    std::string rr = result.toUtf8().constData();

    vector<Point2f> scorrd = MainWindow::stringToVectorPoint2f(rr);

    if(scorrd.size()>=4)
    {
       ui->qt_drawing_output->drawLinesSlot(scorrd);
       region_resutl = result.toUtf8().constData();
       ui->save_region->setEnabled(false);
       ui->save_region->setText("Region Saved");
       region=true;
    }
    else
    {
       QMessageBox* msgBox 	= new QMessageBox();
       msgBox->setWindowTitle("Region Save Failed");
       msgBox->setText("Region must contain at least three points.");
       msgBox->setWindowFlags(Qt::WindowStaysOnTopHint);
       msgBox->show();
    }

}

void MainWindow::remoteMat(cv::Mat mat)
{
    QImage frame = Mat2QImage(mat);
    ui->output->setPixmap(QPixmap::fromImage(frame));
}

char * MainWindow::getTimeChat()
{
    struct timeval tr;
    struct tm* ptmr;
    char time_rasp[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
    return time_rasp;
}

std::string MainWindow::getTimeStr()
{
    struct timeval tv;
    struct tm* ptm;
    char time_string[40];
    gettimeofday (&tv, NULL);
    ptm = localtime (&tv.tv_sec);
    strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S %z", ptm);
    return time_string;
}
char * MainWindow::getTerMinalIpFromCombo()
{
    QString qt_ip = ui->ips_combo->currentText();
    QByteArray ba_ip = qt_ip.toLatin1();
    std::string qt_ip_str = qt_ip.toUtf8().constData();
    char *c_str_ip = ba_ip.data();
}

void MainWindow::enable_diable_Uppper_Bar(bool set)
{
    ui->engage_button->setEnabled(set);
    ui->mapdrive->setEnabled(set);
    ui->label_camera->setEnabled(set);
    ui->label_rec_job->setEnabled(set);
    ui->cameracombo->setEnabled(set);
    ui->label_speed->setEnabled(set);
    ui->speedcombo->setEnabled(set);
    ui->label_up_since->setEnabled(set);
    ui->status_label->setEnabled(set);

    QFont f( "Arial", 10, QFont::Bold);

    if (!set)
    {
        ui->status_label->setStyleSheet("{background-color: rgba( 200, 200, 200, 100% )}, {color: #000000}");
    } else
    {
        ui->status_label->setStyleSheet("QLabel { background-color : rgba( 51, 153, 255, 100% ); color : #FFFFFF; }");
    }

}

void MainWindow::enable_diable_Lateral_Time(bool set)
{
    QFont f( "Arial", 11, QFont::Bold);
    ui->computer_time->setFont(f);
    ui->remote_terminal_time->setFont(f);
    ui->instance_started->setFont(f);
    ui->synched->setFont(f);

    ui->label_computer_time->setEnabled(set);
    ui->computer_time->setEnabled(set);
    ui->label_terminal_time->setEnabled(set);
    ui->remote_terminal_time->setEnabled(set);
    ui->set_time->setEnabled(set);
    ui->synched->setEnabled(set);
    ui->label_terminal_status->setEnabled(set);
    ui->label_rec_started->setEnabled(set);
}

void MainWindow::enable_diable_Lateral_Grid(bool set)
{
    ui->motionmonth->setEnabled(set);
    ui->motionday->setEnabled(set);
    ui->remote_directory->setEnabled(set);
    ui->getxml->setEnabled(set);
    ui->watch_video->setEnabled(set);
    ui->watch_image->setEnabled(set);
    ui->xml_progress->setEnabled(set);
}

void MainWindow::enable_diable_Lower_Bar(bool set)
{
    ui->stream->setEnabled(set);
    ui->picture->setEnabled(set);
    ui->mat_progress->setEnabled(set);
    ui->save_region->setEnabled(set);
    ui->clear_region->setEnabled(set);
    ui->code_label->setEnabled(set);
    ui->codename->setEnabled(set);
    ui->delay_label->setEnabled(set);
    ui->delay->setEnabled(set);
    ui->has_images->setEnabled(set);
    ui->between->setEnabled(set);
    if (!set)
    {
        ui->time_from->setEnabled(set);
        ui->time_to->setEnabled(set);
    }
    ui->label_name->setEnabled(set);
    ui->rec_name->setEnabled(set);
    ui->startup->setEnabled(set);
}

void MainWindow::enable_disable_Save(bool set)
{
    ui->save_rec->setEnabled(set);
    ui->rec_new->setEnabled(set);
    ui->rec->setEnabled(set);
}

void MainWindow::enable_diable_Process(bool set)
{
    ui->label_process->setEnabled(set);
    ui->time_process->setEnabled(set);
    ui->process->setEnabled(set);
    ui->quene->setEnabled(set);
    ui->run->setEnabled(set);
    ui->process_list->setEnabled(set);
    ui->edit_process->setEnabled(set);
    ui->end_process->setEnabled(set);
    ui->label_process_schedule->setEnabled(set);
}

void MainWindow::enableDisableButtons(int set)
{
    switch (set)
    {
        case STARTED_NOT_ENGAGED:
            MainWindow::enable_diable_Uppper_Bar(false);
            MainWindow::enable_diable_Lateral_Time(false);
            MainWindow::enable_diable_Lateral_Grid(false);
            ui->start_recognition->setEnabled(false);
            ui->refresh->setEnabled(false);
            ui->collapse->setEnabled(false);
            ui->expand->setEnabled(false);
            ui->getxml->setEnabled(false);
            ui->watch_video->setEnabled(false);
            ui->watch_image->setEnabled(false);
            MainWindow::enable_diable_Lower_Bar(false);
            MainWindow::enable_diable_Process(false);
            MainWindow::enable_disable_Save(false);
        break;

        case ENGAGED_NO_INSTANCE:
            MainWindow::enable_diable_Uppper_Bar(true);
            MainWindow::enable_diable_Lateral_Time(true);
            MainWindow::enable_diable_Lateral_Grid(false);
            //ui->start_recognition->setEnabled(true);
            ui->refresh->setEnabled(true);
            ui->collapse->setEnabled(false);
            ui->expand->setEnabled(false);
            ui->getxml->setEnabled(false);
            ui->watch_video->setEnabled(false);
            ui->watch_image->setEnabled(false);
            MainWindow::enable_diable_Lower_Bar(true);
            MainWindow::enable_diable_Process(true);
        break;

        case ENGAGED_INSTANCE:
            MainWindow::enable_diable_Uppper_Bar(true);
            MainWindow::enable_diable_Lateral_Time(true);
            MainWindow::enable_diable_Lateral_Grid(true);
            //ui->start_recognition->setEnabled(true);
            ui->refresh->setEnabled(true);
            ui->collapse->setEnabled(true);
            ui->expand->setEnabled(true);
            ui->getxml->setEnabled(true);
            //ui->watch_video->setEnabled(true);
            //ui->watch_image->setEnabled(true);
            MainWindow::enable_diable_Lower_Bar(true);
            MainWindow::enable_diable_Process(true);
        break;
    }
}

void MainWindow::sendSocket(int packagesize, string svradress, string command)
{
    tcpsend_thread = new TCPEchoThread(this);
    tcpsend_thread->SendEcho(packagesize, this,svradress,command);
}

std::string MainWindow::ExtractString( std::string source, std::string start, std::string end )
{
     std::size_t startIndex = source.find( start );
     if( startIndex == std::string::npos )
     {
        return "";
     }
     startIndex += start.length();
     std::string::size_type endIndex = source.find( end, startIndex );
     return source.substr( startIndex, endIndex - startIndex );
}

vector<string> MainWindow::splitString(string input, string delimiter)
{
     vector<string> output;
     char *pch;
     char *str = strdup(input.c_str());
     pch = strtok (str, delimiter.c_str());
     while (pch != NULL)
     {
        output.push_back(pch);
        pch = strtok (NULL,  delimiter.c_str());
     }
     free(str);
     return output;
}

std::vector<std::string> MainWindow::splitProto(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}

std::string MainWindow::IntToString ( int number )
{
    std::ostringstream oss;
    oss<< number;
    return oss.str();
}

void MainWindow::saveLocalProto(QString qproto, motion::Message remote)
{
    //First time.
    int size = remote.ByteSize();
    char dataresponse[size];
    string datastr;
    remote.SerializeToArray(&dataresponse, size);
    std:string encoded_proto = base64_encode(reinterpret_cast<const unsigned char*>(dataresponse),sizeof(dataresponse));

    std::ofstream out;
    out.open (qproto.toUtf8().constData());
    out << encoded_proto << "\n";
    out.close();

}

void MainWindow::loadInstances(motion::Message m)
{

    QTreeWidget *treeWidget = ui->remote_directory;
    ui->remote_directory->setColumnCount(3);
    treeWidget->clear();

    int sizec = m.motioncamera_size();

    for (int t = 0; t < sizec; t++)
    {

        motion::Message::MotionCamera * mcam = m.mutable_motioncamera(t);

        //Filter current.
        int camnum = mcam->cameranumber();
        QVariant id = ui->cameracombo->itemData(ui->cameracombo->currentIndex());
        int index = static_cast<int>(id.toFloat());

        if (camnum==index)
        {
            int sizem = mcam->motionmonth_size();
            for (int i = 0; i < sizem; i++)
            {
                 motion::Message::MotionMonth * mmonth = mcam->mutable_motionmonth(i);

                 std::string mlabel = mmonth->monthlabel();
                 int indexm = ui->motionmonth->findText(mlabel.c_str());
                 if (indexm==-1)
                 {
                    ui->motionmonth->addItem(mlabel.c_str());
                 } else
                 {
                    ui->motionmonth->setCurrentIndex(indexm);
                 }
                 ui->motionmonth->setEnabled(true);
                 int sized = mmonth->motionday_size();

                 int current = sized - 1;
                 bool hastoday = false;

                 if (sized>0)
                 {
                     for (int j = 0; j < sized; j++)
                     {
                         motion::Message::MotionDay * mday = mmonth->mutable_motionday(j);

                         std::string dlabel = mday->daylabel();
                         int indexd = ui->motionday->findText(dlabel.c_str());
                         if (indexd==-1)
                         {
                            ui->motionday->addItem(dlabel.c_str());
                         } else
                         {
                             ui->motionday->setCurrentIndex(indexd);
                             ui->motionday->setCurrentText(dlabel.c_str());
                         }

                         ui->motionday->setEnabled(true);

                         std::string day = getCurrentDayLabel();
                         if(day.find(dlabel) != std::string::npos)
                         {
                                hastoday = true;
                                current = j;
                         }

                         int ins_size = mday->instance_size();

                         if (ins_size>0)
                         {

                             ui->remote_directory->setEnabled(true);
                             ui->collapse->setEnabled(true);
                             ui->expand->setEnabled(true);
                             ui->getxml->setEnabled(true);

                             for (int k = 0; k < ins_size; k++)
                             {

                                motion::Message::Instance * ins = mday->mutable_instance(k);

                                QTreeWidgetItem *parentTreeItem = new QTreeWidgetItem(treeWidget);
                                google::protobuf::uint32 insid = ins->idinstance();

                                google::protobuf::uint32 insnum = ins->number();
                                cout << "INS NUMBER: " << insnum << endl;
                                QString instancenumber = QString::number(insnum);

                                parentTreeItem->setText(0, instancenumber);
                                QString start = QString::fromStdString(ins->instancestart());
                                parentTreeItem->setText(1, start);

                                motion::Message::Video * video = ins->mutable_video();
                                std::string vname = video->name();
                                std::string vpath = video->path();
                                std::string vfile = video->path() + "/" + video->name();

                                ListItems *litems = new ListItems();
                                litems->name = vname;
                                litems->path = vpath;
                                litems->file = vfile;
                                QVariant data;
                                data.setValue(litems);
                                parentTreeItem->setData(0, Qt::UserRole, data);

                                if (ins->has_instanceend())
                                {
                                    std::string insend = ins->instanceend();
                                    QString end = QString::fromStdString(ins->instanceend());
                                    parentTreeItem->setText(2, end);
                                    cout << "amount: " << ins->idinstance() << endl;
                                    cout << "id: " << ins->idinstance() << endl;
                                }

                                int imgsize = ins->image_size();
                                if (imgsize>0)
                                     for (int j = 0; j < imgsize; j++)
                                     {
                                         motion::Message::Image * img = ins->mutable_image(j);
                                         string path = img->path();
                                         vector<string> paths = MainWindow::splitProto(path, '/');
                                         QTreeWidgetItem *imageItem = new QTreeWidgetItem();
                                         imageItem->setTextAlignment ( 0, Qt::AlignLeft);
                                         imageItem->setText(0, paths.at(paths.size()-1).c_str());
                                         imageItem->setText(3, QString::fromStdString(path));
                                         parentTreeItem->addChild(imageItem);
                                         imageItem->setFirstColumnSpanned(true);
                                     }
                                }
                          }
                     }
                 }
             }
        }
    }
}

void MainWindow::dayComboChange(const QString &arg)
{
    /*QTreeWidget *treeWidget = ui->remote_directory;

    motion::Message::MotionCamera * mcam = PROTO.mutable_motioncamera(PROTO.activecam());

    int sizem = mcam->motionmonth_size();

    for (int i = 0; i < sizem; i++)
    {
        const motion::Message::MotionMonth * mmonth = mcam->mutable_motionmonth(i);

        int sized = mmonth->motionday_size();

        for (int j = 0; j < sized; j++)
        {

            const motion::Message::MotionDay & mday = mmonth->motionday(j);

            std::string dlabel = mday.daylabel();
            std::string dcombo = ui->motionday->currentText().toStdString();

            if (dlabel.find(dcombo))
            {
                MainWindow::loadInstancesByDay(ui->remote_directory, &mday);
            }
        }
    }*/
}

void MainWindow::onRecNameTextChanged()
{
    QString oldval = ui->rec->currentText();
    if(!ui->save_region->isEnabled())
    {
        ui->save_rec->setEnabled(true);
    }
    ui->rec_new->setEnabled(true);
}

void MainWindow::updateTime(motion::Message m)
{

    ui->remote_terminal_time->setText(m.time().c_str());

    const char *time_details = m.time().c_str();
    struct tm tm;
    strptime(time_details, "%Y-%m-%d %H:%M:%S %z", &tm);
    time_t t1 = mktime(&tm);

    struct timeval tv;
    struct tm* ptm;
    char time_string[25];

    gettimeofday (&tv, NULL);
    ptm = localtime (&tv.tv_sec);
    strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S %z", ptm);
    time_t t2 = mktime(ptm);

    time_string[25] = '\0';

    ui->computer_time->setText(time_string);

    double diff = difftime(t2, t1);
    int sec = ((diff + 500) / 1000);

    if (sec>20)
    {
        stringstream st;
        st << " Not synched!";
        ui->synched->setStyleSheet("QLabel { background-color : rgba( 255, 0, 0, 100% ); color : #FFFFFF; }");
        ui->synched->setText(st.str().c_str());
    } else
    {
        stringstream ss;
        ss << " Synched!";
        ui->synched->setStyleSheet("QLabel { background-color : rgba( 200, 200, 200, 100% ); color : #000000; }");
        ui->synched->setText(ss.str().c_str());
    }
}

void MainWindow::remoteProto(motion::Message remote)
{
      PROTO.Clear();
      PROTO = remote;

      int action = remote.type();
      switch (action)
      {

        case motion::Message::REFRESH:
        {
          MainWindow::engage_refresh(false);
        }
        break;
        case motion::Message::ENGAGE:
        {

            MainWindow::engage_refresh(true);
        }
        break;
        case motion::Message::GET_TIME:
        {
          MainWindow::updateTime(PROTO);
        }
        break;
        case motion::Message::TIME_SET:
        {
          MainWindow::on_get_time_clicked();
        }
        break;
        case motion::Message::GET_IMAGE:
        {

          std::string encoded_content = PROTO.data();
          std::string strdecoded = base64_decode(encoded_content);
          std::string imagedayfolder = folder_paths.at(3);
          std::string imagefilepath = PROTO.imagefilepath();

          vector<string> imgpath = MainWindow::splitString(imagefilepath, "/");
          std::string imagefilename = imgpath.at(imgpath.size()-1);

          std::string imgfile =  imagedayfolder + "/"  + imagefilename;

          cout << "imagefilename: " << imagefilename << endl;
          cout << "imagedayfolder: " << imagedayfolder << endl;
          cout << "imgfile: " << imgfile << endl;

          MainWindow::saveImage(strdecoded, imgfile);

          QString qimagefolder = QString::fromStdString(imagedayfolder);
          QStringList scriptArgs;
          scriptArgs << QLatin1String("-e")
                     << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
                                          .arg(qimagefolder);
          QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);

          i_spinner->stop();
          
        }
        break;
        case motion::Message::TAKE_PICTURE:
        {
            motion::Message::MotionCamera * mcamp = PROTO.mutable_motioncamera(PROTO.activecam());
            google::protobuf::uint32 matfile = mcamp->activemat();
            motion::Message::MotionCamera * mccamproto = PROTO.mutable_motioncamera(PROTO.activecam());
            mccamproto->set_activemat(matfile);
            MainWindow::saveMat(PROTO.data(), matfile);
            MainWindow::loadMat(matfile);
        }
        break;
        case motion::Message::GET_XML:
        {
            std::string encoded_content = PROTO.data();
            std::string strdecoded = base64_decode(encoded_content);
            std::string xmlfolder = folder_paths.at(2);
            //std::string xmldayfolder = xmlfolder + "/" + MainWindow::getCurrentDayLabel();
            std::string xmlfile =  xmlfolder + "/<import>session.xml";

            cout << "xmlfile:" << xmlfile << endl;

            std::ofstream outxml;
            outxml.open (xmlfile.c_str());
            outxml << strdecoded << "\n";
            outxml.close();

            QString qxmlfolder = QString::fromStdString(xmlfolder);
            QStringList scriptArgs;
            scriptArgs << QLatin1String("-e")
                       << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
                                            .arg(qxmlfolder);
            QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);

            i_spinner->stop();
        }
        break;
        case motion::Message::GET_VIDEO:
        {

          std::string encoded_content = PROTO.data();
          std::string strdecoded = base64_decode(encoded_content);
          std::string videofolder = folder_paths.at(4);
          std::string videofile = videofolder + "/" + video_name;

          cout << "videofile: " << videofile << endl;

          std::ofstream out;
          out.open (videofile.c_str());
          out << strdecoded << "\n";
          out.close();

          QString qvideofolder = QString::fromStdString(videofolder);
          QStringList scriptArgs;
          scriptArgs << QLatin1String("-e")
                     << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
                                          .arg(qvideofolder);
          QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);

          i_spinner->stop();
        }
        break;
        case motion::Message::SAVE_OK:
        {
            motion::Message::MotionCamera * mcamp = PROTO.mutable_motioncamera(PROTO.activecam());
            ui->start_recognition->setEnabled(true);
            bool recognizing = mcamp->recognizing();
            if (recognizing)
            {
                ui->start_recognition->setText("STOP");
                ui->start_recognition->setStyleSheet("QPushButton { background-color : #FF0000; color : #FFFFFF; border:6pxsolidwhite; }");
            } else
            {
                ui->start_recognition->setText("START");
                ui->start_recognition->setStyleSheet("QPushButton { background-color : #66CC00; color : #FFFFFF; border:6pxsolidwhite; }");
            }
            MainWindow::on_refresh_clicked();
        }
        break;
        case motion::Message::REC_START:
        {
            ui->start_recognition->setText("STOP");
            ui->start_recognition->setStyleSheet("QPushButton { background-color : #FF0000; color : #FFFFFF; border:6pxsolidwhite; }");
            ui->start_recognition->setChecked(true);
        }
        break;
        case motion::Message::REC_STOP:
        {
            ui->start_recognition->setText("START");
            ui->start_recognition->setStyleSheet("QPushButton { background-color : #66CC00; color : #FFFFFF; border:6pxsolidwhite; }");
            ui->start_recognition->setChecked(false);
        }
        break;
        case motion::Message::UPDATE_OK:
        {

        }
        break;
    }
}

void MainWindow::engage_refresh(bool engage)
{

    i_spinner->start();

    if (engage)
    { 
        MainWindow::updateTime(PROTO);
        ui->qt_drawing_output->ClearRegion();
    }

    std::string time = PROTO.time();
    MainWindow::updateTime(PROTO);

    int camsize = PROTO.motioncamera_size();
    const motion::Message::MotionCamera * mmactivecam;

    for (int i = 0; i < camsize; i++)
    {
        const motion::Message::MotionCamera & mmcam = PROTO.motioncamera(i);
        QString ccombo = mmcam.cameraname().c_str();
        int indexc = ui->cameracombo->findText(ccombo);
        if (indexc == -1)
        {
            ui->cameracombo->addItem(ccombo);
            int itemdata = mmcam.cameranumber();
            ui->cameracombo->itemData(itemdata);
            int pactivecam = PROTO.activecam();
            int camnum = mmcam.cameranumber();
            if (pactivecam==camnum)
            {
                mmactivecam = &mmcam;
            }
        }
    }

    int activecam = PROTO.activecam();
    const motion::Message::MotionCamera * mcam = PROTO.mutable_motioncamera(activecam);

    int sizee = mcam->motionmonth_size();
    if (sizee>0)
    {
        if (engage)
            MainWindow::enableDisableButtons(ENGAGED_INSTANCE);

        MainWindow::loadInstances(PROTO);

    } else
    {
        if (engage)
            MainWindow::enableDisableButtons(ENGAGED_NO_INSTANCE);
    }

    if (mcam->motionrec_size()>0)
    {
        MainWindow::populateRecCombo(mcam);
    }

    std::string recname;
    if (mcam->has_recname())
    {
        recname = mcam->recname();
        ui->rec_name->setText(recname.c_str());

        int indexn = ui->rec->findText(recname.c_str());
        if (indexn != -1)
        {
            ui->delay->setCurrentIndex(indexn);
            ui->delay->setCurrentText(recname.c_str());
        }
    }

    MainWindow::getTerminalFolder(PROTO);
    MainWindow::loadRecData(mcam);
    MainWindow::enableStartButton(mcam);

    i_spinner->stop();

}


void MainWindow::loadRecData(const motion::Message::MotionCamera * mcam)
{

    QString speed = QString::number(mcam->speed());
    int indexspeed = ui->speedcombo->findText(speed);
    if (indexspeed != -1)
    {
        ui->speedcombo->setCurrentIndex(indexspeed);
    }

    if (mcam->hascron())
    {
        ui->between->setChecked(true);
        ui->time_from->setEnabled(true);
        ui->time_to->setEnabled(true);

        QTime timestart(QTime::fromString(mcam->timestart().c_str(), "hh:mm:ss"));
        ui->time_from->setTime(timestart);
        QTime timestop(QTime::fromString(mcam->timeend().c_str(), "hh:mm:ss"));
        ui->time_to->setTime(timestop);
    } else
    {
        ui->between->setChecked(false);
        ui->time_from->setEnabled(false);
        ui->time_to->setEnabled(false);
    }

    std::string starttime = PROTO.devicestarttime();
    if (PROTO.has_devicestarttime())
        ui->status_label->setText(starttime.c_str());
    else
        ui->status_label->setText("N/A");

    std::string start = mcam->startrectime();
    if (mcam->has_startrectime())
        ui->instance_started->setText(start.c_str());
    else
        ui->instance_started->setText("N/A");

    //if (mcam->has_storeimage())
        ui->has_images->setChecked(true);
    //else
        //ui->has_images->setChecked(false);

    if (mcam->has_runatstartup())
    {
        ui->startup->setChecked(mcam->runatstartup());
    }

    if (mcam->has_codename())
        ui->codename->setText(mcam->codename().c_str());
    else
        ui->codename->setText("test");
    if (mcam->has_delay())
        ui->delay->setCurrentIndex(mcam->delay()-1);
    else
        ui->delay->setCurrentIndex(1);

    if (mcam->has_activemat())
    {
        google::protobuf::uint32 matint =  mcam->activemat();
        if (matint>0)
            MainWindow::loadMat(matint);
    }

    if (mcam->has_coordinates())
    {
        std::string coords = mcam->coordinates();
        vector<Point2f> scorrd = MainWindow::stringToVectorPoint2f(coords);
        ui->qt_drawing_output->drawLinesSlot(scorrd);
    }

    if (mcam->has_lastinstance())
    {
        std::string last = mcam->lastinstance();
    }

    if(mcam->has_timestart())
    {
        QString timef = mcam->timestart().c_str();
        QTime tf = QTime::fromString(timef);
        ui->time_from->setTime(tf);
     } else
    {
        QTime timefrom(9, 0);
        ui->time_from->setTime(timefrom);
    }

    if(mcam->has_timeend())
    {
        QString timet = mcam->timeend().c_str();
        QTime tt = QTime::fromString(timet);
        ui->time_to->setTime(tt);

    } else
    {
        QTime timeto(18, 0);
        ui->time_to->setTime(timeto);
    }

    MainWindow::enableStartButton(mcam);

    ui->engage_button->setDisabled(true);

    if (ui->mapdrive->isChecked())
    {
        mount_thread->MountNetWorkDrive(ui->ips_combo->currentText());
        mount_thread->terminate();
    }

}

void MainWindow::populateRecCombo(const motion::Message::MotionCamera * mmactivecam)
{
    if (mmactivecam)
    {
        int sizerec = mmactivecam->motionrec_size();
        if (sizerec>0)
        {
            ui->rec->setEnabled(true);
            for (int l = 0; l < sizerec; l++)
            {
                const motion::Message::MotionRecognition & mrec = mmactivecam->motionrec(l);
                QString ccombo = mrec.name().c_str();
                int indexr = ui->rec->findText(ccombo);
                if (indexr == -1)
                {
                    ui->rec->addItem(ccombo);
                    int itemdata = mrec.db_idrec();
                    ui->rec->itemData(itemdata);
                    QString rec = mmactivecam->recname().c_str();
                    int isrec = QString::compare(ccombo, rec, Qt::CaseInsensitive);
                    if (isrec==0)
                    {
                        ui->rec->setCurrentIndex(indexr);
                        ui->rec->setCurrentText(ccombo);
                    }
                }
            }
        }
    }
}


void MainWindow::enableStartButton(const motion::Message::MotionCamera * mcam)
{
    //Enable START button.
    ui->start_recognition->setEnabled(true);
    bool recognizing = mcam->recognizing();
    if (recognizing)
    {
        ui->start_recognition->setEnabled(true);
        ui->start_recognition->setText("STOP");
        ui->start_recognition->setStyleSheet("QPushButton { background-color : #FF0000; color : #FFFFFF; border:6pxsolidwhite; }");

    } else
    {
        if (mcam->hasrecjob())
        {
            ui->start_recognition->setEnabled(true);
            ui->start_recognition->setText("START");
            ui->start_recognition->setStyleSheet("QPushButton { background-color : #66CC00; color : #FFFFFF; border:6pxsolidwhite; }");
        } else
        {
            ui->start_recognition->setEnabled(false);
        }
    }
}


void MainWindow::receivedEcho(motion::Message m)
{

    int action = m.type();
    switch (action)
    {
        case motion::Message::REC_HAS_CHANGES:
        {
            //google::protobuf::uint32 nchanges = m.numberofchanges();
            //QString nc = QString::number(nchanges);
            //ui->amount_detected->setText(nc);
            //std:string startrecognition = m.startrecognition();
            //ui->time_detected->setText(m.time().c_str());
        }
        break;
     }
     google::protobuf::ShutdownProtobufLibrary();
}


void MainWindow::resutlEcho(string str)
{

    tcpsend_thread->terminate();

    //cout << str << endl;

    string decoded_proto;

    std::string del_1  = "PROSTA";
    std::string del_2  = "PROSTO";

    std::string strdecoded;
    int total__packages = 0;
    int current_package = 0;
    int current____type = 0;
    int proto_has_files = 0;
    int package____size = 0;

    int total_size = str.size();
    std::size_t found = str.find(del_1);

    if (found!=std::string::npos)
    {

      std::string lpay = MainWindow::ExtractString(str, del_1, del_2);

      vector<string> vpay = MainWindow::splitProto(lpay, '::');

      package____size = atoi(vpay.at(0).c_str());
      total__packages = atoi(vpay.at(2).c_str());
      current_package = atoi(vpay.at(4).c_str());
      current____type = atoi(vpay.at(6).c_str());
      proto_has_files = atoi(vpay.at(8).c_str());

      int del_pos = str.find(del_2);
      int until = str.size() - 40;

      const string & ppayload = str.substr((del_pos+del_2.size()), until);

      //cout << "------------------------" << endl;
      //cout << "ppayload: " << ppayload << endl;

      payload_holder.push_back(ppayload);

      int payloadsize = ppayload.size();

      switch (current____type)
      {
           case motion::Message::TAKE_PICTURE:
           {
              if (current_package==0)
              {
                  ui->mat_progress->setMinimum(0);
                  ui->mat_progress->setMaximum(total__packages);
              }
              ui->mat_progress->setValue(current_package);
              QApplication::processEvents();
           }
           break;
           case motion::Message::GET_IMAGE:
           case motion::Message::ENGAGE:
           case motion::Message::REFRESH:
           case motion::Message::GET_XML:
           case motion::Message::GET_VIDEO:
           {
             if (current_package==0)
             {
                 ui->xml_progress->setMinimum(0);
                 ui->xml_progress->setMaximum(total__packages);
                 i_spinner->stop();
             }
             ui->xml_progress->setValue(current_package);
             QApplication::processEvents();
           }
           break;
       }

       if (total__packages==(current_package+1))
       {
           finished = true;
       }
    }

    if (!finished)
    {
        sendSocket(packagesize, c_str_ip, reply_next_proto);
    }
    else
    {

        std::string payload;
        for (int j=0; j<payload_holder.size(); j++)
        {
            payload += payload_holder.at(j);
        }
        payload_holder.clear();

        //cout << "------------------------" << endl;
        //cout << "payload: " << payload << endl;

        std::string del_3 = "PROFILE";
        std::string filestr;
        if (proto_has_files==motion::Message::PROTO_HAS_FILE)
        {
            int file_pos = payload.find(del_3);
            const string & protostr = payload.substr(0, file_pos);
            int untilfile = payload.size(); // - (protostr.size() + 40);
            const string & fstr = payload.substr((file_pos+del_3.size()), untilfile);
            payload = protostr;
            filestr = fstr;
        }


        //cout << "//////////////////////" << endl;
        //cout << "filestr: " << filestr << endl;

        //std::string base_proto_file = "/jose/repos/motion/PROTO.txt";
        //std::ofstream out;
        //out.open (base_proto_file.c_str());
        //out << filestr << "\n";
        //out.close();

        //std::string base_mat_file = "/jose/repos/motion/MAT.txt";
        //out.open (base_mat_file.c_str());
        //out << filestr << "\n";
        //out.close();

        std::string oridecoded = base64_decode(payload);

        motion::Message mm;

        try
        {
            mm.ParseFromArray(oridecoded.c_str(), oridecoded.size());
        }
        catch (google::protobuf::FatalException fe)
        {
            std::cout << "PbToZmq " << fe.message() << std::endl;
        }

        if (proto_has_files==motion::Message::PROTO_HAS_FILE)
        {
            mm.set_data(filestr.c_str());
        }

        remoteProto(mm);

        switch (current____type)
        {
             case motion::Message::TAKE_PICTURE:
             {
                current_package=0;
                ui->mat_progress->setValue(0);
                QApplication::processEvents();
             }
             break;
             case motion::Message::GET_IMAGE:
             case motion::Message::ENGAGE:
             case motion::Message::REFRESH:
             case motion::Message::GET_XML:
             {
                current_package=0;
                ui->xml_progress->setValue(0);
                QApplication::processEvents();
             }
             break;
        }
        ui->picture->setChecked(false);
        finished=false;

    }
}

void MainWindow::on_remote_directory_itemClicked(QTreeWidgetItem *item, int column)
{
    QString data = item->text(3);

    std::string datastr = data.toStdString();

    if (datastr.empty())
    {
        QVariant idata = item->data(0, Qt::UserRole); // get the data from column 0
        items = idata.value<ListItems*>();
        std::string vname = items->name;
        std::string vfile = items->file;
        std::string vpath = items->path;
        cout << "vname: " << vname << endl;
        cout << "vfile: " << vfile << endl;
        cout << "vpath: " << vpath << endl;
        ui->watch_image->setEnabled(false);
        ui->watch_video->setEnabled(true);
    } else
    {
       ui->watch_image->setEnabled(true);
       ui->watch_video->setEnabled(false);
       imagepath = datastr;
    }
}

void MainWindow::on_watch_image_clicked()
{
    motion::Message m;
    m.set_imagefilepath(imagepath);
    m.set_type(motion::Message::ActionType::Message_ActionType_GET_IMAGE);
    setMessageBodyAndSend(m);
    google::protobuf::ShutdownProtobufLibrary();
}


void MainWindow::on_between_clicked(bool checked)
{
    ui->time_from->setEnabled(checked);
    ui->time_to->setEnabled(checked);
}

void MainWindow::on_rec_activated(const QString &rec)
{
    MainWindow::on_engage(rec);
}

void MainWindow::on_cameracombo_activated(const QString &camera)
{
    QString none;
    MainWindow::on_engage(none);
}


