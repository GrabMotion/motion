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

struct message_thread_args
{
    motion::Message message;
    bool array;
};
struct message_thread_args MessageStructThread;

const unsigned int RCVBUFSIZE = 200000;     // Size of receive buffer
//const int MAXRCVSTRING = 4096;          // Longest string to receive


void * sendMessage (void * arg)
{

    struct message_thread_args *args = (struct message_thread_args *) arg;
    
    bool array = args->array;
    
    motion::Message m = args->message;
    send_proto = m;
    
    string servAddress = m.serverip();
    
    cout << "::servAddress:: " << servAddress <<  endl;
    
    int port = motion::Message::TCP_MSG_PORT;
    unsigned short echoServPort = port;
    
    cout << "::echoServPort:: " << echoServPort <<  endl;
    
    char echoBuffer[RCVBUFSIZE + 1];
    
    try
    {
        
        std::cout << "Establish connection with the echo server :: " << servAddress << " " << echoServPort << std::endl;
        
        // Establish connection with the echo server
        TCPSocket sock(servAddress, echoServPort);

        
        int size = m.ByteSize() * 2;
        
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
                //m.SerializeToString(data);
                //char bts[datastr.length()];
                //strcpy(bts, datastr.c_str());
                //sock.send(bts, sizeof(bts));
                
            }
            catch (google::protobuf::FatalException fe)
            {
                std::cout << "PbToZmq " << fe.message() << std::endl;
            }
            
        }
        
        /*int siz = 110000; //m.ByteSize()+4;
        char *pkt = new char [siz];
        google::protobuf::io::ArrayOutputStream aos(pkt,siz);
        CodedOutputStream *coded_output = new CodedOutputStream(&aos);
        coded_output->WriteVarint32(m.ByteSize());
        m.SerializeToCodedStream(coded_output);*/
    
        //sock.send(pkt, siz);
        
        sock.send(data, sizeof(data));
        
        cout << "::1:: " << endl;
        
        // Buffer for echo string + \0
        int bytesReceived = 0;              // Bytes read on each recv()
        int totalBytesReceived = 0;         // Total bytes read
        
        cout << "::2:: " << endl;
        
        GOOGLE_PROTOBUF_VERIFY_VERSION;
        
        motion::Message mm;

        while (totalBytesReceived < sizeof(data)) {
            // Receive up to the buffer size bytes from the sender
            if ((bytesReceived = (sock.recv(echoBuffer, RCVBUFSIZE))) <= 0) {
                cerr << "Unable to read";
                exit(1);
            }
            totalBytesReceived += bytesReceived;                // Keep tally of total bytes
            echoBuffer[bytesReceived] = '\0';                   // Terminate the string!
            cout << "Received message: " << echoBuffer << endl; // Print the echo buffer
            
            cout << "+++++++++++REPLY PROTO++++++++++++++" << endl;
            
            const string & data = echoBuffer;
            mm.ParseFromString(data);
            receive_proto.Clear();
            receive_proto = mm;
            int size_final = mm.ByteSize();
            
            int type = mm.type();
            cout << "+++++++++++" << "RESPONSE TYPE: " << size_final << " ++++++" << endl;

        }
        cout << endl;
        google::protobuf::ShutdownProtobufLibrary();
        delete data;
    
    } catch(SocketException &e)
    {
        cout << "::error:: " << e.what() << endl;
        cerr << e.what() << endl;
        exit(1);
    }
    
    pthread_cancel(thread_message);
    
    
}

void setMessage(motion::Message m, bool array)
{
    
    cout << "+++++++++++SENDING PROTO++++++++++++++" << endl;

    MessageStructThread.message         = m;
    MessageStructThread.array           = array;
    
    // run the streaming client as a separate thread
    runm = pthread_create(&thread_message, NULL, sendMessage, &MessageStructThread);
    if ( runm  != 0) {
        cerr << "Unable to create streamVideo thread" << endl;
        cout << "BroadcastSender pthread_create failed." << endl;
    }
    
    //pthread_join(    thread_message,          (void**) &runm);
    
    //pthread_cancel(thread_message);
    
}