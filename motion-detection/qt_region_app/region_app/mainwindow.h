#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "threads/broadcastthread.h"
#include "threads/streamingthread.h"
#include "threads/mountthread.h"
#include "threads/tcpechothread.h"

#include <QFileSystemModel>

const unsigned int CONNECT                  = 1000;

const unsigned int STOP_STREAMING           = 1002;
const unsigned int PAUSE_STREAMING          = 1003;

const unsigned int START_RECOGNITION        = 1004;
const unsigned int STOP_RECOGNITION         = 1005;

const unsigned int TCP_PORT                 = 5010;
const unsigned int STREAMING_VIDEO_PORT     = 5030;



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
    BroadcastThread *broadcast_thread;
    StreamingThread *streaming_thread;
    TCPEchoThread *tcpecho_thread;
    MountThread *mount_thread;
    std::string NETWORK_IP;

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

private slots:

    //buttons
    void on_search_button_clicked();
    void on_capture_video_clicked();
    void on_connect_button_clicked();
    void on_start_recognition_clicked();
    void on_start_recognition_toggled(bool checked);

    //mouse
    void Mouse_current_pos();
    void Mouse_pressed();
    void Mouse_left();
    void showMousePosition(QPoint& pos);
    void Mouse_Pressed_Right_Click(std::vector<cv::Point2f>&);

    //sockets
    void BroadcastReceived(QString);
    void broadcastTimeoutSocketException();
    void StreamingUpdateLabelImage(QImage, Mat);
    void ResultEcho(string);

    //shares
    void SharedMounted(QString folder);
    void ShareUmounted();

    void on_list_files_clicked(const QModelIndex &index);
    void on_list_folders_clicked(const QModelIndex &index);

};

#endif // MAINWINDOW_H
