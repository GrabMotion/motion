#ifndef TCPECHOTHREAD_H
#define TCPECHOTHREAD_H

#include <QObject>
#include <QWidget>
#include <QtCore>
#include <QThread>
#include <QMessageBox>

#include "socket/PracticalSocket.h"  // For Socket and SocketException
#include <iostream>           // For cerr and cout
#include <cstdlib>            // For atoi()
#include <sstream>
#include <string>
#include <stdio.h>

#include "protobuffer/motion.pb.h"
#include "b64/base64.h"

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

using namespace std;

class TCPEchoThread : public QThread
{
    Q_OBJECT
public:
   explicit TCPEchoThread(QObject *parent =0);
    void SendEcho (int packagesize, QObject *parent, string svradress, char * message);
    void SendEcho (int packagesize, QObject *parent, string svradress, string command);
    void send(int packagesize, QObject *parent, string svradress, char * message);

private:
    const unsigned int TCP_ECHO_PORT    = 5010;
    std::vector<std::string> splitProto(const std::string &s, char delim);


signals:
   void ResultEcho(string);

};

#endif // TCPECHOTHREAD_H
