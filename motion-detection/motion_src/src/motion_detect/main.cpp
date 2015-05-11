/* 
 * File:   main.cpp
 * Author: jose
 *
 * Created on April 19, 2015, 11:23 PM
 */

#include <cstring>
#include <iostream>
//#include "socket/ServerSocket.h"
//#include "socket/SocketException.h"
#include <string>
#include <sstream>
#include <stdio.h>      
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "practical/PracticalSocket.h" // For UDPSocket and SocketException
#include <cstdlib>           // For atoi()
#include <unistd.h>           // For sleep()
#include "global.h"
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <errno.h>
#include "recognition/detection.h"


using namespace std;
using namespace cv;

const unsigned int RCVBUFSIZE = 32;    // Size of receive buffer
const int MAXRCVSTRING = 4096; // Longest string to receive

// Threading
pthread_mutex_t tcpMutex;
pthread_t thread_broadcast, thread_echo, thread_streaming, thread_recognition;
int runt, runb, runs, runr;

//TCP
string control_computer_ip;

/// TCP Streaming
VideoCapture    capture;
Mat             img0, img1, img2;
int             is_data_ready = 1;
int             clientSock;
char*     	server_ip;
int       	server_port;
int       	server_camera;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct stream_thread_args 
{
    unsigned int port;
    int cam;
};
struct stream_thread_args StreamingStructThread;

struct recognition_thread_args 
{
    bool writeImages;
    bool writeCrops;
};
struct recognition_thread_args RecognitionahaStructThread;

void *ThreadMain(void *arg);    
void HandleTCPClient(TCPSocket *sock);

void quitStrSocket(int retval);
void connectStreaming(string from_ip);

void* streamVideo(void * arg)
{
   
     struct stream_thread_args *args = (struct stream_thread_args *) arg;
     
     std::string cip = control_computer_ip;
     char *control_remote_ip = new char[cip.length() + 1];
     std::strcpy(control_remote_ip, cip.c_str());
     
     server_ip = control_remote_ip;
     server_port = args->port;
     server_camera = args->cam;
     
     //--------------------------------------------------------
    //networking stuff: socket, bind, listen
    //--------------------------------------------------------
    int                 localSocket,
                        remoteSocket,
                        port = server_port;                               
 
    struct  sockaddr_in localAddr,
                        remoteAddr;
    
       
    int addrLen = sizeof(struct sockaddr_in);
    
    localSocket = socket(AF_INET , SOCK_STREAM , 0);
    if (localSocket == -1)
         perror("socket() call failed!!");
       
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = htons( port );
 
    int optval = 1;
    if (setsockopt(localSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) < 0)
    {
        perror("Cannot set SO_REUSEADDR option");
        std::cerr << "on listen socket (%s)\n" << strerror(errno) << endl;
    }

    if( bind(localSocket,(struct sockaddr *)&localAddr , sizeof(localAddr)) < 0) 
    {
         perror("Can't bind() socket");
         exit(1);
    }
    
    //Listening
    listen(localSocket , 1);
    
    std::cout <<  "Waiting for connections...\n"
              <<  "Server Port:" << port << std::endl;
 
    //accept connection from an incoming client
    remoteSocket = accept(localSocket, (struct sockaddr *)&remoteAddr, (socklen_t*)&addrLen);
    if (remoteSocket < 0) {
        perror("accept failed!");
        exit(1);
    }
    std::cout << "Connection accepted" << std::endl;
 
 
    //----------------------------------------------------------
    //OpenCV Code
    //----------------------------------------------------------
    int capDev = server_camera;
 
    VideoCapture cap(capDev); // open the default camera
    Mat img, imgGray;
    img = Mat::zeros(480 , 640, CV_8UC1);    
 
    if (!cap.isOpened()) {
        quitStrSocket(1);
    }
    
    //make it continuous
    if (!img.isContinuous()) {
        img = img.clone();
    }
 
    int imgSize = img.total() * img.elemSize();
    int bytes = 0;
    int key;
 
    //make img continuos
    if ( ! img.isContinuous() ) { 
          img = img.clone();
          imgGray = img.clone();
    }
        
    std::cout << "Image Size:" << imgSize << std::endl;
 
    while(1) {

        /* get a frame from camera */
        cap >> img;

        //do video processing here 
        flip(img, img, 1);
        cvtColor(img, imgGray, CV_BGR2GRAY);

        pthread_mutex_lock(&tcpMutex);
        //send processed image
            if ((bytes = send(remoteSocket, imgGray.data, imgSize, 0)) < 0){
                 std::cerr <<   "\n--> bytes = " << bytes << std::endl;
                 quitStrSocket(1);
            }
            cout << "sending streaming data: " << bytes  << endl;
        pthread_mutex_unlock(&tcpMutex);

        
    }

    
    return 0;
       
}

