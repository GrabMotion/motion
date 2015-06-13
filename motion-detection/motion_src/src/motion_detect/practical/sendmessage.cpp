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

struct message_thread_args
{
    motion::Message message;
    bool array;
};
struct message_thread_args MessageStructThread;

const unsigned int RCVBUFSIZE = 32;     // Size of receive buffer
const int MAXRCVSTRING = 4096;          // Longest string to receive


void * sendMessage (void * arg)
{
    
    struct message_thread_args *args = (struct message_thread_args *) arg;
    
    bool array = args->array;
    
    motion::Message m = args->message;
    send_proto = m;
    
    cout << "::m:: " << m.type() <<  endl;
    
    string servAddress = m.serverip();
    
    cout << "::servAddress:: " << servAddress <<  endl;
    
    int port = motion::Message::TCP_MSG_PORT;
    unsigned short echoServPort = port;
    
    cout << "::echoServPort:: " << echoServPort <<  endl;
    
    char echoBuffer[RCVBUFSIZE + 1];
    
    cout << "::1:: " <<  endl;
    
    try
    {
        
        std::cout << "Establish connection with the echo server :: " << servAddress << " " << echoServPort << std::endl;
        
        // Establish connection with the echo server
        TCPSocket sock(servAddress, echoServPort);
        
        cout << "::2:: " <<  endl;
        
    
        //string data;
        int size = m.ByteSize();
        char data[size];
        
        if (array)
        {
            cout << "ByteSize:: " << size <<  endl;
            try
            {
                m.SerializeToArray(data, size);
                //m.SerializeToArray(&data, size);
            }
            catch (google::protobuf::FatalException fe)
            {
                std::cout << "PbToZmq " << fe.message() << std::endl;
            }
            
        } else
        {
           //m.SerializeToString(data);
        }
        //cout << "::3:: data:: " << data <<  endl;
        
        //char bts[data.length()];
        
        cout << "::4:: " <<  endl;
        //strcpy(bts, data.c_str());
    
        //m.SerializeToArray(&data, m.ByteSize());
        //char bts[data.length()];
        //strcpy(bts, data.c_str());
        cout << "::5:: " <<  endl;
        
        int echoStringLen = sizeof(data); //strlen(bts);
        
        // Send the string to the echo server
        //sock.send(echoString, echoStringLen);
        
        cout << "::6:: " <<  endl;
        sock.send(data, sizeof(data));
        
        // Buffer for echo string + \0
        int bytesReceived = 0;              // Bytes read on each recv()
        int totalBytesReceived = 0;         // Total bytes read
        
        // Receive the same string back from the server
        cout << "Received: " << endl;               // Setup to print the echoed string
        
        while (totalBytesReceived < echoStringLen) {
            // Receive up to the buffer size bytes from the sender
            if ((bytesReceived = (sock.recv(echoBuffer, RCVBUFSIZE))) <= 0) {
                cerr << "Unable to read";
                exit(1);
            }
            totalBytesReceived += bytesReceived;     // Keep tally of total bytes
            echoBuffer[bytesReceived] = '\0';        // Terminate the string!
            cout << "Received message: " << echoBuffer;                      // Print the echo buffer
        }
        cout << endl;
        
        // Destructor closes the socket
        
        
    } catch(SocketException &e)
    {
        cout << "::error:: " << e.what() << endl;
        cerr << e.what() << endl;
        exit(1);
    }
    
    //std::stringstream strm;
    //strm << echoBuffer;

    //pthread_cancel(thread_message);
    
    
}

void setMessage(motion::Message m, bool array)
{
    cout << "::setMessage::" << m.type() << endl;
    
    MessageStructThread.message         = m;
    MessageStructThread.array           = array;
    
    // run the streaming client as a separate thread
    runm = pthread_create(&thread_message, NULL, sendMessage, &MessageStructThread);
    if ( runm  != 0) {
        cerr << "Unable to create streamVideo thread" << endl;
        cout << "BroadcastSender pthread_create failed." << endl;
    }
    
    pthread_join(    thread_message,          (void**) &runm);
    
    pthread_cancel(thread_message);
    
}