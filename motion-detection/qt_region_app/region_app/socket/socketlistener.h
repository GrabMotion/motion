#ifndef SOCKETLISTENER_H
#define SOCKETLISTENER_H

#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QtCore>
#include <QThread>

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

class SocketListener : public QObject
{
    Q_OBJECT
public:
    explicit SocketListener(QObject *parent = 0);
    std::string from_ip;
    void startListening(QObject *parent);
    void * HandleTCPClient (TCPSocket *sock, QObject *parent);
    static  void * threadMain   (void *arg); //(void *clntSock);
    static void * socketThread (void * args);
    static void * watch_echo   (void * args);

private:
    pthread_t thread_wait_echo;
    pthread_mutex_t echo_mutex;
    pthread_cond_t echo_response;
    bool echo_received;
    int resutl_echo;
    void emitSignal(std::string message);

public:
    std::string socket_response;

signals:
    void SocketReceivedSignal(std::string);

};

#endif // SOCKETLISTENER_H