void quitStrSocket(int retval)
{
        if (clientSock){
                close(clientSock);
        }
        if (capture.isOpened()){
                capture.release();
        }
        if (!(img0.empty())){
                (~img0);
        }
        if (!(img1.empty())){
                (~img1);
        }
        if (!(img2.empty())){
                (~img2);
        }
        pthread_mutex_destroy(&mutex);
        exit(retval);
}


void RunUICommand(std::string param, string from_ip){
    
    switch (getGlobalStringToInt(param)){
        
        case CONNECT:
               
            connectStreaming(from_ip);
            
            break;
            
        case STOP_STREAMING:
        case PAUSE_STREAMING:
         
            if (pthread_cancel(thread_streaming)) {
                quitStrSocket(1);
            }
            
            break;  
            
           case START_RECOGNITION:
               
                RecognitionahaStructThread.writeCrops = true;
                RecognitionahaStructThread.writeImages = true;
            
                runr = pthread_create(&thread_recognition, NULL, startRecognition, &RecognitionahaStructThread);
                if ( runr  != 0) {
                    cerr << "Unable to create thread" << endl;
                    cout << "BroadcastSender pthread_create failed." << endl;
                }
                
                pthread_join(    thread_recognition,               (void**) &runr); 
               
                break;
            
            case STOP_RECOGNITION:
                
                if (runr > 0) 
            
                break;
            
    }
    
}

void connectStreaming(string from_ip)
{
        control_computer_ip = from_ip;
        // TCP Streaming
        StreamingStructThread.port = STREAMING_VIDEO_PORT; 
        StreamingStructThread.cam = 0;

         // run the streaming client as a separate thread 
        runs = pthread_create(&thread_streaming, NULL, streamVideo, &StreamingStructThread); 
        if ( runs  != 0) {
            cerr << "Unable to create streamVideo thread" << endl;
           cout << "BroadcastSender pthread_create failed." << endl;
        }

        pthread_join(    thread_streaming,          (void**) &runs);

}


// TCP client handling function
void HandleTCPClient(TCPSocket *sock) 
{
  cout << "Handling client ";
  string from;
  try {
     from = sock->getForeignAddress(); 
    cout << from << ":";
  } catch (SocketException &e) {
    cerr << "Unable to get foreign address" << endl;
  }
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
      cout << "Received message: " << echoBuffer;                      // Print the echo buff
      
       std::stringstream strm;
       strm << echoBuffer;
       mesagge = strm.str();
     
    // end of transmission
    // Echo message back to client
    sock->send(echoBuffer, recvMsgSize);
  }
  // Destructor closes socket
  
  // Run command from the UI
  RunUICommand(mesagge, from);
  
}

void *ThreadMain(void *clntSock) {
  // Guarantees that thread resources are deallocated upon return  
  pthread_detach(pthread_self()); 

  // Extract socket file descriptor from argument  
  HandleTCPClient((TCPSocket *) clntSock);

  delete (TCPSocket *) clntSock;
  return NULL;
}

std::string getIpAddress () {
    
    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;
    void * tmpAddrPtr = NULL;
    std::string address; 
    std::stringstream strm6;
    std::stringstream strm;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) {
            continue;
        }
        if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer); 

            strm << addressBuffer;
            address = strm.str(); 
            
        } else if (ifa->ifa_addr->sa_family == AF_INET6) { // check it is IP6
            // is a valid IP6 Address
            tmpAddrPtr=&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            printf("%s IP Address %s\n", ifa->ifa_name, addressBuffer); 
           
            strm6 << addressBuffer;
            address = strm.str(); 
        } 
    }
    if (ifAddrStruct!=NULL) 
       freeifaddrs(ifAddrStruct);
  
    address.erase (0,9);
       
    return address;
}

void * broadcastsender ( void * args ) {
    
   std::string destAddress = NETWORK_IP + ".255";
   unsigned short destPort = UDP_PORT;
   std::string sech = getIpAddress();
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
      sleep(5);  
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
   // TCP
   unsigned short tcp_port = TCP_PORT;
   try {
        TCPServerSocket servSock(tcp_port);   // Socket descriptor for server  
        for (;;) {      // Run forever  
            // Create separate memory for client argument 
            TCPSocket *clntSock = servSock.accept();
            runt = pthread_create(&thread_echo, NULL, ThreadMain, (void *) clntSock);
            if ( runb  != 0) 
            {
                cerr << "Unable to create ThreadMain thread" << endl;
                cout << "ThreadMain pthread_create failed." << endl;
                exit(1);
            }  
        }
    
    } catch (SocketException &e) {
        cerr << e.what() << endl;
        exit(1);
    }
    
   pthread_join(    thread_broadcast,          (void**) &runb);
   pthread_join(    thread_echo,               (void**) &runt); 
   
   pthread_mutex_destroy(&tcpMutex);
   
   cout << "return 0" << endl;
   
   return 0;
  
    
}

            
        

