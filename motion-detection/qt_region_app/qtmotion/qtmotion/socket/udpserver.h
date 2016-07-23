#ifndef UDPSERVER_H
#define UDPSERVER_H

#include <QObject>
#include <QWidget>
#include <QtCore>
#include <QThread>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>

#include <pthread.h>
#include "pthread/pthread.h"
#include <fstream>
#include <stdio.h>
#include <cstdio>

#include <string>
#include <sstream>
#include <iostream>

#include "protobuffer/motion.pb.h"


class UDPServer : public QObject
{
    Q_OBJECT
public:
    explicit UDPServer(QObject *parent = 0);
    ~UDPServer();

    static void * listen (void * args);
    void startListening (QObject *parent);

signals:

public slots:
};

#endif // UDPSERVER_H
