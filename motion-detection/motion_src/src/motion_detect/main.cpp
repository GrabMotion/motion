/* 
 * File:   main.cpp
 * Author: jose
 *
 * Created on April 19, 2015, 11:23 PM
 */

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

#include <cstdlib>           
#include <unistd.h>    
#include <string.h> /* for strncpy */
#include <sys/ioctl.h>
#include <net/if.h>
#include <ctime>
#include <sys/time.h>
#include <errno.h>
#include <vector>
#include <functional>
#include <fstream>

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "practical/PracticalSocket.h" 
#include "practical/sendmessage.h"
#include "recognition/detection.h"
#include "protobuffer/motion.pb.h"
#include "socket/streamlistener.h"
#include "socket/netcvc.h"
//#include "socket/udp-send.c"

#include "b64/base64.h"

#include <unistd.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>

using namespace google::protobuf::io;

using namespace std;
using namespace cv;

void * ThreadMain(void *clntSock);
motion::Message::ActionType HandleTCPClient(TCPSocket *sock);
void RunUICommand(int result, string from_ip);
void * watch_amount (void * t);

// Threading
pthread_mutex_t tcpMutex, streamingMutex;
pthread_t thread_broadcast, thread_echo, thread_socket,
thread_message, thread_recognition, thread_wait_echo, thread_screenshot, thread_observer;

//Threads
int runt, runb, runs, runr, runl, runm, runw, runss, runo;

/// TCP Streaming
int         clientSock;
char*     	server_ip;
int       	server_port;
int       	server_camera;

//Threads
pthread_cond_t echo_response;
pthread_mutex_t echo_mutex;
bool echo_received;
motion::Message::ActionType resutl_echo;
std::string result_message;
std::string from_ip;

//TCP
std::string local_ip;
string NETWORK_IP;

//Protobuffer
motion::Message PROTO;
motion::Message R_PROTO;
motion::Message T_PROTO;
motion::Message takePictureToProto();

//Capture
bool stop_capture;
bool is_recognizing;
cv::Mat picture;

//UDP
int udpsend(motion::Message m);

#define RCVBUFSIZE 1024

//const unsigned int RCVBUFSIZE = 100000; //4096; //32;     // Size of receive buffer
const int MAXRCVSTRING = 4096;          // Longest string to receive

std::string getGlobalIntToString(int id);

int getGlobalStringToInt(std::string id);
char * setMessageValueBody(int value, std::string body);

int resutl_watch_detected;
std::string image_file_recognized;

struct arg_struct
{
    motion::Message message;
};
void * startRecognition(void * arg);
void * startObserver(void * arg);
void runCommand(motion::Message::ActionType value);
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
std::string getIpAddress();

char * setMessageValueBody(int value, std::string body)
{
    
    std::string command = getGlobalIntToString(value);
    
    char * message = new char[command.size() + 1];
    std::copy(command.begin(), command.end(), message);
    message[command.size()] = '\0';
    
    char * action = new char[body.size() + 1];
    std::copy(body.begin(), body.end(), action);
    action[body.size()] = '\0'; // don't forget the terminating 0
    
    char buffer[256];
    strncpy(buffer, message, sizeof(buffer));
    strncat(buffer, action, sizeof(buffer));
    
    return buffer;
}

struct screenshot_thread_args
{
    motion::Message msg;
};
struct screenshot_thread_args ScreenshorStructThread;

