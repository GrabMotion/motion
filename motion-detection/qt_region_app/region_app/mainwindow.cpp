#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPoint>
#include <QMouseEvent>
#include <QtGui>
#include <QtCore>
#include <QString>
#include <QMessageBox>
#include <vector>
#include <QFileSystemModel>
#include <QObject>
#include <QModelIndex>
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

using namespace std;
using namespace google::protobuf::io;

Mat src;
string filename, xml;
Scalar color(0,0,255); // red color
vector<Point2f> coor;
vector<Point2f> contour;
int coor_num = 0;
int count_clicks;

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

    //Socket Listener Class -- Cannot connect received
    socket_listener = new SocketListener(this);
    socket_listener->startListening(this);

    //Mount Shares
    mount_thread = new MountThread(this);
    connect(mount_thread, SIGNAL(SharedMounted(QString)), this, SLOT(SharedMounted(QString)));

    //Streaming Resutl
    streaming_thread = new StreamingThread(this);
    connect(streaming_thread, SIGNAL(StreamingUpdateLabelImage(std::string, Mat)), this, SLOT(StreamingUpdateLabelImage(std::string, Mat)));

    //Mouse Events
    connect(ui->qt_drawing_output, SIGNAL(sendMousePosition(QPoint&)), this, SLOT(showMousePosition(QPoint&)));

    connect(ui->qt_drawing_output, SIGNAL(Mouse_Pos()), this, SLOT(Mouse_current_pos()));
    connect(ui->qt_drawing_output, SIGNAL(Mouse_Pressed(std::vector<cv::Point2f>&)), this, SLOT(Mouse_pressed(std::vector<cv::Point2f>&)));
    connect(ui->qt_drawing_output, SIGNAL(Mouse_Pressed_Right_Click(std::vector<cv::Point2f>&)), this, SLOT(Mouse_Pressed_Right_Click(std::vector<cv::Point2f>&)));
    connect(ui->qt_drawing_output, SIGNAL(Mouse_Left()), this, SLOT(Mouse_left()));

    ui->connect_button->setEnabled(false);
    ui->list_folders->setEnabled(false);
    ui->list_files->setEnabled(false);
    ui->start_recognition->setEnabled(false);
    //ui->save_region->setEnabled(false);
    ui->rec_with_images->setEnabled(false);

    ui->output->setStyleSheet("background-color: rgba( 200, 200, 200, 100% );");

    ui->scrrenshot->setEnabled(false);
    ui->save_region->setEnabled(false);
    ui->start_recognition->setEnabled(false);

    m_spinner = new QtWaitingSpinner(this);
    QVBoxLayout *spinnerLayout = new QVBoxLayout;
    spinnerLayout->insertWidget(0, m_spinner);
    spinnerLayout->insertStretch(0);
    spinnerLayout->addStretch();
    m_spinner->setNumberOfLines(12);
    m_spinner->setLineLength(11);
    m_spinner->setLineWidth(2);
    m_spinner->setInnerRadius(4);
    ui->top_spinner->insertLayout(1, spinnerLayout);

    spinner_folders = new QtWaitingSpinner(this);
    QVBoxLayout *spinnerLayoutFolder = new QVBoxLayout;
    spinnerLayoutFolder->insertWidget(0, spinner_folders);
    spinnerLayoutFolder->insertStretch(0);
    spinnerLayoutFolder->addStretch();
    spinner_folders->setNumberOfLines(12);
    spinner_folders->setLineLength(11);
    spinner_folders->setLineWidth(2);
    spinner_folders->setInnerRadius(4);
    ui->f_spinner->insertLayout(1, spinnerLayoutFolder);

    QFont f( "Arial", 11, QFont::Bold);
    ui->remote_time_response->setFont(f);
    ui->remote_terminal_time->setFont(f);

    //Local Network
    MainWindow::getLocalNetwork();
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

int getGlobalStringToInt(std::string id){
   return atoi( id.c_str() );
}

