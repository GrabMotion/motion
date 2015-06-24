#ifndef STREAMLISTENER_H
#define STREAMLISTENER_H

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
//#include <sys/malloc.h>

#include <string>
#include <cassert>
#include <limits>
#include <stdexcept>
#include <cctype>

#include <errno.h>


#include "opencv2/opencv.hpp"

#include "../b64/base64.h"
#include "../protobuffer/motion.pb.h"

#include "google/protobuf/message.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl.h"


#include <fstream>

class StreamListener
{
 
public:
    explicit StreamListener();

    void startListening();
    static google::protobuf::uint32 readHdr(char *buf);
    static void readBody(int csock,google::protobuf::uint32 siz);
    static void * socketHandler (void* lp);
    static void * socketThread  (void * args);

private:
    
public:
    std::string socket_response;


};

#endif // STREAMLISTENER_H