char *getTimeRasp()
{
    struct timeval tr;
    struct tm* ptmr;
    char time_rasp[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
    return time_rasp;
}


motion::Message takePictureToProto()
{
    
    cout << "CAPTURING !!!!!!!!!!!!!" << endl;
    
    CvCapture* capture = cvCreateCameraCapture(0);
    if (capture == NULL)
    {
        std::cout << "No cam found." << std::endl;

    }
    
    int w = 1280; //640; //1280; //320;
    int h = 720;  //480; //720;  //240;
    
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, w); //max. logitech 1280
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, h); //max. logitech 720
    
    IplImage* img=0;
    img = cvQueryFrame( capture );
    cvSaveImage("IplImage.JPG",img);
    
    Mat mat(h, w, CV_8U); //CV_8U); // CV_8UC3);
    mat = cvQueryFrame(capture);
    cvtColor(mat, mat, CV_RGB2GRAY);
    imwrite("MAT.jpg", mat);
    
    //Shared mat
    picture = mat;
    
    cout << "+++++++++++CREATING PROTO++++++++++++++" << endl;
    
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    
    motion::Message me;
    me.set_type(motion::Message::TAKE_PICTURE);
    me.set_serverip(PROTO.serverip());
    me.set_time(getTimeRasp());

    // We will need to also serialize the width, height, type and size of the matrix
    int width_s     = mat.cols;
    int height_s    = mat.rows;
    int type_s      = mat.type();
    int size_s      = mat.total() * mat.elemSize();
    
    // Initialize a stringstream and write the data
    int size_init = me.ByteSize();
    
    // Write the whole image data
    std::stringstream ss;
    ss.write((char*)    (&width_s),     sizeof(int));
    ss.write((char*)    (&height_s),    sizeof(int));
    ss.write((char*)    (&type_s),      sizeof(int));
    ss.write((char*)    (&size_s),      sizeof(int));
    ss.write((char*)     mat.data,      size_s);
    
    //Store into proto
    me.set_data(ss.str());
    
    //Write base64 to file for checking.
    std::string basefile = "base64oish.txt";    
    std::ofstream out;
    out.open (basefile.c_str());
    out << me.payload() << "\n";
    out.close();
    
    cout << "ByteSize: " << size_init <<  endl;
    
    cvReleaseCapture(&capture);
    
    google::protobuf::ShutdownProtobufLibrary();
    
    return me;
}

void* streamCast(void * arg)
{
    
    cout << "CAPTURING !!!!!!!!!!!!!" << endl;
    
    CvCapture* capture = cvCreateCameraCapture(0);
    if (capture == NULL)
    {
        std::cout << "No cam found." << std::endl;
        
    }
    
    int w = 1280; //640; //1280; //320;
    int h = 720;  //480; //720;  //240;
    
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, w); //max. logitech 1280
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, h); //max. logitech 720
    
    //IplImage* img=0;
    //img = cvQueryFrame( capture );
    //cvSaveImage("IplImage.JPG",img);
    
    Mat mat(h, w, CV_8UC3); //CV_8U); // CV_8UC3);
    mat = cvQueryFrame(capture);
    cvtColor(mat, mat, CV_RGB2GRAY);
    imwrite("MAT.jpg", mat);
    
    mat = (mat.reshape(0,1)); // to make it continuous
    int  imgSize = mat.total()*mat.elemSize();
    
    /////////////////
    
    int host_port= 1101;
    char* host_name="192.168.1.35";
    
    struct sockaddr_in my_addr;
    
    int bytes;
    
    int hsock;
    int * p_int;
    int err;
    
    hsock = socket(AF_INET, SOCK_STREAM, 0);
    if(hsock == -1){
        printf("Error initializing socket %d\n",errno);
        goto FINISH;
    }
    
    p_int = (int*)malloc(sizeof(int));
    *p_int = 1;
    
    if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
       (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
        printf("Error setting options %d\n",errno);
        free(p_int);
        goto FINISH;
    }
    free(p_int);
    
    my_addr.sin_family = AF_INET ;
    my_addr.sin_port = htons(host_port);
    
    memset(&(my_addr.sin_zero), 0, 8);
    my_addr.sin_addr.s_addr = inet_addr(host_name);
    if( connect( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
        if((err = errno) != EINPROGRESS){
            fprintf(stderr, "Error connecting socket %d\n", errno);
            goto FINISH;
        }
    }
    
    if( (bytes=send(hsock, (void *) mat.data, imgSize,0))== -1 ) {
        fprintf(stderr, "Error sending data %d\n", errno);
        goto FINISH;
    }
    fprintf(stderr, "Bytes sent %d\n", bytes);
    
    
    cvReleaseCapture(&capture);
    
    
FINISH:
    close(hsock);
    
}