void MainWindow::ResultEcho(string response)
{

    if (response.compare(getGlobalIntToString(CONNECT)) == 0)
    {
        ui->start_recognition->setEnabled(true);
        ui->status_label->setText("connected");
        ui->status_label->setStyleSheet("background-color: lightgreen;");
        ui->scrrenshot->setEnabled(true);
        mount_thread->MountNetWorkDrive(ui->ips_combo->currentText());

    }
    else if (response.compare(getGlobalIntToString(GET_TIME)) == 0)
    {

    }
    tcpecho_thread->terminate();
}

void MainWindow::setRemoteMessage(const char * str)
{

    google::protobuf::uint32 action;

    detection::Message main_message;
    fstream input(str, ios::in | ios::binary);
    main_message.ParseFromIstream(&input);

    for (int i = 0; i < main_message.motion_size(); i++)
    {
        detection::Motion main_motion = main_message.motion(i);
        google::protobuf::uint32 idMotion;
        idMotion = main_motion.idmotion();

        detection::Motion::Action main_action;

        for (int j = 0; j < main_motion.action_size(); j++)
        {
            detection::Motion::Action main_action = main_motion.action(j);
            action = main_action.idaction();
        }


    }

    google::protobuf::ShutdownProtobufLibrary();
    /*
    std::string message = str.toStdString();
    int value;

    if (str.size()>4)
    {
      std::string id_action = message.substr (0,4);
      value = atoi(id_action.c_str());
      result_message = message.substr (4,message.size());
    }
    else
    {
      value = atoi(message.c_str());
    }

    q_response = QString::fromStdString(result_message);
    */

    switch(action)
    {

        case TIME_SET:
            ui->remote_time_response->setText(q_response);
            break;

        case GET_TIME:
            ui->remote_terminal_time->setText(q_response);
            break;

        case AMOUNT_DETECTED:
            //refresh_results();
            ui->amount_detected->setText(q_response);
            break;
    }


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
    ui->connect_button->setEnabled(true);
}

void MainWindow::StreamingUpdateLabelImage(std::string path, Mat mat)
{
    src = mat;
    QPixmap pixmap((QString::fromStdString(path)));
    ui->output->setPixmap(pixmap);
}

void MainWindow::broadcastTimeoutSocketException()
{
    m_spinner->stop();
    QMessageBox* msgBox 	= new QMessageBox();
    msgBox->setWindowTitle("Trying to connect with terminals");
    msgBox->setText("No terminal found on the network.");
    msgBox->setWindowFlags(Qt::WindowStaysOnTopHint);
    msgBox->show();
    ui->connect_button->setEnabled(false);
}

std::string MainWindow::getActiveTerminalIPString()
{
    QString qs = ui->ips_combo->currentText();
    std::string utf8_text = qs.toUtf8().constData();
    return qs.toLocal8Bit().constData();
}

void MainWindow::on_connect_button_clicked()
{

    spinner_folders->start();

    tcpecho_thread = new TCPEchoThread(this);
    connect(tcpecho_thread, SIGNAL(ResultEcho(string)), this, SLOT(ResultEcho(string)));

    QString qt_ip = ui->ips_combo->currentText();
    QByteArray ba_ip = qt_ip.toLatin1();
    char *c_str_ip = ba_ip.data();

    string data;
    detection::Message main_message;
    detection::Motion main_motion = main_message.motion(0);
    detection::Motion::Action main_action = main_motion.action(0);
    main_action.set_type(main_motion.CONNECT);

    //demo::People p;
    //p.set_name("Hideto");
    //p.set_id(123);
    //p.set_email("hideto.bj@gmail.com");

    main_message.SerializeToString(&data);
    char message[data.length()];
    strcpy(message, data.c_str());

    tcpecho_thread->SendEcho(c_str_ip, message); //getGlobalIntToString(CONNECT));
}

void MainWindow::refresh_results()
{
    MainWindow::RefreshTreViewModel(treeViewPath, ipPath);
}

