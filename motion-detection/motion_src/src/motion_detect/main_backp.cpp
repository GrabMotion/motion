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

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "practical/PracticalSocket.h" 
#include "practical/sendmessage.h"

#include "recognition/detection.h"

#include "protobuffer/motion.pb.h"

#include "b64/encode.h"
#include "b64/decode.h"

using namespace std;
using namespace cv;

void * ThreadMain(void *clntSock);
int HandleTCPClient(TCPSocket *sock);
void RunUICommand(int result, string from_ip);
void * watch_amount (void * t);

// Threading
pthread_mutex_t tcpMutex, streamingMutex;
pthread_t thread_broadcast, thread_echo, thread_socket,
thread_message, thread_recognition, thread_wait_echo, thread_screenshot;

//Threads
int runt, runb, runs, runr, runl, runm, runw, runss;

/// TCP Streaming
int         clientSock;
char*     	server_ip;
int       	server_port;
int       	server_camera;

//Threads
pthread_cond_t echo_response;
pthread_mutex_t echo_mutex;
bool echo_received;
int resutl_echo;
std::string result_message;
std::string from_ip;

//TCP
std::string local_ip;
string NETWORK_IP;

//Protobuffer
motion::Message send_proto;
motion::Message receive_proto;
void restoreProto(bool isarray, char ** data, std::string file);

#define MAXDATASIZE 1000

#define RCVBUFSIZE 500000

//const unsigned int RCVBUFSIZE = 100000; //4096; //32;     // Size of receive buffer
const int MAXRCVSTRING = 4096;          // Longest string to receive

std::string getGlobalIntToString(int id);

int getGlobalStringToInt(std::string id);
char * setMessageValueBody(int value, std::string body);

int resutl_watch_detected;
std::string image_file_recognized;

struct recognition_thread_args
{
    bool writeImages;
    bool writeCrops;
};
struct recognition_thread_args RecognitionahaStructThread;
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

void sendEcho(std::string serv, char * echo, short port )
{
    
    std::string servAddress  = serv;            //argv[1]; // First arg: server address
    //char *echoString    = echo;                 //argv[2];   // Second arg: string to echo
    int echoStringLen   = strlen(echo);   // Determine input length
    unsigned short echoServPort = port; //(argc == 4) ? atoi(argv[3]) : 7;
    
    cout << "servAddress: " << servAddress << endl;
    cout << "echoServPort: " << echoServPort << endl;
    
    try
    {
        // Establish connection with the echo server
        TCPSocket sock(servAddress, echoServPort);
        
        // Send the string to the echo server
        sock.send(echo, echoStringLen);
        
        char echoBuffer[RCVBUFSIZE + 1];    // Buffer for echo string + \0
        int bytesReceived = 0;              // Bytes read on each recv()
        int totalBytesReceived = 0;         // Total bytes read
        // Receive the same string back from the server
        cout << "Received: ";               // Setup to print the echoed string
        
        GOOGLE_PROTOBUF_VERIFY_VERSION;
        
        motion::Message mr;
        
        while (totalBytesReceived < echoStringLen) {
            // Receive up to the buffer size bytes from the sender
            if ((bytesReceived = (sock.recv(echoBuffer, RCVBUFSIZE))) <= 0) {
                cerr << "Unable to read";
                exit(1);
            }
            
            totalBytesReceived += bytesReceived;     // Keep tally of total bytes
            echoBuffer[bytesReceived] = '\0';        // Terminate the string!
            cout << echoBuffer;                      // Print the echo buffer
            
            const string & data = echoBuffer;
            
            mr.ParseFromString(data);
            
            receive_proto.Clear();
            receive_proto = mr;
            
            cout << "Type Received: " << mr.type() << endl;


        }
        cout << endl;
        
        // Destructor closes the socket
        
    } catch(SocketException &e) {
        cout << "Error!: " << e.what() << endl;
        cerr << e.what() << endl;
        exit(1);
    }

}

