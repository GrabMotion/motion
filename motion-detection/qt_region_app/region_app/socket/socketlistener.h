#ifndef SOCKETLISTENER_H
#define SOCKETLISTENER_H

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QtCore>
#include <QThread>
#include <QtWidgets/qdialog.h>
#include <QMessageBox>

#include "socket/PracticalSocket.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <cstdio>

#include <string>
#include <sstream>
#include <iostream>
#include <sys/malloc.h>

#include "protobuffer/motion.pb.h"
#include <opencv2/opencv.hpp>
#include "image/mat2qimage.h"

#include <string>
#include <cassert>
#include <limits>
#include <stdexcept>
#include <cctype>

#include "b64/base64.h"
//#include "b64/encode.h"
//#include "b64/decode.h"

#include <google/protobuf/message.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <fstream>

class SocketListener : public QObject
{
    Q_OBJECT
public:
    explicit SocketListener(QObject *parent = 0);
    std::string from_ip;
    void startListening(QObject *parent);

    void * HandleTCPClient (TCPSocket *sock, QObject *parent);
    static void * threadMain   (void * arg);
    static void * socketThread (void * args);
    static void * watch_echo   (void * args);

private:
    pthread_t thread_wait_echo;
    pthread_mutex_t echo_mutex;
    pthread_cond_t echo_response;
    bool echo_received;
    int resutl_echo;

public:
    std::string socket_response;


};

#endif // SOCKETLISTENER_H
