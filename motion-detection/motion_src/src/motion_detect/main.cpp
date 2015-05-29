/* 
 * File:   main.cpp
 * Author: jose
 *
 * Created on April 19, 2015, 11:23 PM
 */

#include "global.h"

#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>      
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "practical/PracticalSocket.h" 
#include <cstdlib>           
#include <unistd.h>    
#include <string.h> /* for strncpy */
#include <sys/ioctl.h>
#include <net/if.h>
#include <ctime>
#include <sys/time.h> // {get,set}timeofday

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <errno.h>
#include "recognition/detection.h"
#include <vector>
#include <functional>
//#include "streamvideo.h"
#include "remotecam.hpp"
//#include "rtp/hwclock.c"

using namespace std;
using namespace cv;

const unsigned int TCP_ECHO_PORT                = 5010;
const unsigned int UDP_PORT                     = 5020;
const unsigned int STREAMING_VIDEO_PORT         = 5030;
const unsigned int TCP_MSG_PORT                 = 5040;

const unsigned int CONNECT                  = 1000;
const unsigned int STOP_STREAMING           = 1002;
const unsigned int PAUSE_STREAMING          = 1003;

const unsigned int START_RECOGNITION        = 1004;
const unsigned int STOP_RECOGNITION         = 1005;

const unsigned int DISSCONNECT              = 1006;

const unsigned int GET_TIME                 = 1007;


void * ThreadMain(void *clntSock);
int HandleTCPClient(TCPSocket *sock);
void RunUICommand(int result, string from_ip);
void * sendMessage (void * arg);
void setMessage(char * message_send);

// Threading
pthread_mutex_t tcpMutex, streamingMutex;
pthread_t thread_broadcast, thread_echo, thread_socket,
thread_message, thread_recognition, thread_wait_echo;


//Threads
int runt, runb, runs, runr, runl, runm;

/// TCP Streaming
int             clientSock;
char*     	server_ip;
int       	server_port;
int       	server_camera;

//Threads
pthread_cond_t echo_response;
pthread_mutex_t echo_mutex;
bool echo_received;
int resutl_echo;
std::string from_ip;

//TCP
std::string local_ip;
string NETWORK_IP;

const unsigned int RCVBUFSIZE = 32;    // Size of receive buffer
const int MAXRCVSTRING = 4096; // Longest string to receive

std::string getGlobalIntToString(int id);

int getGlobalStringToInt(std::string id);

struct message_thread_args
{
    unsigned int port;
    string machine_ip;
    char * message;
    
};
struct message_thread_args MessageStructThread;

int getGlobalStringToInt(std::string id)
{
    return atoi( id.c_str() );
}

std::string getGlobalIntToString(int id)
{
    std::stringstream strm;
    strm << id;

    return strm.str();
}

//Detection
//int scene_changes_amount = 0;

struct recognition_thread_args 
{
    bool writeImages;
    bool writeCrops;
};
struct recognition_thread_args RecognitionahaStructThread;

std::string getIpAddress();

void * sendMessage (void * arg)
{
 
    struct message_thread_args *args = (struct message_thread_args *) arg;
    
    string servAddress = args->machine_ip;
    char *echoString = args->message;   // Second arg: string to echo
    
    int echoStringLen = strlen(echoString);   // Determine input length
    
    unsigned short echoServPort = args->port;
    
    char echoBuffer[RCVBUFSIZE + 1];
    
    try
    {
        
        std::cout << "Establish connection with the echo server" << servAddress << " " << echoServPort << std::endl;
        
        // Establish connection with the echo server
        TCPSocket sock(servAddress, echoServPort);
        
        // Send the string to the echo server
        sock.send(echoString, echoStringLen);
        
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
        
    } catch(SocketException &e) {
        cerr << e.what() << endl;
        exit(1);
    }
    
    std::stringstream strm;
    strm << echoBuffer;
    
    
}

void setMessage(char * message_send)
{
    control_computer_ip = from_ip;
    
    MessageStructThread.port            = TCP_ECHO_PORT;
    MessageStructThread.machine_ip      = control_computer_ip;
    MessageStructThread.message         = message_send;
    
    cout << "TCP_PORT." << TCP_MSG_PORT <<  " control_computer_ip: " << control_computer_ip << " message_send " << message_send << endl;
    
    // run the streaming client as a separate thread
    runm = pthread_create(&thread_message, NULL, sendMessage, &MessageStructThread);
    if ( runm  != 0) {
        cerr << "Unable to create streamVideo thread" << endl;
        cout << "BroadcastSender pthread_create failed." << endl;
    }
    
    pthread_join(    thread_message,          (void**) &runm);
    
    pthread_cancel(thread_message);
    
}

