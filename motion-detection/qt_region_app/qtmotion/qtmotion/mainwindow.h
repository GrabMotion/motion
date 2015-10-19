#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>

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
    //StreamingThread     *streaming_thread;
    TCPEchoThread       *tcpsend_thread;
    MountThread         *mount_thread;
    //SocketListener      *socket_listener;
    //StreamListener      *stream_listener;
    //StreamSender        *stream_sender;
    //UDPServer           *udp_server;
    //MatListener         *mat_listener;

    std::string NETWORK_IP;
    QString share;
    QString getShare();

    std::string getIpAddress();
    std::string getTime();
    void remoteProto(motion::Message payload);
    void resutlEcho(string resutl);
    void receivedEcho(motion::Message m);

    std::string ExtractString( std::string source, std::string start, std::string end );
    std::vector<std::string> splitProto(const std::string &s, char delim);

private:
    Ui::MainWindow *ui;
    QtWaitingSpinner *m_spinner;
    QtWaitingSpinner *i_spinner;
    //QtWaitingSpinner *spinner_folders;
    QImage last_stored_frame;
    std::string getActiveTerminalIPString();

    QFileSystemModel *dirModel;
    QFileSystemModel *fileModel;

    std::string ip_address;

    void closeEvent (QCloseEvent *event);

    void split(const string& s, char c, vector<string>& v);
    std::string local_ip;

    //void RefreshTreViewModel(QString roo, QString rip);
    //void refresh_results();
    QString treeViewPath;
    QString ipPath;
    QString getSharedFolder();
    void getLocalNetwork();
    void getTerminalFolder(motion::Message m);
    void saveProto(string encodedproto, std::string file);

    void setRemoteProto(motion::Message payload);

    std::string result_message;
    QString q_response;

    void SocketErrorMessage(QString &e);
    void sendSocket(int packagesize, string svradress, string command);

    void enableDisableButtons(int set);
    void enable_diable_Uppper_Bar(bool set);
    void enable_diable_Lateral_Time(bool set);
    void enable_diable_Lateral_Grid(bool set);
    void enable_diable_Lower_Bar(bool set);
    void enable_diable_Process(bool set);
    void enable_disable_Save(bool set);

    void on_engage(QString rec);

    std::string getTimeStr();
    char * getTimeChat();
    char * getTerMinalIpFromCombo();

    void remoteMat(cv::Mat mat);

    void testBase();
    std::string get_file_contents(std::string filename);
    int pcount = 0;
    vector<string> payload_holder;
    vector<string> splitString(string input, string delimiter);

    bool finished=false;

    void saveMat(std::string encodedmat, google::protobuf::uint32 file);
    void loadMat(google::protobuf::uint32 file);
    cv::Mat extractMat(string loadedmat);
    void saveImage(std::string oridecoded, std::string file);

    vector<Point2f> stringToVectorPoint2f(std::string storedcoord);

    std::string region_resutl;
    bool region;

    void setMessageBodyAndSend(motion::Message m);

    void saveLocalProto(QString qproto, motion::Message remote);
    void loadInstances(motion::Message m);
    motion::Message PROTO;

    std::string IntToString ( int number );

    std::string getCurrentDayLabel();
    std::string getCurrentMonthLabel();

    string reply_next_proto;
    int packagesize;
    char *c_str_ip;

    void updateTime(motion::Message m);

    std::string video_name;
    vector<string> folder_paths;

    std::string imagepath;

    void engage_refresh(bool engage);
    void populateRecCombo(const motion::Message::MotionCamera * mmactivecam);
    void enableStartButton(const motion::Message::MotionCamera * mcam);

    void loadRecData(const motion::Message::MotionCamera * mcam);

public:
    Q_SLOT void setremoteProto(motion::Message payload)
    {
        remoteProto(payload);
    }
    Q_SLOT void setremoteMat(cv::Mat mat)
    {
        remoteMat(mat);
    }
    Q_SLOT void listenerProto(motion::Message payload)
    {
        receivedEcho(payload);
    }
    Q_SLOT void socketMessage(std::string msg)
    {
        resutlEcho(msg);
    }

private slots:

    //buttons
    void on_engage_button_clicked();
    void on_search_button_clicked();
    //void on_disconnect_clicked();
    void on_save_region_clicked();
    void on_get_time_clicked();
    void on_set_time_clicked();
    void on_picture_clicked();
    void on_stream_clicked();
    void on_clear_region_clicked();
    void on_start_recognition_toggled(bool checked);
    void on_refresh_clicked();
    void on_getxml_clicked();

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
    //void on_list_files_clicked(const QModelIndex &index);
    //void on_list_folders_clicked(const QModelIndex &index);

    //Region
    void savedRegionResutl(QString re);

    //Instances.
    void on_remote_directory_itemClicked(QTreeWidgetItem *item, int column);
    void dayComboChange(const QString &arg);
    void on_watch_video_clicked();
    void on_watch_image_clicked();
    void on_collapse_clicked();
    void on_expand_clicked();
    void onRecNameTextChanged();
    void on_save_rec_clicked();
    void on_rec_new_clicked();
    void on_rec_activated(const QString &arg1);
    void on_between_clicked(bool checked);

    void on_cameracombo_activated(const QString &arg1);

signals:
    void drawLinesSignal(std::vector<cv::Point2f> lines);


};



#endif // MAINWINDOW_H