void restoreProto(bool isarray, char ** data, std::string file)
{
    cout << "+++++++++++RESTORING PROTO++++++++++++++" << endl;
    
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    
    motion::Message mm;
    
    cout << "::1::" << endl;
    
    if (isarray)
    {
        mm.ParseFromArray(&data, sizeof(data));
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
    cv::Mat m_d = Mat(mm.height(), mm.width(), mm.typemat(), data_r).clone();
    
    imwrite(file /*"image_2.jpg"*/, m_d);
    
    google::protobuf::ShutdownProtobufLibrary();
    delete data_r;
    delete data;

}

void* streamCast(void * arg)
{
    
    struct screenshot_thread_args *args = (struct screenshot_thread_args *) arg;
    
    motion::Message message = args->msg;
    
    cout << "CAPTURING !!!!!!!!!!!!!" << endl;
    
    CvCapture* capture = cvCreateCameraCapture(0);
    if (capture == NULL)
    {
        std::cout << "No cam found." << std::endl;
        return 0;
    }
    
    int w = 1280; //640; //1280; //320;
    int h = 720; //480; //720; //240;
    
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, w); //max. logitech 1280
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, h); //max. logitech 720
    
    //double width = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
    //double height = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
    
    IplImage* img=0;
    img = cvQueryFrame( capture );
    cvSaveImage("IplImage.JPG",img);
    
    Mat mat(h, w, CV_8U); //CV_8U); // CV_8UC3);
    mat = cvQueryFrame(capture);
    cvtColor(mat, mat, CV_RGB2GRAY);
    imwrite("MAT.jpg", mat);
    
    cout << "+++++++++++CREATING PROTO++++++++++++++" << endl;
    
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    
    motion::Message me;
    me.set_type(motion::Message::SET_MAT);
    
    me.set_serverip(message.serverip());
    me.set_time(getTimeRasp());
    
    int rows = mat.rows;
    int cols = mat.cols;
    
    me.set_rows(rows);
    me.set_cols(cols);
    
    cv::Size s = mat.size();
    me.set_height(h);
    me.set_width(w);
    
    // We will need to also serialize the width, height, type and size of the matrix
    int width_s     = mat.cols;
    int height_s    = mat.rows;
    int type_s      = mat.type();
    size_t size_s   = mat.total() * mat.elemSize();
    
    me.set_size(size_s);
    me.set_typemat(type_s);
    
    // Initialize a stringstream and write the data
    std::stringstream ss;
    int size_init = me.ByteSize();
    
    cout << "width      : " << me.width() << endl;
    cout << "rows       : " << rows << endl;
    cout << "height     : " << me.height() << endl;
    cout << "cols       : " << cols << endl;
    cout << "Mat type   : " << me.type() << endl;
    cout << "Mat size   : " << size_s << endl;
    cout << "Proto size : " << size_init << endl;
    
    // Write the whole image data
    ss.write((char*) mat.data, size_s);
    
    cout << "::1::" << endl;
    
    // Base64 encode the stringstream
    base64::encoder E;
    std::stringstream encoded;
    E.encode(ss, encoded);

        cout << "::2::" << endl;
    
    //Store into proto
    std::string encoded_str = encoded.str();
    me.set_data(encoded_str); //mat_data);

    bool array = true;
    char * data_init[size_init];
    
        cout << "::3::" << endl;
    
    if (array)
    {
        me.SerializeToArray(&data_init, size_init);
        
    } else
    {
        //string data;
        //me.SerializeToString(&data);
        //char bts[data.length()];
        //strcpy(bts, data.c_str());
        //sendEcho(me.serverip(), bts, motion::Message::TCP_MSG_PORT);
    }
    
        cout << "::4::" << endl;
    
    cvReleaseCapture(&capture);
    google::protobuf::ShutdownProtobufLibrary();
    delete data_init;
    
    cout << "::5::" << endl;

    
    //Restore Proto
    restoreProto(true, data_init, "MAT_LOCAL.jpg");
    
    cout << "::6::" << endl;


    //Send proto to the server
    setMessage(me, true);
    //sendEcho(me.serverip(), data_init, motion::Message::TCP_MSG_PORT);
    
    return 0;
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

