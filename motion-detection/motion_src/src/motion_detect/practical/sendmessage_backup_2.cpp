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

#include "../b64/encode.h"
#include "../b64/decode.h"

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"


struct message_thread_args
{
    motion::Message message;
    bool array;
};
struct message_thread_args MessageStructThread;

const unsigned int RCVBUFSIZE = 500000;     // Size of receive buffer
const int MAXRCVSTRING = 4096;          // Longest string to receive


void * sendMessage (void * arg)
{
    
    struct message_thread_args *args = (struct message_thread_args *) arg;
    
    bool array = args->array;
    
    motion::Message m = args->message;
    send_proto = m;
    
    string servAddress = m.serverip();
    
    cout << "+++++++++++SENDING PROTO++++++++++++++" << endl;
    
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
       
        //string data;
        int size = m.ByteSize();
        char data[size];
        
        if (array)
        {
            cout << "ByteSize:: " << size <<  endl;
            try
            {
                m.SerializeToArray(&data, size);
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
        
        sock.send(data, sizeof(data));
        
        // Buffer for echo string + \0
        int bytesReceived = 0;              // Bytes read on each recv()
        int totalBytesReceived = 0;         // Total bytes read
        
        GOOGLE_PROTOBUF_VERIFY_VERSION;
        
        motion::Message mm;

        while (totalBytesReceived < sizeof(data)) {
            // Receive up to the buffer size bytes from the sender
            if ((bytesReceived = (sock.recv(echoBuffer, RCVBUFSIZE))) <= 0) {
                cerr << "Unable to read";
                exit(1);
            }
            totalBytesReceived += bytesReceived;     // Keep tally of total bytes
            echoBuffer[bytesReceived] = '\0';        // Terminate the string!
            cout << "Received message: " << echoBuffer;                      // Print the echo buffer
            
            //const string & data = echoBuffer;
            receive_proto.Clear();
            receive_proto = mm;
            
            //restoreProto(true, echoBuffer, "MAT_REMOTE.jpg");
            
            cout << "+++++++++++RESTORING PROTO++++++++++++++" << endl;
            
            cout << "::1::" << endl;
            
            bool isarray = true;
            
            if (isarray)
            {
                mm.ParseFromArray(&echoBuffer, sizeof(echoBuffer));
            }
            else
            {
                //mm.ParseFromString(&data);
            }
            
            cout << "::2::" << endl;
            
            int size_final = mm.ByteSize();
            
            cout << "width      : " << mm.width()   << endl;
            cout << "rows       : " << mm.rows()    << endl;
            cout << "height     : " << mm.height()  << endl;
            cout << "cols       : " << mm.cols()    << endl;
            cout << "Mat type   : " << mm.type()    << endl;
            cout << "Mat size   : " << mm.size()    << endl;
            cout << "Proto size : " << size_final   << endl;
            
            std::string mdata = mm.data();
            
            std::stringstream input_o;
            input_o << mdata;
            
            // Base64 decode the stringstream
            base64::decoder D;
            std::stringstream decoded;
            D.decode(input_o, decoded);
            
            //Store into proto
            std::string decoded_str = decoded.str();
            
            // Allocate a buffer for the pixels
            char* data_r = new char[mm.size()];
            // Read the pixels from the stringstream
            decoded.read(data_r, mm.size());
            
            // Construct the image (clone it so that it won't need our buffer anymore)
            cv::Mat m_d = cv::Mat(mm.height(), mm.width(), mm.typemat(), data_r).clone();
            
            imwrite("image_response_mac.jpg", m_d);
            
            
        }
        cout << endl;
        
        delete data;
        
        // Destructor closes the socket
        
        
    } catch(SocketException &e)
    {
        cout << "::error:: " << e.what() << endl;
        cerr << e.what() << endl;
        exit(1);
    }
    
    //std::stringstream strm;
    //strm << echoBuffer;

    pthread_cancel(thread_message);
    
    
}

void setMessage(motion::Message m, bool array)
{

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