#ifndef TCPMESSAGETHREAD_H
#define TCPMESSAGETHREAD_H

#include <QObject>
#include <QWidget>
#include <QThread>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include <errno.h>
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "socket/PracticalSocket.h"

using namespace std;

class TcpMessageThread : public QThread
{
    Q_OBJECT
public:
    explicit TcpMessageThread(QObject *parent =0);
    void ReceiveMessage(char *c_str_ip, QString ip);

private:
    const unsigned int RCVBUFSIZE = 32;    // Size of receive buffer
    const int MAXRCVSTRING = 4096; // Longest string to receive
    int soket_message;

    const unsigned int TCP_MSG_PORT                 = 5040;

signals:
   void ResultMessage(string);

};

#endif // TCPMESSAGETHREAD_H
