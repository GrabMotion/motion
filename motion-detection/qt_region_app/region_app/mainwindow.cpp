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

//Socket
//http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html

Mat main_mat;

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
    socket_listener = new SocketListener(this);
    socket_listener->startListening(this);

    //Video Streamig
    mat_listener = new MatListener(this);
    mat_listener->startListening(this);

    //Mount Shares
    mount_thread = new MountThread(this);
    connect(mount_thread, SIGNAL(SharedMounted(QString)), this, SLOT(SharedMounted(QString)));

    //Mouse Operations
    connect(ui->qt_drawing_output, SIGNAL(sendMousePosition(QPoint&)), this, SLOT(showMousePosition(QPoint&)));
    connect(ui->qt_drawing_output, SIGNAL(Mouse_Pos()), this, SLOT(Mouse_current_pos()));
    connect(ui->qt_drawing_output, SIGNAL(savedRegionResutl(QString)), this, SLOT(savedRegionResutl(QString)));

    enableDisableButtons(false);

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

    //Spinner
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

    ui->mat_progress->setValue(0);

    //ui->ips_combo->addItem("192.168.1.47"); //208.70.188.15");
    //ui->connect_button->setEnabled(true);

    //Tests
    //testBase();

    //std::string mat = "/jose/repos/motion/mat.txt";
    //string loaded = MainWindow::get_file_contents(mat);
    //std::string oridecoded = base64_decode(loaded);
    //loadMat(oridecoded);

}

//void MainWindow::getMouseCoordinates(mouse_coordinates mouse)
//{
//    m_coordinates = new mouse_coordinates();
//}

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
    ui->engage_button->setEnabled(false);
}

std::string MainWindow::getActiveTerminalIPString()
{
    QString qs = ui->ips_combo->currentText();
    std::string utf8_text = qs.toUtf8().constData();
    return qs.toLocal8Bit().constData();
}

void MainWindow::on_engage_button_clicked()
{
   setMessageBodyAndSend(motion::Message::ActionType::Message_ActionType_ENGAGE);
}

void MainWindow::setMessageBodyAndSend(motion::Message::ActionType type)
{

    spinner_folders->start();

    QString qt_ip = ui->ips_combo->currentText();
    QByteArray ba_ip = qt_ip.toLatin1();
    std::string qt_ip_str = qt_ip.toUtf8().constData();
    char *c_str_ip = ba_ip.data();

    string data;
    motion::Message m;
    m.set_type(type);
    m.set_serverip(getIpAddress());
    m.set_time(getTime());
    m.SerializeToString(&data);
    char bts[data.length()];
    strcpy(bts, data.c_str());

    sendSocket(c_str_ip, bts);

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

    //xml = xmlfile;
    //QFile * xmlFile = new QFile(region_file);
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

    //if( ! xmlFile->open(QIODevice::WriteOnly) )
    //{
    //  QMessageBox::warning(NULL, "Test", "Unable to open: " + region_file , "OK");
    //}

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

void MainWindow::on_stream_clicked()
{

}

void MainWindow::on_picture_clicked()
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

    setMessageBodyAndSend(motion::Message::ActionType::Message_ActionType_TAKE_PICTURE);


}

void MainWindow::on_save_region_clicked()
{
    ui->qt_drawing_output->SaveRegion();
}

void MainWindow::on_clear_region_clicked()
{

}

void MainWindow::on_start_recognition_clicked()
{
    setMessageBodyAndSend(motion::Message::ActionType::Message_ActionType_REC_START);
}


void MainWindow::on_disconnect_clicked()
{
    setMessageBodyAndSend(motion::Message::ActionType::Message_ActionType_DISSCONNECT);
}

void MainWindow::on_get_time_clicked()
{
    setMessageBodyAndSend(motion::Message::ActionType::Message_ActionType_GET_TIME);
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
        mr.set_type(motion::Message::ActionType::Message_ActionType_REC_START);

        struct timeval tv;
        struct tm* ptm;
        char time_string[40];
        gettimeofday (&tv, NULL);
        ptm = localtime (&tv.tv_sec);
        strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S", ptm);
        mr.set_time(time_string);

        mr.set_serverip(getIpAddress());
        mr.set_regioncoords(region_resutl);

        mr.set_storeimage(true);
        mr.set_storecrop(false);

        //stream_sender = new StreamSender(this);
        //connect(stream_sender, SIGNAL(ResultEcho(string)), this, SLOT(ResultEcho(string)));
        //stream_sender->sendStream(getActiveTerminalIPString(), mr);
        //stream_sender->terminate();



        delete ptm;
        google::protobuf::ShutdownProtobufLibrary();

    }
    else
    {
        /*std::string command = getGlobalIntToString(STOP_RECOGNITION);
        tcpecho_thread = new TCPEchoThread(this);
        connect(tcpecho_thread, SIGNAL(ResultEcho(string)), this, SLOT(ResultEcho(string)));
        tcpecho_thread->SendEcho(getActiveTerminalIPString(), command);
        tcpecho_thread->terminate();*/
    }

}



