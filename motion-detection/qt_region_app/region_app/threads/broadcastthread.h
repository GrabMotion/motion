#ifndef BROADCASTTHREAD_H
#define BROADCASTTHREAD_H

#include <QObject>
#include <QWidget>
#include <QtCore>
#include <QThread>
#include "socket/PracticalSocket.h"
#include <QStringList>

#include <iostream>            // For cout and cerr
#include <cstdlib>             // For atoi()
#include <sstream>

using namespace std;

class BroadcastThread : public QThread
{
       Q_OBJECT
public:
    explicit BroadcastThread(QObject *parent =0);
    void run();
    bool Stop();

signals:
    void BroadcastReceived(QString);
    void BroadcastTimeoutSocketException();

};

#endif // BROADCASTTHREAD_H
