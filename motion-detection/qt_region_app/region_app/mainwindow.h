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

//#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <fstream>
#include <string>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include "b64/base64.h"

#include "drawing/mouse_coordinates.h"

#include <sstream>
#include <vector>
#include <iostream>
#include <string>

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
    TCPEchoThread       *tcpsend_thread;
    MountThread         *mount_thread;
    SocketListener      *socket_listener;
    StreamListener      *stream_listener;
    StreamSender        *stream_sender;
    UDPServer           *udp_server;
    MatListener         *mat_listener;

    std::string NETWORK_IP;
    QString share;
    QString getShare();

    std::string getIpAddress();
    std::string getTime();
    void remoteProto(motion::Message payload);
    void resutlEcho(string resutl);

private:
    Ui::MainWindow *ui;
    QtWaitingSpinner *m_spinner;
    QtWaitingSpinner *spinner_folders;
    QImage last_stored_frame;
    std::string getActiveTerminalIPString();

    QFileSystemModel *dirModel;
    QFileSystemModel *fileModel;

    void closeEvent (QCloseEvent *event);

    void split(const string& s, char c, vector<string>& v);
    std::string local_ip;

    void RefreshTreViewModel(QString roo, QString rip);
    void refresh_results();
    QString treeViewPath;
    QString ipPath;
    QString getSharedFolder();
    void getLocalNetwork();

    void setRemoteProto(motion::Message payload);

    std::string result_message;
    QString q_response;

    void SocketErrorMessage(QString &e);
    void sendSocket(string svradress, string command);

    void enableDisableButtons(bool set);

    std::string getTimeStr();
    char * getTimeChat();
    char * getTerMinalIpFromCombo();

    void remoteMat(cv::Mat mat);

    void testBase();
    std::string get_file_contents(std::string filename);
    int pcount = 0;
    int msg_split_vector_size;
    int realsize;
    int packegesize;
    google::protobuf::uint32 packagesize;
    vector<string> payload_holder;
    string payload;
    bool complete;
    google::protobuf::uint32 packagetype;
    google::protobuf::uint32 mode;
    std::vector<std::string> splitProto(const std::string &s, char delim);
    vector<string> splitString(string input, string delimiter);
    std::string ExtractString( std::string source, std::string start, std::string end );
    std::string strdecoded;
    bool finished=false;
    int payload_size;

    void loadMat(string mat);

    vector<Point2f> stringToVectorPoint2f(std::string storedcoord);

    std::string region_resutl;

    void setMessageBodyAndSend(motion::Message::ActionType type);

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
    void on_engage_button_clicked();
    void on_search_button_clicked();
    void on_start_recognition_clicked();
    void on_start_recognition_toggled(bool checked);
    void on_disconnect_clicked();
    void on_save_region_clicked();
    void on_get_time_clicked();
    void on_set_time_clicked();
    void on_picture_clicked();
    void on_stream_clicked();
    void on_clear_region_clicked();

    //mouse
    void Mouse_current_pos();
    void showMousePosition(QPoint&);

    //sockets
    void BroadcastReceived(QString);
    void broadcastTimeoutSocketException();
    //void StreamingUpdateLabelImage(std::string, Mat);

    //shares
    void SharedMounted(QString folder);
    void ShareUmounted();

    //Files
    void on_list_files_clicked(const QModelIndex &index);
    void on_list_folders_clicked(const QModelIndex &index);

    //Region
    void savedRegionResutl(QString re);



signals:
    void drawLinesSignal(std::vector<cv::Point2f> lines);


};

#endif // MAINWINDOW_H