int screenshot(motion::Message m)
{
    
    // TCP Streaming
    ScreenshorStructThread.msg = m;
    
    // run the streaming client as a separate thread
    runss = pthread_create(&thread_screenshot, NULL, streamCast, &ScreenshorStructThread);
    if ( runss  != 0) {
        cerr << "Unable to create streamVideo thread" << endl;
        cout << "BroadcastSender pthread_create failed." << endl;
    }
    
    pthread_join(    thread_screenshot,          (void**) &runss);
    
    pthread_cancel(thread_screenshot);
    
    return 0;
    
}

#define BUFLEN 2048
#define MSGS 5	/* number of messages to send */
#define SERVICE_PORT	21234	/* hard-coded port number */

int udpsend(motion::Message m)
{
    
    struct sockaddr_in myaddr, remaddr;
    int fd, i, slen=sizeof(remaddr);
    char *server = "192.168.1.35";	/* change this to use a different server */
    //char buf[BUFLEN];
    
    printf("Sending packet %d to %s port %d size %d\n", i, server, SERVICE_PORT, m.ByteSize()); //strlen(buf));
    

    std::string buf;
    m.SerializeToString(&buf);
    
    /* create a socket */
    
    if ((fd=socket(AF_INET, SOCK_DGRAM, 0))==-1)
        printf("socket created\n");
    
    /* bind it to all local addresses and pick any port number */
    
    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(0);
    
    if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("bind failed");
        return 0;
    }
    
    /* now define remaddr, the address to whom we want to send messages */
    /* For convenience, the host address is expressed as a numeric IP address */
    /* that we will convert to a binary format via inet_aton */
    
    memset((char *) &remaddr, 0, sizeof(remaddr));
    remaddr.sin_family = AF_INET;
    remaddr.sin_port = htons(SERVICE_PORT);
    if (inet_aton(server, &remaddr.sin_addr)==0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
    
    /* now let's send the messages */
    
    //for (i=0; i < MSGS; i++) {
    
    printf("Sending packet %d to %s port %d size %d\n", i, server, SERVICE_PORT, m.ByteSize()); //strlen(buf));
    //sprintf(buf, "This is packet %d", i);
    //if (sendto(fd, buf, strlen(buf), 0, (struct sockaddr *)&remaddr, slen)==-1)
    
    if(sendto(fd, buf.data(), strlen(buf.c_str()), 0, (struct sockaddr *)&remaddr, sizeof(remaddr)))
        perror("sendto");
    
    //}
    close(fd);
    return 0;
}

void * startObserver(void * arg)
{
    
    while (1)
    {
        if (R_PROTO.recognizing())
        {
            
           sendMessage(R_PROTO, motion::Message::SOCKET_PROTO_TOARRAY);
            
            
        } else
        {
            break;
        }
    }

}