QString MainWindow::getSharedFolder()
{
    QDir rootDir;
    rootDir = QDir::currentPath();
    rootDir.cdUp();
    rootDir.cdUp();
    rootDir.cdUp();
    QString roStr = rootDir.absolutePath();
    QString roo = roStr + "/" + "shares";
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

    RefreshTreViewModel(share, rip);

    ui->list_files->setEnabled(true);

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

    xml = xmlfile;

    QFile * xmlFile = new QFile(region_file);
    /*if (QFile(region_file).exists())
    {
        QString path = region;
        QDir dir(path);
        dir.setNameFilters(QStringList() << "*.*");
        dir.setFilter(QDir::Files);
        foreach(QString dirFile, dir.entryList())
        {
            dir.remove(dirFile);
        }

    }*/
    if( ! xmlFile->open(QIODevice::WriteOnly) )
    {
      QMessageBox::warning(NULL, "Test", "Unable to open: " + region_file , "OK");
    }

    spinner_folders->stop();
}

QString MainWindow::getShare()
{
    return share;
}

void MainWindow::RefreshTreViewModel(QString roo, QString rip)
{
    dirModel = new QFileSystemModel(this);
    dirModel->setFilter(QDir::NoDotAndDotDot | QDir::AllDirs);
    dirModel->setRootPath(roo);

    ui->list_folders->setModel(dirModel);
    ui->list_folders->setRootIndex(dirModel->setRootPath(rip));
    ui->list_folders->hideColumn(1);
    ui->list_folders->hideColumn(2);
    ui->list_folders->header()->resizeSection(1, 100);
    ui->list_folders->setEnabled(true);

    fileModel = new QFileSystemModel(this);
    fileModel->setFilter(QDir::NoDotAndDotDot | QDir::Files);
    ui->list_files->setModel(fileModel);
}


void MainWindow::on_list_folders_clicked(const QModelIndex &index)
{
    QString sPath = fileModel->fileInfo(index).absoluteFilePath();
    ui->list_files->setRootIndex(fileModel->setRootPath(sPath));
}

void MainWindow::on_list_files_clicked(const QModelIndex &index)
{
    QString sPath = fileModel->fileInfo(index).absoluteFilePath();
    QPixmap pixmap(sPath);
    ui->remote_capture->setPixmap(pixmap);
}

void MainWindow::ShareUmounted()
{

}

void MainWindow::on_scrrenshot_clicked()
{

    QString qtip = ui->ips_combo->currentText();
    QString share = getSharedFolder();

    string scree_shot = share.toStdString() + "/"  + qtip.toStdString() +  "/screenshots/screen.jpg"; //region + "region.xml";
    string screenshots = share.toStdString() + "/"  + qtip.toStdString() +  "/screenshots"; //region + "region.xml";

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

    QString qt_ip = ui->ips_combo->currentText();
    QByteArray ba_ip = qt_ip.toLatin1();
    char *c_str_ip = ba_ip.data();
    QString sh = getSharedFolder();

    streaming_thread->StartStreaming(c_str_ip, qt_ip, sh);
}


void MainWindow::on_start_recognition_toggled(bool checked)
{

    if (checked){

        std::string command = getGlobalIntToString(START_RECOGNITION);
        tcpecho_thread = new TCPEchoThread(this);
        connect(tcpecho_thread, SIGNAL(ResultEcho(string)), this, SLOT(ResultEcho(string)));
        tcpecho_thread->SendEcho(getActiveTerminalIPString(), command);

    }
    else
    {
        std::string command = getGlobalIntToString(STOP_RECOGNITION);
        tcpecho_thread = new TCPEchoThread(this);
        connect(tcpecho_thread, SIGNAL(ResultEcho(string)), this, SLOT(ResultEcho(string)));
        tcpecho_thread->SendEcho(getActiveTerminalIPString(), command);
    }

}

void MainWindow::on_disconnect_clicked()
{
    std::string command = getGlobalIntToString(DISSCONNECT);
    tcpecho_thread = new TCPEchoThread(this);
    connect(tcpecho_thread, SIGNAL(ResultEcho(string)), this, SLOT(ResultEcho(string)));
    tcpecho_thread->SendEcho(getActiveTerminalIPString(), command);

}

