#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "threads/broadcastthread.h"
#include "threads/streamingthread.h"
#include "threads/mountthread.h"
#include "threads/tcpechothread.h"
#include "socket/socketlistener.h"
#include "socket/streamlistener.h"
#include "socket/streamsender.h"
#include "socket/udpserver.h"
#include "socket/matlistener.h"

#include "protobuffer/motion.pb.h"

#include <QFileSystemModel>
#include <sys/time.h>

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>


#include <iostream>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "b64/base64.h"


const unsigned int CONNECT                  = 1000;

const unsigned int STOP_STREAMING           = 1002;
const unsigned int PAUSE_STREAMING          = 1003;

const unsigned int START_RECOGNITION        = 1004;
const unsigned int STOP_RECOGNITION         = 1005;

const unsigned int DISSCONNECT              = 1006;

const unsigned int GET_TIME                 = 1007;
const unsigned int SET_TIME                 = 1008;
const unsigned int TIME_SET                 = 1009;

const unsigned int AMOUNT_DETECTED          = 1010;
const unsigned int FILE_RECOGNIZED          = 1011;

const unsigned int TCP_ECHO_PORT            = 5010;
const unsigned int UDP_PORT                 = 5020;
const unsigned int STREAMING_VIDEO_PORT     = 5030;
const unsigned int TCP_MSG_PORT             = 5040;

class QtWaitingSpinner;

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    //Threads
    BroadcastThread     *broadcast_thread;
    StreamingThread     *streaming_thread;
    TCPEchoThread       *tcpecho_thread;
    MountThread         *mount_thread;
    SocketListener      *socket_listener;
    StreamListener      *stream_listener;
    StreamSender        *stream_sender;
    UDPServer           *udp_server;
    MatListener         *mat_listener;

    std::string NETWORK_IP;
    QString share;
    QString getShare();

    QString xmlstring;

private:
    Ui::MainWindow *ui;
    QtWaitingSpinner *m_spinner;
    QtWaitingSpinner *spinner_folders;
    QImage last_stored_frame;
    std::string getActiveTerminalIPString();

    QFileSystemModel *dirModel;
    QFileSystemModel *fileModel;

    void closeEvent (QCloseEvent *event);

    std::string getIpAddress ();
    void split(const string& s, char c, vector<string>& v);
    std::string local_ip;

    void RefreshTreViewModel(QString roo, QString rip);
    void refresh_results();
    QString treeViewPath;
    QString ipPath;
    QString getSharedFolder();
    void getLocalNetwork();
    void setRemoteMessage(QString str);
    void setRemoteProto(motion::Message payload);

    std::string result_message;
    QString q_response;

    void SocketErrorMessage(QString &e);

    void testBase();
    std::string getTime();

    std::string getTimeStr();
    char * getTimeChat();
    char * getTerMinalIpFromCombo();

    void remoteProto(motion::Message payload);
    void remoteMat(cv::Mat mat);

    void savePointsAsXML(vector<Point2f> &contour );


public:
    Q_SLOT void setremoteProto(motion::Message payload)
    {
        remoteProto(payload);
    }
    Q_SLOT void setremoteMat(cv::Mat mat)
    {
        remoteMat(mat);
    }

private slots:

    //buttons
    void on_search_button_clicked();
    void on_connect_button_clicked();
    void on_start_recognition_clicked();
    void on_start_recognition_toggled(bool checked);

    //mouse
    void Mouse_current_pos();
    void Mouse_pressed(std::vector<cv::Point2f>&);
    void Mouse_left();
    void showMousePosition(QPoint&);
    void Mouse_Pressed_Right_Click(std::vector<cv::Point2f>&);

    //sockets
    void BroadcastReceived(QString);
    void broadcastTimeoutSocketException();
    //void StreamingUpdateLabelImage(std::string, Mat);
    void ResultEcho(string);

    //shares
    void SharedMounted(QString folder);
    void ShareUmounted();

    void on_list_files_clicked(const QModelIndex &index);
    void on_list_folders_clicked(const QModelIndex &index);

    void on_disconnect_clicked();

    void on_screenshot_clicked();
    void on_save_region_clicked();
    void on_get_time_clicked();

    void on_set_time_clicked();


signals:
    void SocketReceivedSignal(std::string);

};

#endif // MAINWINDOW_H