void RunUICommand(int result, string from_ip)
{
    
    char * message_send;
    std::string command = "HOLA";

    
    switch (result)
    {
            
        case GET_TIME:
            
            //message_send = new char[command.size() + 1];
            //std::copy(command.begin(), command.end(), message_send);
            //message_send[command.size()] = '\0';
            
            struct timeval tv;
            time_t nowtime;
            struct tm *nowtm;
            char tmbuf[64], buf[64];
            
            gettimeofday(&tv, NULL);
            nowtime = tv.tv_sec;
            nowtm = localtime(&nowtime);
            strftime(tmbuf, sizeof tmbuf, "%Y-%m-%d %H:%M:%S", nowtm);
            snprintf(buf, sizeof buf, "%s.%06d", tmbuf, tv.tv_usec);
            
            cout << "buf TIME " << buf << "tmbuf TIME " << buf << endl;
            
            setMessage(buf);
            
            break;
        
        
        case CONNECT:
            
            connectStreaming(from_ip);
            
            break;
            
        case START_RECOGNITION:
               
            RecognitionahaStructThread.writeCrops = true;
            RecognitionahaStructThread.writeImages = true;
            
            runr = pthread_create(&thread_recognition, NULL, startRecognition, &RecognitionahaStructThread);
            if ( runr  != 0) {
                cerr << "Unable to create thread" << endl;
                cout << "startRecognition pthread_create failed." << endl;
            }
                
            pthread_join(    thread_recognition,               (void**) &runr);
               
            break;
            
        case STOP_RECOGNITION:
                
            pthread_cancel(thread_recognition);
            
            break;
            
    }
    
}


// TCP client handling function
int HandleTCPClient(TCPSocket *sock)
{
    
  int value;
  cout << "Handling client ";
  string from;
  try {
     from = sock->getForeignAddress(); 
    cout << from << ":";
  } catch (SocketException &e) {
    cerr << "Unable to get foreign address" << endl;
  }
  
  from_ip = from;
    
  try {
    cout << sock->getForeignPort();
  } catch (SocketException &e) {
    cerr << "Unable to get foreign port" << endl;
  }
  cout << " with thread " << pthread_self() << endl;

  int bytesReceived = 0;              // Bytes read on each recv()
    int totalBytesReceived = 0;         // Total bytes read
  
  // Send received string and receive again until the end of transmission
  char echoBuffer[RCVBUFSIZE];
  int recvMsgSize;
  std::string mesagge;
  while ((recvMsgSize = sock->recv(echoBuffer, RCVBUFSIZE)) > 0) { // Zero means
     
      totalBytesReceived += recvMsgSize;     // Keep tally of total bytes
      echoBuffer[recvMsgSize] = '\0';        // Terminate the string!
      cout << "Received message: " << echoBuffer << endl;                      // Print the echo buff
      
       std::stringstream strmm;
       strmm << echoBuffer;
       mesagge = strmm.str();
       value = atoi(mesagge.c_str());
     
    // end of transmission
    // Echo message back to client
    sock->send(echoBuffer, recvMsgSize);
  }
  // Destructor closes socket
  
  // Run command from the UI
  //RunUICommand(mesagge, from);
  
  return value;
    
}

void * ThreadMain(void *clntSock)
{

  // Guarantees that thread resources are deallocated upon return
  pthread_detach(pthread_self());
    
    pthread_mutex_lock(&echo_mutex);
    
        resutl_echo = HandleTCPClient((TCPSocket *) clntSock);
        pthread_cond_signal(&echo_response);
        cout << "RESULT IN THREAD:: " << resutl_echo << " from_ip: " << from_ip << endl;
    
    pthread_mutex_unlock(&echo_mutex);

    delete (TCPSocket *) clntSock;
    //pthread_exit((void *) resutl);
    pthread_exit(NULL);
    return (void *) resutl_echo;
}

void * watch_echo (void * t)
{
    pthread_mutex_lock(&echo_mutex);
    while (!echo_received)
    {
        pthread_cond_wait(&echo_response, &echo_mutex);
        std::cout << "RECIBIDO!!!. resutl_echo " << resutl_echo << endl;
        
        RunUICommand(resutl_echo, from_ip);
        
    }
    pthread_mutex_lock(&echo_mutex);
    pthread_exit(NULL);
}