void MainWindow::on_set_time_clicked()
{

    struct timeval tv;
    struct tm* ptm;
    char time_string[40];

    gettimeofday (&tv, NULL);
    ptm = localtime (&tv.tv_sec);
    strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S", ptm);

    cout << "time_string TIME :: " << time_string << endl;

    std::string command = getGlobalIntToString(452346254734573);

    char * message = new char[command.size() + 1];
    std::copy(command.begin(), command.end(), message);
    message[command.size()] = '\0'; // don't forget the terminating 0

    char buffer[256];
    strncpy(buffer, message, sizeof(buffer));
    strncat(buffer, time_string, sizeof(buffer));

    cout << "buffer:: " << buffer << endl;

    tcpsend_thread->SendEcho(getActiveTerminalIPString(), buffer);

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


void MainWindow::testBase()
{

    /*GOOGLE_PROTOBUF_VERIFY_VERSION;

    ::motion::Message testm;

    testm.set_type(motion::Message::ActionType::Message_ActionType_REC_START);
    testm.set_data("hola");
    testm.set_code("code");
    testm.set_payload("gkbfdgdfkhg kfsdhgkjhdfkjhgdfhgkjsfdhgkhdfsjkgnkfjdgnkhdfnsjghfkdsjhgnksdfhgknhdfkjghskdfhgkjsdhnkj");

    motion::Message::Instance * inst = testm.add_instance();

    inst->set_idinstance(0);
    inst->set_amount("amount");
    inst->set_filepath("hola");

    motion::Message::Instance * inst1 = testm.add_instance();

    inst1->set_idinstance(1);
    inst1->set_amount("fndgfdhgdf gsdfgfdgsdfgsfdgsdfgsdfg");
    inst1->set_filepath("fdgksdfkgdfgkdfjkgkjhfd");

    std::string file = "/jose/repos/motion/mm.pb";

    int size_init = testm.ByteSize();
    char data_init[size_init];

    try
    {
        testm.SerializeToArray(data_init, size_init);
    }
    catch (google::protobuf::FatalException fe)
    {
        std::cout << "PbToZmq " << fe.message() << std::endl;
    }

    std:string enc = base64_encode(reinterpret_cast<const unsigned char*>(data_init),sizeof(data_init));

    std::string basefile = file;
    std::ofstream out;
    out.open (basefile.c_str());
    out << enc << "\n";
    out.close();*/

    string loaded = MainWindow::get_file_contents("/jose/repos/motion/encoded_proto.txt");

    std::string oridecoded = base64_decode(loaded);

    motion::Message testmm;
    testmm.ParseFromArray(oridecoded.c_str(), oridecoded.size());

    cout << "data: " << testmm.data();

    /*for (int i = 0; i < testmm.instance_size(); i++)
    {
        const motion::Message::Instance & ins = testmm.instance(i);
        cout << "amount: " << ins.amount() << endl;
        cout << "id: " << ins.idinstance() << endl;
        cout << "filepatgh: " << ins.filepath() << endl;
    }*/

    google::protobuf::ShutdownProtobufLibrary();

    // The data we need to deserialize
    int width_d = 0;
    int height_d = 0;
    int type_d = 0;
    size_t size_d = 0;

    //std::string basefile = "/jose/repos/mat.txt";
    //string loaded = get_file_contents(basefile);

    std::string local_mat = base64_decode(testmm.data());

    stringstream decoded;
    //std::stringstream decoded;
    decoded << local_mat;

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

void MainWindow::loadMat(string encodedmat)
{

        std::string basefile = "/jose/repos/encoded_mat.txt";
        std::ofstream out;
        out.open (basefile.c_str());
        out << encodedmat << "\n";
        out.close();

       std::string oridecoded = base64_decode(encodedmat);

       stringstream decoded;
       //std::stringstream decoded;
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

      // Delete our buffer
      delete[]data_d;

      //std::string basefile = "/jose/repos/motion/region.txt";
      //string storedcoord = get_file_contents(basefile);
      //vector<Point2f> coordinates = stringToVectorPoint2f(storedcoord);
      //ui->qt_drawing_output->drawLinesSlot(coordinates);
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

void MainWindow::savedRegionResutl(QString re)
{
    region_resutl = re.toUtf8().constData();
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

void MainWindow::enableDisableButtons(bool set)
{

    ui->engage_button->setEnabled(set);
    ui->list_folders->setEnabled(set);
    ui->list_files->setEnabled(set);
    ui->start_recognition->setEnabled(set);
    ui->save_region->setEnabled(set);
    ui->rec_with_images->setEnabled(set);
    ui->stream->setEnabled(set);
    ui->picture->setEnabled(set);
    ui->clear_region->setEnabled(set);
    ui->set_time->setEnabled(set);
    ui->disconnect->setEnabled(set);
    ui->get_time->setEnabled(set);
    ui->amount_label->setEnabled(set);
    ui->remote_terminal_label->setEnabled(set);

    //qlabel
    ui->output->setEnabled(set);
    ui->qt_drawing_output->setEnabled(set);

    if (!set)
        ui->output->setStyleSheet("background-color: rgba( 200, 200, 200, 100% );");
    else
        ui->status_label->setStyleSheet("background-color: lightgreen;");
        ui->status_label->setText("Engaged");

    ui->output->setStyleSheet("border: 1px solid grey");
    ui->remote_capture->setStyleSheet("border: 1px solid grey");

}

void MainWindow::sendSocket(string svradress, string command)
{
    //TCP Socket
    tcpsend_thread = new TCPEchoThread(this);
    tcpsend_thread->SendEcho(svradress, command);
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

    // Works just like cout
    oss<< number;

    // Return the underlying string
    return oss.str();
}

void MainWindow::remoteProto(motion::Message m)
{
      int action = m.type();
      switch (action)
      {
        case motion::Message::ENGAGE:
        {
            enableDisableButtons(true);
            ui->engage_button->setDisabled(true);
            mount_thread->MountNetWorkDrive(ui->ips_combo->currentText());
            mount_thread->terminate();
            break;
        }
        case motion::Message::GET_TIME:
        {
          QString gt = QString::fromUtf8(m.time().c_str());
          ui->remote_terminal_time->setText(gt);
          break;
        }
        case motion::Message::TIME_SET:
        {
          QString ts = QString::fromUtf8(m.time().c_str());
          ui->remote_terminal_time->setText(ts);
          break;
        }
        case motion::Message::TAKE_PICTURE:
            loadMat(m.data());
          break;
    }
}


void MainWindow::resutlEcho(string str)
{

    string decoded_proto;

    std::string del_1  = "PROTO_START_DELIMETER";
    std::string del_2  = "PROTO_STOP_DELIMETER";

    std::string strdecoded;
    int total__packages;
    int current_package;

    int total_size = str.size();
    std::size_t found = str.find(del_1);

    if (found!=std::string::npos)
    {
       std::string lpay = MainWindow::ExtractString(str, del_1, del_2);

       vector<string> vpay = MainWindow::splitProto(lpay, '::');

       total__packages = atoi(vpay.at(0).c_str());
       current_package = atoi(vpay.at(2).c_str());

       int del_pos = str.find(del_2);
       int until = str.size() - 50;

       const string & ppayload = str.substr((del_pos+del_2.size()), until);

       payload_holder.push_back(ppayload);

       if (current_package==0)
       {
           ui->mat_progress->setMinimum(0);
           ui->mat_progress->setMaximum(total__packages);
       }
       ui->mat_progress->setValue(current_package);

       if (total__packages==(current_package+1))
       {
           finished = true;
       }
    }

    if (!finished)
    {

        motion::Message::ActionType reply;
        if (!finished)
        {
           reply =  motion::Message::ActionType::Message_ActionType_RESPONSE_NEXT;
        }
        else
        {
           reply = motion::Message::ActionType::Message_ActionType_RESPONSE_END;
        }
        setMessageBodyAndSend(reply);

    }
    else
    {

        std::string payload;
        for (int j=0; j<payload_holder.size(); j++)
        {
            payload += payload_holder.at(j);
        }
        payload_holder.clear();

        //std::string basefile = "/jose/repos/motion/encoded_remote_proto_" + IntToString(current_package) + ".txt";
        std::string basefile = "/jose/repos/motion/encoded_remote_proto.txt";
        std::ofstream out;
        out.open (basefile.c_str());
        out << payload << "\n";
        out.close();

        std::string strdecoded = base64_decode(payload);

        GOOGLE_PROTOBUF_VERIFY_VERSION;
        motion::Message mm;

        mm.ParseFromArray(strdecoded.c_str(), strdecoded.size());

        remoteProto(mm);

        finished=false;

        google::protobuf::ShutdownProtobufLibrary();

    }

}




