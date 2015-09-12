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

#include "protobuffer/motion.pb.h"
#include <opencv2/opencv.hpp>
#include "b64/base64.h"


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
    static int resutl_echo;

    int msg_split_vector_size;
    int pcount;
    int realsize;
    int packegesize;
    google::protobuf::uint32 packagesize;
    vector<string> payload_holder;
    string payload;
    bool complete;
    google::protobuf::uint32 type;
    google::protobuf::uint32 mode;
    std::vector<std::string> split(const std::string &s, char delim);
    vector<string> splitString(string input, string delimiter);
    std::string ExtractString( std::string source, std::string start, std::string end );

public:
    std::string socket_response;

};

#endif // SOCKETLISTENER_H