void * socketThread (void * args)
{
    
    // TCP
    unsigned short tcp_port = TCP_ECHO_PORT;
    void *status;
    
    pthread_mutex_init(&echo_mutex, NULL);
    pthread_cond_init(&echo_response, NULL);
    
    runl = pthread_create (&thread_wait_echo, NULL, watch_echo, NULL);
    if ( runl  != 0)
    {
        cerr << "Unable to create ThreadMain thread" << endl;
        cout << "ThreadM:::.in pthread_create failed." << endl;
        exit(1);
    }
    
    try {
        TCPServerSocket servSock(tcp_port);   // Socket descriptor for server
        for (;;) {      // Run forever
            cout << "new TCPServerSocket() runt::" << runt << endl;
            // Create separate memory for client argument
            TCPSocket *clntSock = servSock.accept();
            //runt
            runt =  pthread_create(&thread_echo, NULL, ThreadMain, (void *) clntSock);
            if ( runt  != 0)
            {
                cerr << "Unable to create ThreadMain thread" << endl;
                cout << "ThreadM:::.in pthread_create failed." << endl;
                exit(1);
            }
            cout << "ThreadMain pthread_create created!!!!!." << endl;
            pthread_join(    thread_echo,               (void**) &runt);
            //cout << "STATUS!!! = " << runt << endl;
            //return (void *) runt;
        }
        
    } catch (SocketException &e) {
        cerr << e.what() << endl;
        exit(1);
    }

}

std::string getIpAddress () {
    
    int fd;
    struct ifreq ifr;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;
    /* I want IP address attached to "eth0" */
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);
    ioctl(fd, SIOCGIFADDR, &ifr);
    close(fd);
    
    char * ipaddrs = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
    
    std::string ip_txt(ipaddrs);
    
    std::cout << " IP TERMINAL :: " << ip_txt << '\n';
       
    return ip_txt;
}

void split(const string& s, char c,
           vector<string>& v) {
   string::size_type i = 0;
   string::size_type j = s.find(c);

   while (j != string::npos) {
      v.push_back(s.substr(i, j-i));
      i = ++j;
      j = s.find(c, j);

      if (j == string::npos)
         v.push_back(s.substr(i, s.length()));
   }
}

void * broadcastsender ( void * args ) {
    
  
   std::string sech = getIpAddress();
   cout  <<  "IP: " << sech << endl;
   
    //size_t f = sech.find("127.0.0.1");
    //sech.replace(f, std::string("127.0.0.1").length(), "");
    
    //if (sech.find("-") != std::string::npos) {
    //    std::string delimiter = "-";
    //    std::string token = sech.substr(1, sech.find(delimiter)); // token is "scott"
    //}
    
   local_ip = sech;

   vector<string> ip_vector;
   split(local_ip, '.', ip_vector);
   
   for (int i=0; i<ip_vector.size(); i++)
   {
        if ( i==0 | i==1) 
        {
             NETWORK_IP +=  ip_vector[i] + ".";
        }
        else if ( i==2 )
        {
             NETWORK_IP +=  ip_vector[i];
        } 
   }
   
   std::string destAddress = NETWORK_IP + ".255";
   unsigned short destPort = UDP_PORT;
   char *sendString = new char[sech.length() + 1];
   std::strcpy(sendString, sech.c_str());
   char recvString[MAXRCVSTRING + 1];
  
    try 
    {
    UDPSocket sock;
    cout  <<  "Sent Broadcast to: " << destAddress << " port: " << destPort << " sent: " << sendString << endl;
    int countud;
    for (;;) {
      pthread_mutex_lock(&tcpMutex);
            sock.sendTo(sendString, strlen(sendString), destAddress, destPort);
            cout << "UPD Send: " << countud << endl;
            countud++;
      pthread_mutex_unlock(&tcpMutex);
      sleep(3);
    }
    delete [] sendString;
  
  } catch (SocketException &e) {
    cerr << e.what() << endl;
    cout  <<  "Error: " << cerr << endl;
    exit(1);
  }

}

int main (int argc, char * const argv[])
{    
  
  
   pthread_mutex_init(&tcpMutex, 0);  
   
   // UDP
   runb = pthread_create(&thread_broadcast, NULL, broadcastsender, NULL);
   if ( runb  != 0) {
       cerr << "Unable to create thread" << endl;
       cout << "BroadcastSender pthread_create failed." << endl;
   }
   
    //Socket
    runs = pthread_create(&thread_socket, NULL, socketThread, NULL);
    if ( runs  != 0) {
        cerr << "Unable to create thread" << endl;
        cout << "BroadcastSender pthread_create failed." << endl;
    }
    
   cout << "join thread_broadcast" << endl;
   //pthread_join(    thread_broadcast,          (void**) &runb);
   cout << "join thread_echo" << endl;
   pthread_join(    thread_socket,               (void**) &runs);
    
    cout << "LAST!!! = " << runs << endl;
    
   
   pthread_mutex_destroy(&tcpMutex);
   
   cout << "return 0" << endl;
   
   return 0;
  
    
}

            
        

