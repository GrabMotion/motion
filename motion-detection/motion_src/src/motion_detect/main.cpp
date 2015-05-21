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

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <errno.h>
#include "recognition/detection.h"
#include <vector>
#include <functional>
//#include "streamvideo.h"
#include "remotecam.hpp"

using namespace std;
using namespace cv;

const unsigned int TCP_PORT                 = 5010;
const unsigned int UDP_PORT                 = 5020;

const unsigned int CONNECT                  = 1000;
const unsigned int STOP_STREAMING           = 1002;
const unsigned int PAUSE_STREAMING          = 1003;

const unsigned int START_RECOGNITION        = 1004;
const unsigned int STOP_RECOGNITION         = 1005;

// Threading
pthread_mutex_t tcpMutex, streamingMutex;
pthread_t thread_broadcast, thread_echo, /*thread_streaming,*/ thread_recognition;
//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//Threads
int runt, runb, /*runs,*/ runr;

/// TCP Streaming
int             clientSock;
char*     	server_ip;
int       	server_port;
int       	server_camera;


//TCP
std::string local_ip;
string NETWORK_IP;

const unsigned int RCVBUFSIZE = 32;    // Size of receive buffer
const int MAXRCVSTRING = 4096; // Longest string to receive

std::string getGlobalIntToString(int id);

int getGlobalStringToInt(std::string id);


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

void *ThreadMain(void *arg);    
void HandleTCPClient(TCPSocket *sock);

std::string getIpAddress();

void RunUICommand(std::string param, string from_ip){
    
    
    switch (getGlobalStringToInt(param)){
        
        case CONNECT:
            
            time_t currentTime;
            struct tm *localTime;
            
            time( &currentTime );                   // Get the current time
            localTime = localtime( &currentTime );  // Convert the current time to the local time
            
            int Day     ,
                Month   ,
                Year    ,
                Hour    ,
                Min     ,
                Sec     ;
            
            Day    = localTime->tm_mday;
            Month  = localTime->tm_mon + 1;
            Year   = localTime->tm_year + 1900;
            Hour   = localTime->tm_hour;
            Min    = localTime->tm_min;
            Sec    = localTime->tm_sec;
            
            //std::cout << "This program was exectued at: " << Hour << ":" << Min << ":" << Sec << std::endl;
            //std::cout << "And the current date is: " << Day << "/" << Month << "/" << Year << std::endl;
        
            std::cout << "Message received at: " << Hour << ":" << Min << ":" << Sec << std::endl;
            std::cout << "And the current date is: " << Day << "/" << Month << "/" << Year << std::endl;
            std::cout << "+++++++++++++++++++++++" << endl;
            
            
            //sprintf(newdate,"date --set %s %s %s %s", month, date.substr(8,2), date.substr(0,4), newtime);
            //system(newdate);
            
            //time_t  timev;
            //me(&timev);
            //std::stringstream strmt;
            //strmt << timev;
            //std::string stime_now = strmt.str();
            //cout << "TIME NOW!!!." << timev << endl;
            
            connectStreaming(from_ip);
            
            
            break;
            
        case STOP_STREAMING:
        case PAUSE_STREAMING:
        
            //if (pthread_cancel(thread_streaming)) {
            //}
            
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
                
                //if (runr > 0) 
            
                break;
            
    }
    
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
      
       std::stringstream strmm;
       strmm << echoBuffer;
       mesagge = strmm.str();
     
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

            
        

