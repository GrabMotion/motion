#ifndef STREAMINGTHREAD_H
#define STREAMINGTHREAD_H

#include <QObject>
#include <QWidget>
#include <QThread>
#include <QStringList>

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <errno.h>

#include "image/mat2qimage.h"

using namespace cv;

class StreamingThread : public QThread
{
    Q_OBJECT
public:
    explicit StreamingThread(QObject *parent =0);
    void StartStreaming(char * serverIp,  int port);
    void StopStreaming();

 private:
    int soket_streaming;
    //QMutex streamingMutex;
    QImage frame;
    pthread_mutex_t streamingMutex;
    bool run = true;

 signals:
    void StreamingUpdateLabelImage(QImage, Mat);

};

#endif // STREAMINGTHREAD_H
