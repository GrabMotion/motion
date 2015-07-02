#include "../practical/sendmessage.h"

#include "../b64/base64.h"

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
#include <cstdlib>

#include <unistd.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

using namespace std;
using namespace google::protobuf::io;

vector<std::string> msg_split_vector;
motion::Message::ActionType value_response;
bool noend = false;

struct param_struc
{
    motion::Message message;
    motion::Message::SocketType type;
};
struct param_struc ParamStruc;

//const unsigned int RCVBUFSIZE = 200000;     // Size of receive buffer
//const int MAXRCVSTRING = 4096;          // Longest string to receive

std::string IntToString ( int number )
{
    std::ostringstream oss;
    
    // Works just like cout
    oss<< number;
    
    // Return the underlying string
    return oss.str();
}


int div_ceil(int numerator, int denominator)
{
    std::div_t res = std::div(numerator, denominator);
    return res.rem ? (res.quot + 1) : res.quot;
}


vector<std::string> splitStringBySize(std::string payload, int package)
{
    int s3 = div_ceil(payload.size(), package);
    vector<std::string> splitted;
    int pos, length, buff=package;
    
    for (int i=1; i<s3; i++)
    {
        if (i==1)
        {
            pos = 0;
            length = package;
        }
        else if (i==(s3-1))
        {
            buff += package;
            pos = buff;
            length = payload.size() + 100;
        }
        else
        {
            buff += package;
            pos = buff;
            length = package;
        }
    std:string block;
        block = payload.substr(pos, length);
        splitted.push_back(block);
    }
    
    return splitted;
}


void * messageThread (void * arg)
{

    struct param_struc *args = (struct param_struc *) arg;
    
    motion::Message m                       = args->message;
    motion::Message::SocketType type        = args->type;
    
    string servAddress;
    
    if (T_PROTO.has_serverip())
    {
       servAddress = T_PROTO.serverip();
    }
    else
    {
        cout << "NO SERVER IP ADDRESS, break!!" << endl;
        return 0;
    }
    
    cout << "::ServAddress:: " << servAddress <<  endl;
    
    int port = motion::Message::TCP_MSG_PORT;
    
    cout << "::Port:: " << port << endl;
    
    int count_sent_split;

    try
    {
        
        std::cout << "Connecting with :: " << servAddress << " port :: " << port << std::endl;
        
        // Establish connection with the echo server
        TCPSocket sock(servAddress, port);

        //Initialize objects to serialize.
        int size = m.ByteSize();
        char data[size];
        string datastr;
        switch (type)
        {
            case motion::Message::SOCKET_PROTO_TOARRAY:
                try
                {
                    m.SerializeToArray(&data, size);
                }
                catch (google::protobuf::FatalException fe)
                {
                    std::cout << "PbToZmq " << fe.message() << std::endl;
                }
                break;
                
            case motion::Message::SOCKET_PROTO_TOSTRING:
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
                break;
    
                
        }
        
        std:string encoded_proto = base64_encode(reinterpret_cast<const unsigned char*>(data),sizeof(data));
        std::string msg;
        
        if ( size > motion::Message::SOCKET_BUFFER_SMALL_SIZE )
        {
            
            cout << "size > SOCKET_BUFFER_SMALL_SIZE " << size <<  endl;

            msg_split_vector = splitStringBySize(encoded_proto, motion::Message::SOCKET_BUFFER_MEDIUM_SIZE);
            
            std::stringstream ss;
            
            string header =
            
            "PROTO_START_DELIMETER" +
            IntToString (motion::Message::SPLITTED_MESSAGE)         + ":" +
            IntToString (motion::Message::SOCKET_PROTO_TOARRAY)     + ":" +
            IntToString(msg_split_vector.size())                    + ":" +
            IntToString(size)                                       + ":" +
            IntToString(motion::Message::SOCKET_BUFFER_SMALL_SIZE)  + ":" +
            "PROTO_STOP_DELIMETER";

            msg = header + msg_split_vector.at(0);
            cout << "package: " << 0 << " header size "  << msg.size() <<  endl;
            sock.send(msg.c_str(), msg.size());
            count_sent_split++;
        }
        else
        {
            cout << "size < SOCKET_BUFFER_SMALL_SIZE " << encoded_proto <<  endl;
            string header =
            "PROTO_START_DELIMETER" + IntToString (motion::Message::SINGLE_MESSAGE) + ":" +
                IntToString (motion::Message::SOCKET_PROTO_TOARRAY)                 +
            "PROTO_STOP_DELIMETER";
            msg = header + encoded_proto;
            sock.send(msg.c_str(), msg.size());
        }
            
        google::protobuf::ShutdownProtobufLibrary();
                

        char echoBuffer[motion::Message::SOCKET_BUFFER_SMALL_SIZE];    // Buffer for echo string + \0
        int bytesReceived = 0;              // Bytes read on each recv()
        int totalBytesReceived = 0;         // Total bytes read
        
        
        while (!noend)
        {
            
            cout << "Receiving # : " << count_sent_split << " at: " << getTimeRasp() << endl;
        
            if ((bytesReceived = (sock.recv(echoBuffer, motion::Message::SOCKET_BUFFER_SMALL_SIZE))) <= 0) {
                cerr << "Unable to read!!!!!!!!";
            }
            
            totalBytesReceived += bytesReceived;
            echoBuffer[bytesReceived] = '\0';

            const string & dataresponse = echoBuffer;
            
            GOOGLE_PROTOBUF_VERIFY_VERSION;
            motion::Message ms;
            ms.ParseFromString(dataresponse);
            
            value_response = ms.type();
            
            cout << "Type Received : " << value_response << endl;
            
            if (value_response==200) //motion::Message::RESPONSE_OK)
            {
                    cout << "RESPONSE OK" << endl;
                    count_sent_split=0;
                    noend=true;
            }
            else if (value_response==201) //motion::Message::RESPONSE_NEXT)
            {
                    cout << "RESPONSE NEXT at: " << getTimeRasp() << endl;
                    std::string msgr = msg_split_vector.at(count_sent_split);
                    sleep(10000);
                    cout << "Sending at: " << getTimeRasp() << endl;
                    sock.send(msgr.c_str(), msgr.size());
                
                    count_sent_split++;
                    cout << "package: " << count_sent_split << " size "  << msgr.size() <<  endl;
            }
            else if (value_response==202) //motion::Message::RESPONSE_END)
            {
                    cout << "RESPONSE END" << endl;
                    count_sent_split=0;
                    noend=true;
            }
        
            google::protobuf::ShutdownProtobufLibrary();
        }
        cout << endl;
        
        
    } catch(SocketException &e)
    {
        cout << "::error:: " << e.what() << endl;
        cerr << e.what() << endl;
    }
    noend=false;
    cout << "Cancel thread." << endl;
    pthread_cancel(thread_message);
    
    
}

void sendMessage(motion::Message m, motion::Message::SocketType type)
{
    
    cout << "+++++++++++SENDING PROTO++++++++++++++" << endl;

    ParamStruc.message         = m;
    ParamStruc.type            = type;

    // run the streaming client as a separate thread
    runm = pthread_create(&thread_message, NULL, messageThread, &ParamStruc);
    if ( runm  != 0) {
        cerr << "Unable to create streamVideo thread" << endl;
        cout << "BroadcastSender pthread_create failed." << endl;
    }
    
    //pthread_join(    thread_message,          (void**) &runm);
    
    //pthread_cancel(thread_message);
    
}