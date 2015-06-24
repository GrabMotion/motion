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

#include <pthread.h>

using namespace google::protobuf::io;
using namespace std;
using namespace cv;

Mat main_mat;
std::string filename, xml;
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
    //socket_listener = new SocketListener(this);
    //socket_listener->startListening(this);

    //Stream Listener Class -- Cannot connect received
    stream_listener = new StreamListener(this);
    stream_listener->startListening(this);

    //Mount Shares
    mount_thread = new MountThread(this);
    connect(mount_thread, SIGNAL(SharedMounted(QString)), this, SLOT(SharedMounted(QString)));

    //Streaming Resutl
    //streaming_thread = new StreamingThread(this);
    //connect(streaming_thread, SIGNAL(StreamingUpdateLabelImage(std::string, Mat)), this, SLOT(StreamingUpdateLabelImage(std::string, Mat)));

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

    ui->screenshot->setEnabled(false);
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

    //ui->ips_combo->addItem("192.168.1.47"); //208.70.188.15");
    //ui->connect_button->setEnabled(true);

    //testBase();
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

    GOOGLE_PROTOBUF_VERIFY_VERSION;

    int action;
    motion::Message m;
    m.ParseFromString(response);

    action = m.type();
    std::string payload;
    QString qpayload;
    std::string mtime;

    if (m.has_payload())
    {
        payload = m.payload();
        qpayload = QString::fromStdString(payload);
    }

    if (m.has_time())
    {
       mtime  = m.time();
    }

    std::cout << "Action received:: " << action << " time: " << mtime << std::endl;

    switch (action)
    {
        case motion::Message::CONNECT:
            ui->start_recognition->setEnabled(true);
            ui->status_label->setText(qpayload);
            ui->status_label->setStyleSheet("background-color: lightgreen;");
            ui->screenshot->setEnabled(true);
            mount_thread->MountNetWorkDrive(ui->ips_combo->currentText());
            break;


    }

    /*if (response.compare(getGlobalIntToString(CONNECT)) == 0)
    {


    }
    else if (response.compare(getGlobalIntToString(GET_TIME)) == 0)
    {

    }*/

    google::protobuf::ShutdownProtobufLibrary();

    tcpecho_thread->terminate();
    mount_thread->terminate();
}

char* convert2char(QString input)
{
    return (char*)input.data();
}