void runCommand(int value)
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    motion::Message m;
    
    string dataconnect;
    int echoStringLen;
    
    switch (value)
    {
            
        /*case motion::Message::CONNECT:
            
            //connectStreaming(from_ip);
            
            m.set_type(motion::Message::CONNECT);
            m.set_serverip(receive_proto.serverip());
            m.set_time(getTimeRasp());
            m.SerializeToString(&dataconnect);
            char bts[dataconnect.length()];
            strcpy(bts, dataconnect.c_str());
            echoStringLen = sizeof(bts); //strlen(bts);
            
            //sock->send(bts, sizeof(bts));
            
            cout << " ::SII:: " << endl;
            
            setMessage(m, false);
            
            break;*/
            
        case motion::Message::GET_MAT:
            
            screenshot(receive_proto);
            
            break;
            
        case motion::Message::GET_TIME:
            
            struct timeval tv;
            struct tm* ptm;
            char time_string[40];
            
            gettimeofday (&tv, NULL);
            ptm = localtime (&tv.tv_sec);
            strftime (time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S %z", ptm);
            
            cout << "time_string TIME " << time_string << endl;
            
            m.set_type(motion::Message::GET_TIME);
            m.set_time(time_string);
            
            setMessage(m, false);
            
            break;
            
        case motion::Message::SET_TIME:
            
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
            
            m.set_type(motion::Message::TIME_SET);
            m.set_payload(time_string);
            
            setMessage(m, false);
            
            break;
            
        case motion::Message::START_RECOGNITION:
            
            RecognitionahaStructThread.writeCrops = true;
            RecognitionahaStructThread.writeImages = true;
            
            runr = pthread_create(&thread_recognition, NULL, startRecognition, &RecognitionahaStructThread);
            if ( runr  != 0) {
                cerr << "Unable to create thread" << endl;
                cout << "startRecognition pthread_create failed." << endl;
            }
            
            pthread_join( thread_recognition, (void**) &runr);
            
            break;
            
        case motion::Message::STOP_RECOGNITION:
            
            pthread_cancel(thread_recognition);
            
            break;
    }
    
    cout << " ::SIIIiII:: " << endl;
    
    cout << " ::ShutdownProtobufLibrary:: " << endl;
    
    google::protobuf::ShutdownProtobufLibrary();
    
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
  char echoBuffer[MAXDATASIZE];
  int recvMsgSize;
  std::string message;
    
  while ((recvMsgSize = sock->recv(echoBuffer, RCVBUFSIZE)) > 0)
  { // Zero means
     
      totalBytesReceived += recvMsgSize;     // Keep tally of total bytes
      echoBuffer[recvMsgSize] = '\0';        // Terminate the string!
      

      const string & data = echoBuffer;
      
      GOOGLE_PROTOBUF_VERIFY_VERSION;
      motion::Message ms;
      ms.ParseFromString(data);
      
      receive_proto.Clear();
      receive_proto = ms;
      
      cout << "Type Received: " << ms.type() << endl;
      
      value = ms.type();
      
      if (ms.has_time())
      {
            cout << "VALUE!! " << value << " TIME!! " << ms.time() << endl;
      }
      
      motion::Message mr;
      
      string dataconnect;
      int echoStringLen;
      
      mr.set_type(ms.type());
      mr.set_serverip(ms.serverip());
      mr.set_time(getTimeRasp());
      mr.SerializeToString(&dataconnect);
      char bts[dataconnect.length()];
      strcpy(bts, dataconnect.c_str());
      echoStringLen = sizeof(bts);
      
      sock->send(bts, sizeof(bts));
      
      google::protobuf::ShutdownProtobufLibrary();
      
      
      /*char * message_send;
      std::string command = "";
      char *response;
      char* init;
      char buf[MAXDATASIZE];
      string data;*/
      
      //sock->send(echoBuffer, recvMsgSize);
      
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

/*void * watch_echo (void * t)
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
}*/

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
            
            cout << "EXECUTING!!!." << endl;
            runCommand(resutl_echo);
            
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
    
    /*motion::Message mf;
    mf.set_type(motion::Message::SET_MAT);
    mf.set_serverip("192.168.1.35");
    mf.set_time("timess");
    screenshot(mf);
    google::protobuf::ShutdownProtobufLibrary();*/
    

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

            
        
