#include "../practical/sendmessage.h"

#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>
#include <vector>

#include <unistd.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

using namespace google::protobuf::io;

const int MAXDATASIZE  =  100500;
const int PAYLOADSIZE  =  100000;

pthread_t thread_echo_socket;

struct message_thread_args
{
    motion::Message message;
    bool array;
};
struct message_thread_args SocketStruct;


void * protoSend (void * arg)
{
   
    struct message_thread_args *args = (struct message_thread_args *) arg;
    
    bool array = args->array;
    
    motion::Message m = args->message;
    send_proto = m;
    
    string servAddress = m.serverip();
    
    cout << "::servAddress:: " << servAddress <<  endl;
    
    int port = motion::Message::TCP_ECHO_PORT;
    unsigned short echoServPort = port;
    
    cout << "::echoServPort:: " << echoServPort <<  endl;
    
    char echoBuffer[MAXDATASIZE + 1];
    
    try
    {
        
        std::cout << "Establish connection with the echo server :: " << servAddress << " port: " << echoServPort << std::endl;
        
        // Establish connection with the echo server
        TCPSocket sock(servAddress, echoServPort);

        int size = m.ByteSize();
        
        //Initialize objects to serialize.
        char data[size];
        string datastr;
        
        if (array)
        {
            cout << "ByteArraySize:: " << size <<  endl;
            try
            {
                m.SerializeToArray(&data, size);
                
            }
            catch (google::protobuf::FatalException fe)
            {
                std::cout << "PbToZmq " << fe.message() << std::endl;
            }
            
        } else
        {
            cout << "ByteStringSize:: " << size <<  endl;
            try
            {
                m.SerializeToString(&datastr);
            }
            catch (google::protobuf::FatalException fe)
            {
                std::cout << "PbToZmq " << fe.message() << std::endl;
            }
            
        }
        
        //std::vector<char*> protov(size+4);
        //protov.push_back(data);
        //protov.push_back("hola");
        
        
        
        if (array)
        {
            
            // send
            //char buff[10000];
            size_t len = strlen(data);
            char *p = data;
            ssize_t n;
            while ( len > 0 && (n=sock.send(&p,len,0)) > 0 ) {
                p += n;
                len =- (size_t)n;
            }
            if ( len > 0 || n < 0 ) {
                // oops, something went wrong
            }
            
            //sock.send(&protov, protov.size());
            //sock.send(data, sizeof(data));
        } else
        {
            char bts[datastr.length()];
            strcpy(bts, datastr.c_str());
            sock.send(bts, sizeof(bts));
        }
        
        google::protobuf::ShutdownProtobufLibrary();
       
    } catch(SocketException &e)
    {
        cout << "::error:: " << e.what() << endl;
        cerr << e.what() << endl;
        exit(1);
    }
    
    pthread_cancel(thread_echo_socket);
    
    
}

void protoSocket(motion::Message m, bool array)
{
    
    cout << "+++++++++++SENDING PROTO++++++++++++++" << endl;

    SocketStruct.message         = m;
    SocketStruct.array           = array;

    // run the streaming client as a separate thread
    runm = pthread_create(&thread_echo_socket, NULL, &protoSend, &SocketStruct);
    
    if ( runm  != 0) {
        cerr << "Unable to create streamVideo thread" << endl;
        cout << "BroadcastSender pthread_create failed." << endl;
    }
    
    //pthread_join(    thread_echo_socket,          (void**) &runm);
    
    //pthread_cancel(thread_echo_socket);
    
}