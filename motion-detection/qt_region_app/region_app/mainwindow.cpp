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

Mat src;
string filename, xml;
Scalar color(0,0,255); // red color
vector<Point2f> coor;
int coor_num = 0;

MainWindow::MainWindow(QWidget *parent) :

    QMainWindow(parent),
    ui(new Ui::MainWindow)

{
    ui->setupUi(this);
    ui->capture_video->setCheckable(true);

    //BroadCast Sockets:
    broadcast_thread = new BroadcastThread(this);
    connect(broadcast_thread, SIGNAL(BroadcastReceived(QString)), this, SLOT(BroadcastReceived(QString)));
    connect(broadcast_thread, SIGNAL(BroadcastTimeoutSocketException()), this, SLOT(broadcastTimeoutSocketException()));

    //Mount Shares
    mount_thread = new MountThread(this);
    connect(mount_thread, SIGNAL(SharedMounted(QString)), this, SLOT(SharedMounted(QString)));

    //Streaming Resutl
    streaming_thread = new StreamingThread(this);
    connect(streaming_thread, SIGNAL(StreamingUpdateLabelImage(std::string, Mat)), this, SLOT(StreamingUpdateLabelImage(std::string, Mat)));

    //Mouse Events
    connect(ui->qt_drawing_output, SIGNAL(sendMousePosition(QPoint)), this, SLOT(showMousePosition(QPoint&)));

    connect(ui->qt_drawing_output, SIGNAL(Mouse_Pos()), this, SLOT(Mouse_current_pos()));
    connect(ui->qt_drawing_output, SIGNAL(Mouse_Pressed()), this, SLOT(Mouse_pressed()));
    connect(ui->qt_drawing_output, SIGNAL(Mouse_Pressed_Right_Click(std::vector<cv::Point2f>&)), this, SLOT(Mouse_Pressed_Right_Click(std::vector<cv::Point2f>&)));
    connect(ui->qt_drawing_output, SIGNAL(Mouse_Left()), this, SLOT(Mouse_left()));

    ui->connect_button->setEnabled(false);
    ui->list_folders->setEnabled(false);
    ui->list_files->setEnabled(false);
    ui->refresh_results->setEnabled(false);
    ui->start_recognition->setEnabled(false);
    //ui->save_region->setEnabled(false);
    ui->rec_with_images->setEnabled(false);
    ui->rec_with_cropped->setEnabled(false);

    //ui->web_results->setEnabled(false);
    //ui->webView->setEnabled(false);
    //ui->webView->setVisible(false);

    ui->output->setStyleSheet("background-color: rgba( 200, 200, 200, 100% );");

    QPixmap pix_playst(":/img/playstatic.png");
    QIcon icon_playst(pix_playst);
    ui->capture_video->setIcon(icon_playst);
    ui->capture_video->setIconSize(pix_playst.size());
    ui->capture_video->setCheckable(true);
    //ui->capture_video->setStyleSheet( "background-color: rgba( 255, 255, 255, 0% );" );
    ui->capture_video->setEnabled(false);


    QPixmap pix_stopst(":/images/img/stopstatic.png");
    QIcon icon_stopst(pix_stopst);

    /*ui->capture_stop->setIcon(icon_stopst);
    ui->capture_stop->setIconSize(pix_stopst.size());
    ui->capture_stop->setStyleSheet( "background-color: rgba( 255, 255, 255, 0% );" );
    ui->capture_stop->setEnabled(false);*/

    /* This spinner will be embedded in a layout. For the modal, centred spinner,
     * see "launchBlockingSpinner" below*/

    m_spinner = new QtWaitingSpinner(this);
    QVBoxLayout *spinnerLayout = new QVBoxLayout;
    spinnerLayout->insertWidget(0, m_spinner);
    spinnerLayout->insertStretch(0);
    spinnerLayout->addStretch();
    m_spinner->setNumberOfLines(12);
    m_spinner->setLineLength(11);
    m_spinner->setLineWidth(2);
    m_spinner->setInnerRadius(4);
    ui->spinnerframe->insertLayout(1, spinnerLayout);

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

    if (response.compare(getGlobalIntToString(PAUSE_STREAMING)) == 0)
    {
        QPixmap pixar = QPixmap::fromImage(last_stored_frame);
        ui->output->setPixmap(pixar);

        streaming_thread->StopStreaming();
        streaming_thread->terminate();

    }
    else if (response.compare(getGlobalIntToString(CONNECT)) == 0)
    {
        ui->capture_video->setIcon(QIcon(":/img/play.png"));
        ui->capture_video->setEnabled(true);
        ui->start_recognition->setEnabled(true);
        ui->status_label->setText("connected");
        ui->status_label->setStyleSheet("background-color: lightgreen;");
        ui->refresh_results->setEnabled(true);

        mount_thread->MountNetWorkDrive(ui->ips_combo->currentText());


    }
    else if (response.compare(getGlobalIntToString(STOP_STREAMING)) == 0)
    {
        ui->qt_drawing_output->setStyleSheet("background-color: rgba( 200, 200, 200, 100% );");

        QPixmap pix_play(":/img/play.png");
        QIcon icon_play(pix_play);
        ui->capture_video->setIcon(icon_play);
        ui->capture_video->setIconSize(pix_play.size());
        ui->capture_video->setEnabled(true);
        ui->capture_video->setText("Stream Video");

        QPixmap pix_stopst(":/img/stopstatic.png");
        QIcon icon_stopst(pix_stopst);
        //ui->capture_stop->setIcon(icon_stopst);
        //ui->capture_stop->setIconSize(pix_stopst.size());
        //ui->capture_stop->setEnabled(false);

        ui->output->setStyleSheet("background-color: rgba( 200, 200, 200, 100% );");

    }

    tcpecho_thread->terminate();

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
    //QPixmap pixar = QPixmap::fromImage(frame);

    //src = imread(path, CV_LOAD_IMAGE_COLOR);

    QPixmap pixmap((QString::fromStdString(path)));
    ui->output->setPixmap(pixmap);
    //last_stored_frame = frame;
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

    time_t currentTime;
    struct tm *localTime;

    time( &currentTime );                   // Get the current time
    localTime = localtime( &currentTime );  // Convert the current time to the local time

    int Day     ,
        Month   ,
        Year    ,
        Hour    ,
        Min     ,
        Sec     ;

    Day    = localTime->tm_mday;
    Month  = localTime->tm_mon + 1;
    Year   = localTime->tm_year + 1900;
    Hour   = localTime->tm_hour;
    Min    = localTime->tm_min;
    Sec    = localTime->tm_sec;

    std::cout << "Message sent at: " << Hour << ":" << Min << ":" << Sec << std::endl;
    std::cout << "And the current date is: " << Day << "/" << Month << "/" << Year << std::endl;
    std::cout << "+++++++++++++++++++++++" << endl;

    spinner_folders->start();

    tcpecho_thread = new TCPEchoThread(this);
    connect(tcpecho_thread, SIGNAL(ResultEcho(string)), this, SLOT(ResultEcho(string)));

    QString qt_ip = ui->ips_combo->currentText();
    QByteArray ba_ip = qt_ip.toLatin1();
    char *c_str_ip = ba_ip.data();

    tcpecho_thread->SendEcho(c_str_ip, getGlobalIntToString(CONNECT));
}