void MainWindow::setRemoteMessage(QString qstr)
{

    GOOGLE_PROTOBUF_VERIFY_VERSION;

    std::string response = qstr.toUtf8().constData();

    int action;
    motion::Message mm;
    mm.ParseFromString(response);

    action = mm.type();

    //char* str = convert2char(qstr);
    //int action;
    //motion::Message mm;
    //mm.ParseFromArray(&str, sizeof(str));

    if (mm.has_type())
    {
        action = mm.type();
    }

    std::string payload;
    QString qpayload;
    std::string mtime;

    if (mm.has_payload())
    {
        payload = mm.payload();
        qpayload = QString::fromStdString(payload);
    }

    if (mm.has_time())
    {
       mtime  = mm.time();
    }

    std::cout << "Action received:: " << action << " time: " << mtime << std::endl;

    switch (action)
    {
        case motion::Message::SET_MAT:
            motion::Message mbytes;
            //mbytes.ParseFromArray(response.data(), response.size());
            cv::Mat data_mat;
            if(mm.ByteSize() > 0)
            {
                std::string mdata = mm.data();
                typedef unsigned char byte;
                std::vector<byte> vectordata(mdata.begin(),mdata.end());
                cv::Mat data_mat(vectordata,true);
                QImage frame = Mat2QImage(data_mat);
                //QPixmap pixmap((QString::fromStdString(path)));
                ui->output->setPixmap(QPixmap::fromImage(frame));
            }
            break;
    }

    /*
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
    */

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

    /*switch(action)
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
    }*/


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

/*void MainWindow::StreamingUpdateLabelImage(std::string path, Mat mat)
{
    src = mat;
    QPixmap pixmap((QString::fromStdString(path)));
    ui->output->setPixmap(pixmap);
}*/

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

#define MAXDATASIZE 20

void MainWindow::on_connect_button_clicked()
{

    spinner_folders->start();

    tcpecho_thread = new TCPEchoThread(this);
    connect(tcpecho_thread, SIGNAL(ResultEcho(string)), this, SLOT(ResultEcho(string)));

    QString qt_ip = ui->ips_combo->currentText();
    QByteArray ba_ip = qt_ip.toLatin1();
    std::string qt_ip_str = qt_ip.toUtf8().constData();
    char *c_str_ip = ba_ip.data();

    //char buf[MAXDATASIZE];
    string data;
    motion::Message m;
    m.set_type(motion::Message::ActionType::Message_ActionType_CONNECT);
    m.set_serverip(qt_ip_str);
    m.SerializeToString(&data);
    char bts[data.length()];
    strcpy(bts, data.c_str());

    tcpecho_thread->SendEcho(c_str_ip, bts);

    tcpecho_thread->terminate();

    google::protobuf::ShutdownProtobufLibrary();

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

void MainWindow::ShareUmounted(){}

void MainWindow::on_screenshot_clicked()
{

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

    //QString sh = getSharedFolder();
    //streaming_thread->StartStreaming(c_str_ip, qt_ip, sh);

    tcpecho_thread = new TCPEchoThread(this);
    //dataconnect(tcpecho_thread, SIGNAL(ResultEcho(string)), this, SLOT(ResultEcho(string)));

    std::string qt_ip_str = qt_ip.toUtf8().constData();
    QByteArray ba_ip = qt_ip.toLatin1();
    char *c_str_ip = ba_ip.data();

    //char buf[MAXDATASIZE];

    string data;
    motion::Message m;
    m.set_type(motion::Message::ActionType::Message_ActionType_GET_MAT);
    m.set_serverip(getIpAddress());
    m.SerializeToString(&data);
    char bts[data.length()];
    strcpy(bts, data.c_str());

    tcpecho_thread->SendEcho(c_str_ip, bts);

    google::protobuf::ShutdownProtobufLibrary();

    tcpecho_thread->terminate();

}

void MainWindow::on_disconnect_clicked()
{
    std::string command = getGlobalIntToString(DISSCONNECT);
    tcpecho_thread = new TCPEchoThread(this);
    connect(tcpecho_thread, SIGNAL(ResultEcho(string)), this, SLOT(ResultEcho(string)));
    tcpecho_thread->SendEcho(getActiveTerminalIPString(), command);
    tcpecho_thread->terminate();
}

void MainWindow::on_get_time_clicked()
{
    std::string command = getGlobalIntToString(GET_TIME);
    tcpecho_thread = new TCPEchoThread(this);
    connect(tcpecho_thread, SIGNAL(ResultEcho(string)), this, SLOT(ResultEcho(string)));
    tcpecho_thread->SendEcho(getActiveTerminalIPString(), command);

}

/////////////SAVE_XML////////////////

void MainWindow::savePointsAsXML(vector<Point2f> &contour ){

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

    // Declare a printer
    TiXmlPrinter printer;

    // attach it to the document you want to convert in to a std::string
    doc.Accept(&printer);

    // Create a std::string and copy your document data in to the string
    std::string xml_temp = printer.CStr();

    std::string xmldecoded = base64_encode(reinterpret_cast<const unsigned char*>(xml_temp.c_str()), xml_temp.length());

    xmlstring = QString::fromStdString(xmldecoded);

}

/////////////MOUSE////////////////

void MainWindow::Mouse_Pressed_Right_Click(vector<Point2f>&coor)
{
    vector<Point2f> & contour = coor;
    vector<Point2f> insideContour;

    for(int j = 0; j < main_mat.rows; j++){
        for(int i = 0; i < main_mat.cols; i++){
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

    for(int j = 0; j < main_mat.rows; j++)
    {
        for(int i = 0; i < main_mat.cols; i++)
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

void MainWindow::on_start_recognition_toggled(bool checked)
{

    if (checked)
    {
        GOOGLE_PROTOBUF_VERIFY_VERSION;

        string data;
        motion::Message mr;
        mr.set_type(motion::Message::ActionType::Message_ActionType_START_RECOGNITION);

        struct timeval tv;
        struct tm* ptm;
        char time_string[40];
        gettimeofday (&tv, NULL);
        ptm = localtime (&tv.tv_sec);
        strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S", ptm);
        mr.set_time(time_string);

        mr.set_serverip(getIpAddress());
        std::string utf8_xml = xmlstring.toUtf8().constData();
        cout << "utf8_xml: " << utf8_xml.size() << endl;
        mr.set_regiondata(utf8_xml);
        mr.set_storeimage(true);
        mr.set_storecrop(false);

        //mr.SerializeToString(&data);
        //char bts[data.length()];
        //strcpy(bts, data.c_str());

        stream_sender = new StreamSender(this);
        //connect(stream_sender, SIGNAL(ResultEcho(string)), this, SLOT(ResultEcho(string)));
        stream_sender->sendStream(getActiveTerminalIPString(), mr);
        stream_sender->terminate();

        delete ptm;
        google::protobuf::ShutdownProtobufLibrary();

    }
    else
    {
        std::string command = getGlobalIntToString(STOP_RECOGNITION);
        tcpecho_thread = new TCPEchoThread(this);
        connect(tcpecho_thread, SIGNAL(ResultEcho(string)), this, SLOT(ResultEcho(string)));
        tcpecho_thread->SendEcho(getActiveTerminalIPString(), command);
        tcpecho_thread->terminate();
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

    tcpecho_thread->terminate();
}

void MainWindow::SocketErrorMessage(QString &e)
{
    QMessageBox* msgBox 	= new QMessageBox();
    msgBox->setWindowTitle("Trying to connect with terminals");
    msgBox->setText(e);
    msgBox->setWindowFlags(Qt::WindowStaysOnTopHint);
    msgBox->show();

    /*if (msgBox == QMessageBox::Yes)
  {
    qDebug() << "Yes was clicked";
    QApplication::quit();
  } else {
    qDebug() << "Yes was *not* clicked";
  }*/

}

std::string get_file_contents(string filename)
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


void MainWindow::testBase()
{

    // The data we need to deserialize
    int width_d = 0;
    int height_d = 0;
    int type_d = 0;
    size_t size_d = 0;

    std::string basefile = "/jose/repos/base64oish_MAC.txt";
    string loaded = get_file_contents(basefile);

    std::string oridecoded = base64_decode(loaded);

    stringstream decoded;
    //std::stringstream decoded;
    decoded << oridecoded;

    // Read the width, height, type and size of the buffer
    decoded.read((char*)(&width_d), sizeof(int));
    decoded.read((char*)(&height_d), sizeof(int));
    decoded.read((char*)(&type_d), sizeof(int));
    decoded.read((char*)(&size_d), sizeof(int));

    // Allocate a buffer for the pixels
    char* data_d = new char[size_d];
    // Read the pixels from the stringstream
    decoded.read(data_d, size_d);

    // Construct the image (clone it so that it won't need our buffer anymore)
    Mat deserialized = Mat(height_d, width_d, type_d, data_d).clone();
    cout  << "::::::::::::::::::::::::::::::::::" << endl;
    cout  << "width: " << width_d << endl;
    cout  << "height: " << height_d << endl;
    cout  << "type: " << type_d << endl;
    cout  << "size: " << size_d << endl;

    // Delete our buffer
    delete[]data_d;

    google::protobuf::ShutdownProtobufLibrary();

    //Save image converted
    imwrite("/jose/repos/image__decoded__10000.jpg", deserialized);

    QImage frame = Mat2QImage(deserialized);
    ui->output->setPixmap(QPixmap::fromImage(frame));
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

void MainWindow::setRemoteProto(motion::Message payload)
{

      int action = payload.type();
      int size_init = payload.ByteSize();
      int size_data_primitive = payload.data().size();
      std::string mdata = payload.data();
      int size_encoded = mdata.size();

      //Write base64 to file for checking.
      std::string basefile = "/jose/repos/base64oish_MAC.txt";
      std::ofstream out;
      out.open (basefile.c_str());
      out << mdata << "\n";
      out.close();

      //Decode from base64
      std::string oridecoded = base64_decode(mdata);
      int ori_size = oridecoded.size();

      //cast to stringstream to read data.
      std::stringstream decoded;
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

      // Construct the image (clone it so that it won't need our buffer anymore)
      main_mat = cv::Mat(height_d, width_d, type_d, data_d).clone();

      //Render image.
      imwrite("/jose/repos/image_2.jpg", main_mat);
      QImage frame = Mat2QImage(main_mat);
      ui->output->setPixmap(QPixmap::fromImage(frame));

      //QMetaObject::invokeMethod(parent, "remoteImage", Q_ARG(QImage, frame));

      cout << "+++++++++++++++++RECEIVING PROTO+++++++++++++++++++"   << endl;
      cout << "Mat size   : " << main_mat.size                            << endl;
      cout << "Char type  : " << type_d                               << endl;
      cout << "Proto size : " << payload.size()                         << endl;
      cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++"  << endl;
      cout << "size_encoded           : " << size_encoded             << endl;
      cout << "ori_size               : " << ori_size                 << endl;
      cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++"  << endl;
      cout <<  endl;

      google::protobuf::ShutdownProtobufLibrary();

      // Delete our buffer
      delete[]data_d;
}


char * MainWindow::getTimeRasp()
{
    struct timeval tr;
    struct tm* ptmr;
    char time_rasp[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
    return time_rasp;
}