void runCommand(motion::Message::ActionType value)
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    
    cout << "runCommand:: " << value << endl;
    
    switch (value)
    {
            
        case motion::Message::RESPONSE_OK:
        {
            cout << "RESPONSE OK" << endl;
            break;
        }
        case motion::Message::ENGAGE:
        {
            
            cout << "motion::Message::ENGAGE" << endl;
            
            motion::Message te;
            
            te.set_time(T_PROTO.time());
            te.set_type(motion::Message::ENGAGE);
            
            sendMessage(te, motion::Message::SOCKET_PROTO_TOARRAY);
            
            struct arg_struct arguments;
           
            arguments.message = PROTO;
            
            runo = pthread_create(&thread_observer, NULL, startObserver, (void*) &arguments);
            if ( runo  != 0) {
                cerr << "Unable to create thread" << endl;
                cout << "start observer pthread_create failed." << endl;
            }
            
            pthread_join( thread_recognition, (void**) &runr);
            
            break;
        }
        case motion::Message::TAKE_PICTURE:
        {
            motion::Message picture = takePictureToProto();
            sendMessage(picture, motion::Message::SOCKET_PROTO_TOARRAY);
            break;
        }
        case motion::Message::STRM_START:
        {
            netcvc();
            break;
        }
        case motion::Message::STRM_STOP:
        {
            stop_capture = true;
            break;
        }
        case motion::Message::GET_TIME:
        {
            motion::Message m;
            m.set_type(motion::Message::GET_TIME);
            m.set_serverip(PROTO.serverip());
            m.set_time(getTimeRasp());
            sendMessage(m, motion::Message::SOCKET_PROTO_TOARRAY);
            
            break;
        }
        case motion::Message::SET_TIME:
        {
            struct tm tmremote;
            char *bufr;
            
            bufr = new char[result_message.length() + 1];
            strcpy(bufr, result_message.c_str());
            
            cout << "::bufr:: " << bufr << endl;
            
            memset(&tmremote, 0, sizeof(struct tm));
            strptime(bufr, "%Y-%m-%d %H:%M:%S", &tmremote);
            
            std::cout << std::endl;
            std::cout << " Seconds  :"  << tmremote.tm_sec  << std::endl;
            std::cout << " Minutes  :"  << tmremote.tm_min  << std::endl;
            std::cout << " Hours    :"  << tmremote.tm_hour << std::endl;
            std::cout << " Day      :"  << tmremote.tm_mday << std::endl;
            std::cout << " Month    :"  << tmremote.tm_mon  << std::endl;
            std::cout << " Year     :"  << tmremote.tm_year << std::endl;
            std::cout << std::endl;
            
            struct tm mytime;
            struct timeval systime;
            char * text_time;
            
            mytime.tm_sec 	= tmremote.tm_sec  ;
            mytime.tm_min 	= tmremote.tm_min  ;
            mytime.tm_hour 	= tmremote.tm_hour ;
            mytime.tm_mday 	= tmremote.tm_mday ;
            mytime.tm_mon 	= tmremote.tm_mon  ;
            mytime.tm_year 	= tmremote.tm_year ;
            
            systime.tv_sec = mktime(&mytime);
            systime.tv_usec = 0;
            
            settimeofday(&systime, NULL);
            
            gettimeofday(&systime, NULL);
            
            text_time = ctime(&systime.tv_sec);
            
            printf("The system time is set to %s\n", text_time);
            
            motion::Message m;
            m.set_type(motion::Message::TIME_SET);
            m.set_serverip(PROTO.serverip());
            m.set_time(getTimeRasp());
            sendMessage(m, motion::Message::SOCKET_PROTO_TOARRAY);
        
            break;
        }
        case motion::Message::REC_START:
        {
            struct arg_struct arguments;
            arguments.message = PROTO;
            
            runr = pthread_create(&thread_recognition, NULL, startRecognition, (void*) &arguments);
            if ( runr  != 0) {
                cerr << "Unable to create thread" << endl;
                cout << "startRecognition pthread_create failed." << endl;
            }
            pthread_join( thread_recognition, (void**) &runr);
            break;
        }
        case motion::Message::REC_STOP:
        {
            pthread_cancel(thread_recognition);
            break;
        }
    }
    google::protobuf::ShutdownProtobufLibrary();
    
}


