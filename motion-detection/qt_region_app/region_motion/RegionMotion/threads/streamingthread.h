#ifndef STREAMINGTHREAD_H
#define STREAMINGTHREAD_H

#include <QObject>
#include <QWidget>
#include <QThread>
#include <QStringList>

/*#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"*/

#include <QCoreApplication>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.h>
#include <iostream>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <errno.h>
#include <sstream>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "image/mat2qimage.h"

using namespace cv;

class StreamingThread : public QThread
{
    Q_OBJECT
public:
    explicit StreamingThread(QObject *parent =0);
    //void StartStreaming(char * serverIp,  int port, QString share);
    void StartStreaming(char *c_str_ip, QString ip, QString share);
    void StopStreaming();

 private:
    int soket_streaming;
    //QMutex streamingMutex;
    QImage frame;
    pthread_mutex_t streamingMutex;
    bool run = true;

    int width = 640; //1280;
    int height = 480; //720;
    int jpegQuality = 95;

 signals:
    void StreamingUpdateLabelImage(std::string, Mat img);

};

#endif // STREAMINGTHREAD_H
