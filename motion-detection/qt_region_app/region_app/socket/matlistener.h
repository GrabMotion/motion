#ifndef MatListener_H
#define MatListener_H

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QtCore>
#include <QThread>
#include <QtWidgets/qdialog.h>
#include <QMessageBox>

#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include <pthread.h>
#include <unistd.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "image/mat2qimage.h"
#include "b64/base64.h"

using namespace cv;

class MatListener : public QObject
{
    Q_OBJECT
public:
    explicit MatListener(QObject *parent = 0);

    void startListening (QObject *parent);
    static void * streamThread(void * args);
    static void * streamServer(void* arg);
    static void  quit(std::string msg, int retval);

private:


};

#endif // MatListener_H
