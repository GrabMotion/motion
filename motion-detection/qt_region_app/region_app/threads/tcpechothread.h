#ifndef TCPECHOTHREAD_H
#define TCPECHOTHREAD_H

#include <QObject>
#include <QWidget>
#include <QtCore>
#include <QThread>

#include "socket/PracticalSocket.h"  // For Socket and SocketException
#include <iostream>           // For cerr and cout
#include <cstdlib>            // For atoi()
#include <sstream>
#include <string>

using namespace std;

class TCPEchoThread : public QThread
{
    Q_OBJECT
public:
   explicit TCPEchoThread(QObject *parent =0);
   void SendEcho(string svradress, string command);

private:
    const int RCVBUFSIZE = 32;

signals:
   void ResultEcho(string);

};

#endif // TCPECHOTHREAD_H