void MainWindow::on_refresh_results_clicked()
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


    /*if( remove( region_file ) != 0 )
    {
        fstream f;
        f.open( region_file, ios::out );
        f << flush;
        f.close();
    }*/

    /*QFile file(region_file);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        cout << "file not created.\n";
    }*/



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

    //if(ui->capture_video->isChecked())
    //{
        //QPixmap pix_pause(":/img/pause.png");
        //QIcon icon_pause(pix_pause);
        //ui->capture_video->setIcon(icon_pause);
        //ui->capture_video->setIconSize(pix_pause.size());
        //ui->capture_video->setEnabled(true);
        //ui->capture_video->setText("Pause Stream");

        //QPixmap pix_stopst(":/img/stop.png");
        //QIcon icon_stopst(pix_stopst);

        /*ui->capture_stop->setIcon(icon_stopst);
        ui->capture_stop->setIconSize(pix_stopst.size());
        ui->capture_stop->setEnabled(true);*/

        QString qt_ip = ui->ips_combo->currentText();
        QByteArray ba_ip = qt_ip.toLatin1();
        char *c_str_ip = ba_ip.data();
        QString sh = getSharedFolder();
        //streaming_thread->StartStreaming(c_str_ip, STREAMING_VIDEO_PORT, sh);

        streaming_thread->StartStreaming(c_str_ip, qt_ip, sh);

        //ui->output->setStyleSheet("background-color: rgba( 255, 255, 255, 0% );");

    //}
    //else
    //{
        // Pause Streaming
        //QPixmap pix_play(":/img/play.png");
        //QIcon icon_play(pix_play);
        //ui->capture_video->setIcon(icon_play);
        //ui->capture_video->setIconSize(pix_play.size());
        //ui->capture_video->setEnabled(true);
        //ui->capture_video->setText("Stream Video");

        //std::cout << "PAUSE" << std::endl;

        //streaming_thread->StopStreaming();
        //streaming_thread->terminate();

        /*std::string command = getGlobalIntToString(PAUSE_STREAMING);
        tcpecho_thread = new TCPEchoThread(this);
        connect(tcpecho_thread, SIGNAL(ResultEcho(string)), this, SLOT(ResultEcho(string)));
        tcpecho_thread->SendEcho(getActiveTerminalIPString(), command);*/

    //}
}