// TCP client handling function
motion::Message::ActionType HandleTCPClient(TCPSocket *sock)
{
    
  motion::Message::ActionType value;
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
  
try
{
    
  // Send received string and receive again until the end of transmission
  char echoBuffer[motion::Message::SOCKET_BUFFER_SMALL_SIZE];
  int recvMsgSize;
  std::string message;
    
  while ((recvMsgSize = sock->recv(echoBuffer, motion::Message::SOCKET_BUFFER_SMALL_SIZE)) > 0)
  { // Zero means
     
      totalBytesReceived += recvMsgSize;     // Keep tally of total bytes
      echoBuffer[recvMsgSize] = '\0';        // Terminate the string!
      
      const string & data = echoBuffer;
      
      GOOGLE_PROTOBUF_VERIFY_VERSION;
      motion::Message ms;
      ms.ParseFromString(data);
      
      PROTO.Clear();
      PROTO = ms;
      
      cout <<       "Type Received : " << ms.type() << endl;
      
      value = ms.type();
      
      if (ms.has_time())
      {
            cout << "Time          : " << ms.time() << endl;
      }
      
      T_PROTO.set_serverip(ms.serverip());
      
      cout <<       "From Ip       : " << T_PROTO.serverip() << endl;

      motion::Message mr;
      
      string dataconnect;
      int echoStringLen;
      
      mr.set_type(motion::Message::RESPONSE_OK);
      mr.set_serverip(ms.serverip());
      mr.set_time(getTimeRasp());
      mr.SerializeToString(&dataconnect);
      char bts[dataconnect.length()];
      strcpy(bts, dataconnect.c_str());
      echoStringLen = sizeof(bts);
      
      //Respond socket.
      sock->send(bts, sizeof(bts));
      
      //Run Command.
      runCommand(value);

      google::protobuf::ShutdownProtobufLibrary();
      
  }
    
}
catch(SocketException &e)
{
        cout << "::error:: " << e.what() << endl;
        cerr << e.what() << endl;
}
  return value;
    
}

void * ThreadMain(void *clntSock)
{
    
    cout << "ThreadMain:: " << endl;

  // Guarantees that thread resources are deallocated upon return
  pthread_detach(pthread_self());
    
    pthread_mutex_lock(&echo_mutex);
    
        resutl_echo = HandleTCPClient((TCPSocket *) clntSock);
        //pthread_cond_signal(&echo_response);
        //cout << "RESULT IN THREAD:: " << resutl_echo << " from_ip: " << from_ip << endl;
    
    pthread_mutex_unlock(&echo_mutex);

    cout << "DELETING:: " << endl;
    
    delete (TCPSocket *) clntSock;
    //pthread_exit((void *) resutl);
    pthread_exit(NULL);
    return (void *) resutl_echo;
}

void * socketThread (void * args)
{
    
    // TCP
    unsigned short tcp_port = motion::Message::TCP_ECHO_PORT;
    void *status;
    
    pthread_mutex_init(&echo_mutex, NULL);
    pthread_cond_init(&echo_response, NULL);
    
    /*runl = pthread_create (&thread_wait_echo, NULL, watch_echo, NULL);
    if ( runl  != 0)
    {
        cerr << "Unable to create ThreadMain thread" << endl;
        cout << "ThreadM:::.in pthread_create failed." << endl;
        exit(1);
    }*/
    
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
            
            //cout << "EXECUTING!!!." << endl;
            //runCommand(resutl_echo);
            
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

void split(const string& s, char c, vector<string>& v)
{
   string::size_type i = 0;
   string::size_type j = s.find(c);

   while (j != string::npos)
   {
      v.push_back(s.substr(i, j-i));
      i = ++j;
      j = s.find(c, j);

      if (j == string::npos)
         v.push_back(s.substr(i, s.length()));
   }
}

void * broadcastsender ( void * args )
{
    
   std::string sech = getIpAddress();
   cout  <<  "IP: " << sech << endl;
   
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
   unsigned short destPort = motion::Message::UDP_PORT;
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

    motion::Message T_PROTO;
    T_PROTO.set_starttime(getTimeRasp());

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
   
    //Stream Socket Server.
   StreamListener * stream_listener = new StreamListener();
   stream_listener->startListening();

    pthread_join(    thread_socket,               (void**) &runs);
    
   cout << "LAST!!! = " << runs << endl;
    
   pthread_mutex_destroy(&tcpMutex);
   
   cout << "return 0" << endl;
   
   
   
   return 0;
  
    
}

            
        