void MainWindow::on_get_time_clicked()
{
    std::string command = getGlobalIntToString(GET_TIME);
    tcpecho_thread = new TCPEchoThread(this);
    connect(tcpecho_thread, SIGNAL(ResultEcho(string)), this, SLOT(ResultEcho(string)));
    tcpecho_thread->SendEcho(getActiveTerminalIPString(), command);

}

/////////////SAVE_XML////////////////

void savePointsAsXML(vector<Point2f> &contour ){

    TiXmlDocument doc;
    TiXmlDeclaration decl("1.0", "", "");
    doc.InsertEndChild(decl);
    for(int i = 0; i <= contour.size(); i++)
    {
        TiXmlElement point("point");
        point.SetAttribute("x",contour[i].x);
        point.SetAttribute("y",contour[i].y);
        doc.InsertEndChild(point);
    }
    if(doc.SaveFile(xml.c_str()))
        cout << "file saved succesfully.\n";
    else
        cout << "file not saved, something went wrong!\n";
}

/////////////MOUSE////////////////

void MainWindow::Mouse_Pressed_Right_Click(vector<Point2f>&coor)
{
    vector<Point2f> & contour = coor;
    vector<Point2f> insideContour;

    for(int j = 0; j < src.rows; j++){
        for(int i = 0; i < src.cols; i++){
            Point2f p(i,j);
            if(pointPolygonTest(contour,p,false) >= 0) // yes inside
                insideContour.push_back(p);
        }
    }
    cout << "# points inside contour: " << insideContour.size() << endl;
    savePointsAsXML(insideContour);
}

void MainWindow::on_save_region_clicked()
{
    //TODO
    //vector<Point2f> & contour = coor;
    coor = contour;
    vector<Point2f> insideContour;

    for(int j = 0; j < src.rows; j++)
    {
        for(int i = 0; i < src.cols; i++)
        {
            Point2f p(i,j);
            if(pointPolygonTest(contour,p,false) >= 0) // yes inside
                insideContour.push_back(p);
        }
    }
    cout << "# points inside contour: " << insideContour.size() << endl;
    savePointsAsXML(insideContour);
    count_clicks=0;
}


void MainWindow::showMousePosition( QPoint & pos )
{
    ui->label_xy->setText("x: " + QString::number(pos.x()) + " y : "  + QString::number(pos.y()) );
}

void MainWindow::Mouse_pressed(vector<Point2f>&coor)
{
    contour = coor;
    count_clicks++;
    if (count_clicks>2)
    {
        ui->save_region->setEnabled(true);
    }
    cout << "PRESSED at (x,t): " <<  ui->output->x() << " " << ui->output->y() << endl;
}

void MainWindow::Mouse_current_pos(){}
void MainWindow::Mouse_left(){}

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


std::string MainWindow::getIpAddress () {

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
            printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);

            strm << addressBuffer;
            address = strm.str();

        } else if (ifa->ifa_addr->sa_family == AF_INET6) { // check it is IP6
            // is a valid IP6 Address
            tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer);

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

void MainWindow::on_start_recognition_clicked(){}

void MainWindow::on_set_time_clicked()
{

    struct timeval tv;
    struct tm* ptm;
    char time_string[40];

    gettimeofday (&tv, NULL);
    ptm = localtime (&tv.tv_sec);
    strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S", ptm);

    cout << "time_string TIME :: " << time_string << endl;

    std::string command = getGlobalIntToString(SET_TIME);

    char * message = new char[command.size() + 1];
    std::copy(command.begin(), command.end(), message);
    message[command.size()] = '\0'; // don't forget the terminating 0

    char buffer[256];
    strncpy(buffer, message, sizeof(buffer));
    strncat(buffer, time_string, sizeof(buffer));

    cout << "buffer:: " << buffer << endl;

    tcpecho_thread = new TCPEchoThread(this);
    connect(tcpecho_thread, SIGNAL(ResultEcho(string)), this, SLOT(ResultEcho(string)));
    tcpecho_thread->SendEcho(getActiveTerminalIPString(), buffer);
}