void MainWindow::on_capture_video_clicked()
{


}

/*void MainWindow::on_capture_stop_clicked()
{
    std::string command = getGlobalIntToString(STOP_STREAMING);
    tcpecho_thread = new TCPEchoThread(this);
    tcpecho_thread->SendEcho(getActiveTerminalIPString(), command);
}*/

/*void MainWindow::on_view_webview_clicked()
{
   // ui->webView->load(QUrl(lineEdit->text("http://google.com")));
}*/

void MainWindow::on_start_recognition_toggled(bool checked)
{

    if (checked){

        std::string command = getGlobalIntToString(START_RECOGNITION);
        tcpecho_thread = new TCPEchoThread(this);
        connect(tcpecho_thread, SIGNAL(ResultEcho(string)), this, SLOT(ResultEcho(string)));
        tcpecho_thread->SendEcho(getActiveTerminalIPString(), command);

        /*QStateMachine *machine = new QStateMachine;

        QState *state1 = new QState(machine);
        state1->assignProperty(button, "geometry", QRect(0, 0, 100, 30));
        machine->setInitialState(state1);

        QState *state2 = new QState(machine);
        state2->assignProperty(button, "geometry", QRect(250, 250, 100, 30));

        QSignalTransition *transition1 = state1->addTransition(button,
            SIGNAL(clicked()), state2);
        transition1->addAnimation(new QPropertyAnimation(button, "geometry"));

        QSignalTransition *transition2 = state2->addTransition(button,
            SIGNAL(clicked()), state1);
        transition2->addAnimation(new QPropertyAnimation(button, "geometry"));

        machine->start();*/


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

void MainWindow::showMousePosition( QPoint & pos )
{
    ui->xy_pos->setText("x: " + QString::number(pos.x()) + " y : "  + QString::number(pos.y()) );
}

void MainWindow::Mouse_pressed()
{
    cout << "PRESSED at (x,t): " <<  ui->capture_video->x() << " " << ui->capture_video->y() << endl;
}

void MainWindow::Mouse_current_pos()
{

}

void MainWindow::Mouse_left()
{

}


void MainWindow::on_start_recognition_clicked()
{

}

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

void MainWindow::on_list_files_clicked(const QModelIndex &index)
{

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


void MainWindow::split(const string& s, char c,
           vector<string>& v) {
   string::size_type i = 0;
   string::size_type j = s.find(c);

   while (j != string::npos) {
      v.push_back(s.substr(i, j-i));
      i = ++j;
      j = s.find(c, j);

      if (j == string::npos)
         v.push_back(s.substr(i, s.length()));
   }
}





