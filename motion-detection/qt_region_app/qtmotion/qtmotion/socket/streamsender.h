#ifndef STREAMSENDER_H
#define STREAMSENDER_H

#include <QObject>
#include <QWidget>
#include <QtCore>
#include <QThread>

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

#include <unistd.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

#include "protobuffer/motion.pb.h"
#include <opencv2/opencv.hpp>
#include "b64/base64.h"


class StreamSender : public QThread
{
    Q_OBJECT
public:
    explicit StreamSender(QObject *parent = 0);
    ~StreamSender();

    void sendStream (std::string svradress, motion::Message payload);

signals:

public slots:
};

#endif // STREAMSENDER_H
