#ifndef TCPMESSAGETHREAD_H
#define TCPMESSAGETHREAD_H

#include <QObject>
#include <QWidget>
#include <QThread>

using namespace std;

class TcpMessageThread : public QThread
{
    Q_OBJECT
public:
    explicit TcpMessageThread(QObject *parent =0);
    //void ReceiveMessage(char *c_str_ip, string command);

signals:
   void ResultMessage(string);

};

#endif // TCPMESSAGETHREAD_H
