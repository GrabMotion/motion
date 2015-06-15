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
#include <pthread.h>
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

//#include <google/protobuf/message.h>
#include "protobuffer/motion.pb.h"
#include <opencv2/opencv.hpp>
#include "image/mat2qimage.h"

class SocketListener : public QObject
{
    Q_OBJECT
public:
    explicit SocketListener(QObject *parent = 0);
    std::string from_ip;
    void startListening(QObject *parent);

    void * HandleTCPClient (TCPSocket *sock, QObject *parent);
    static  void * threadMain   (void *arg);
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

//signals:
    //void SocketReceivedSignal(std::string);

//signals:
  // void ResultEchoMessage(string);

};

#endif // SOCKETLISTENER_H
