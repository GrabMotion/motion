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
#include <signal.h>

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
#include <dirent.h>
#include <sys/stat.h>

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "practical/PracticalSocket.h" 
//#include "practical/sendmessage.h"
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
pthread_t thread_broadcast, thread_echo, thread_socket,
thread_message, thread_recognition, thread_wait_echo,
thread_observer, thread_send_echo;

//Threads
int runt, runb, runs, runr, runl, runm, runw, runss, runo, ruse;

/// TCP Streaming
int         clientSock;
char*     	server_ip;
int       	server_port;
int       	server_camera;

//Threads
pthread_cond_t echo_response;
pthread_mutex_t protoMutex, fileMutex;
bool echo_received;
motion::Message::ActionType resutl_echo;
std::string result_message;
std::string from_ip;

//TCP
std::string local_ip;
string NETWORK_IP;
vector<std::string> msg_split_vector;
motion::Message::ActionType value_response;
int count_sent__split=0;
int count_vector_size=0;
std::string msg;
int div_ceil(int numerator, int denominator);
std::string IntToString ( int number );
std::string fixedLength(int value, int digits);
int inttype;

//Send
void * sendEcho(motion::Message m);

//Protobuffer
motion::Message PROTO;
motion::Message R_PROTO;
motion::Message T_PROTO;
motion::Message takePictureToProto(motion::Message);
std::string starttime;
motion::Message getLocalPtoro(motion::Message m);

//Recognition
bool stop_capture;
bool is_recognizing;
cv::Mat picture;
bool stop_recognizing;
int number_of_changes;
int resutl_watch_detected;
std::string startrecognitiontime;
string DIR_FORMAT           = "%d%h%Y"; // 1Jan1970

//UDP
int udpsend(motion::Message m);

#define RCVBUFSIZE 1024

//const unsigned int RCVBUFSIZE = 100000; //4096; //32;     // Size of receive buffer
const int MAXRCVSTRING = 4096;          // Longest string to receive

std::string getGlobalIntToString(int id);

int getGlobalStringToInt(std::string id);
char * setMessageValueBody(int value, std::string body);

inline bool file_exist (const std::string& name) {
    ifstream f(name.c_str());
    if (f.good()) {
        f.close();
        return true;
    } else {
        f.close();
        return false;
    }
}

struct arg_struct
{
    motion::Message message;
};
void * startRecognition(void * arg);
void * startObserver(void * arg);
motion::Message runCommand(motion::Message m);
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

std::string get_file_contents(std::string filename)
{
    std::ifstream in(filename.c_str(), std::ios::in | std::ios::binary);
    if (in)
    {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return(contents);
    }
    throw(errno);
}


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

char *getShortTimeRasp()
{
    struct timeval tr;
    struct tm* ptmr;
    char time_rasp[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_rasp, sizeof (time_rasp), "%H:%M:%S", ptmr);
    return time_rasp;
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

motion::Message getImageToProto(motion::Message m)
{
    
    cout << "+++++++++++LOADING IMAGE TO PROTO++++++++++++++" << endl;
    
    Mat mat = imread(m.imagefilepath());
    
    //Shared mat
    picture = mat;

    GOOGLE_PROTOBUF_VERIFY_VERSION;
    
    m.set_type(motion::Message::TAKE_PICTURE);
    m.set_serverip(PROTO.serverip());
    m.set_time(getTimeRasp());
    
    // We will need to also serialize the width, height, type and size of the matrix
    int width_s     = mat.cols;
    int height_s    = mat.rows;
    int type_s      = mat.type();
    int size_s      = mat.total() * mat.elemSize();
    
    // Initialize a stringstream and write the data
    int size_init = m.ByteSize();
    
    // Write the whole image data
    std::stringstream ss;
    ss.write((char*)    (&width_s),     sizeof(int));
    ss.write((char*)    (&height_s),    sizeof(int));
    ss.write((char*)    (&type_s),      sizeof(int));
    ss.write((char*)    (&size_s),      sizeof(int));
    ss.write((char*)     mat.data,      size_s);
    
    std::string ssstring = ss.str();
    
    std::string oriencoded = base64_encode(reinterpret_cast<const unsigned char*>(ssstring.c_str()), ssstring.length());
    
    //Store into proto
    m.set_data(oriencoded.c_str());
    
    google::protobuf::ShutdownProtobufLibrary();
    
    return m;
}


motion::Message takePictureToProto(motion::Message m)
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
    
    Mat mat(h, w, CV_8U); //CV_8U); // CV_8UC3);
    mat = cvQueryFrame(capture);
    cvtColor(mat, mat, CV_RGB2GRAY);
    
    //imwrite("MAT.jpg", mat);
    
    //Shared mat
    picture = mat;
    
    cout << "+++++++++++CREATING PROTO++++++++++++++" << endl;
    
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    
    m.set_type(motion::Message::TAKE_PICTURE);
    m.set_serverip(PROTO.serverip());
    m.set_time(getTimeRasp());
    
    // We will need to also serialize the width, height, type and size of the matrix
    int width_s     = mat.cols;
    int height_s    = mat.rows;
    int type_s      = mat.type();
    int size_s      = mat.total() * mat.elemSize();
    
    // Initialize a stringstream and write the data
    int size_init = m.ByteSize();
    
    // Write the whole image data
    std::stringstream ss;
    ss.write((char*)    (&width_s),     sizeof(int));
    ss.write((char*)    (&height_s),    sizeof(int));
    ss.write((char*)    (&type_s),      sizeof(int));
    ss.write((char*)    (&size_s),      sizeof(int));
    ss.write((char*)     mat.data,      size_s);
    
    std::string ssstring = ss.str();
    
    std::string oriencoded = base64_encode(reinterpret_cast<const unsigned char*>(ssstring.c_str()), ssstring.length());
    
    //Store into proto
    m.set_data(oriencoded.c_str());
    
    struct timeval tp;
    gettimeofday(&tp, NULL);
    int ms = tp.tv_sec * 1000 + tp.tv_usec / 1000;
    
    cout << "activemat::::: " << ms << endl;
    
    T_PROTO.add_matfile(ms);
    T_PROTO.set_activemat(ms);
    
    m.add_matfile(ms);
    m.set_activemat(ms);

    //Write base64 to file for checking.
    std::string basefile = "data/mat/" + IntToString(ms);
    std::ofstream out;
    out.open (basefile.c_str());
    out << m.data() << "\n";
    out.close();
    
    cout << "ByteSize: " << size_init <<  endl;
    
    cvReleaseCapture(&capture);
    
    google::protobuf::ShutdownProtobufLibrary();
    
    return m;
}

void * sendProto (void * arg)
{
    
    struct arg_struct *args = (struct arg_struct *) arg;
    motion::Message me      = args->message;
    string servAddress      = T_PROTO.serverip();
    
    google::protobuf::uint32 pport = motion::Message::TCP_ECHO_PORT;
    google::protobuf::uint32 buffersize = motion::Message::SOCKET_BUFFER_NANO_SIZE + 28;
    
    cout << "serverIp: " << servAddress << endl;
    cout << "serverPort: " << pport << endl;
    
    int echoServPort = pport;
    char echoBuffer[buffersize];

    //string data;
    me.set_time(getTimeRasp());
   
    //Initialize objects to serialize.
    int size = me.ByteSize();
    char datasend[size];
    string datastr;
    me.SerializeToArray(&datasend, size);
    google::protobuf::ShutdownProtobufLibrary();
    
    std:string encoded_proto = base64_encode(reinterpret_cast<const unsigned char*>(datasend),sizeof(datasend));
    
    cout << "encoded_proto: " << encoded_proto << endl;
    
    char * message = new char[encoded_proto.size() + 1];
    std::copy(encoded_proto.begin(), encoded_proto.end(), message);
    message[encoded_proto.size()] = '\0'; // don't forget the terminating 0
    
    try
    {
        TCPSocket sock(servAddress, echoServPort);
        sock.send(message, sizeof(message));
        
    } catch(SocketException &e)
    {
        cerr << e.what() << endl;
    }
}

void * sendEcho(motion::Message m)
{
    struct arg_struct arguments;
    arguments.message = m;
    
    ruse = pthread_create(&thread_send_echo, NULL, sendProto, (void*) &arguments);
    
    if ( ruse  != 0)
    {
        cerr << "Unable to create thread" << endl;
        cout << "startRecognition pthread_create failed." << endl;
    }
}

void * startObserver(void * arg)
{
    
    cout << "startObserver" << endl;
    
    while (true)
    {
            if (is_recognizing)
            {
                
                std::cout << "number_of_changes = " << resutl_watch_detected << std::endl;
                
                if (resutl_watch_detected>0)
                {
                    
                    motion::Message m;
                    m.set_recognizing(true);
                    pthread_mutex_lock(&protoMutex);
                    m = R_PROTO;
                    pthread_mutex_unlock(&protoMutex);
                    
                    //Initialize objects to serialize.
                    int size = m.ByteSize();
                    
                    //cout << "Proto size   : " << size << endl;
                    
                    char dataresponse[size];
                    string datastr;
                    
                    m.SerializeToArray(&dataresponse, size);
                    
                    std:string encoded_proto = base64_encode(reinterpret_cast<const unsigned char*>(dataresponse),sizeof(dataresponse));
                    
                    //Write base64 to backup with mutex.
                    std::string basefile = "data/data/localproto.txt";
                    std::ofstream out;
                    pthread_mutex_lock(&fileMutex);
                    out.open (basefile.c_str());
                    out << encoded_proto << "\n";
                    out.close();
                    pthread_mutex_unlock(&fileMutex);
                    
                    sleep(1);
                }
            }
        
        sleep(1);
    }
    cout << "stopObserver" << endl;
    
}

std::string IntToString ( int number )
{
    std::ostringstream oss;
    oss<< number;
    return oss.str();
}

int div_ceil(int numerator, int denominator)
{
    std::div_t res = std::div(numerator, denominator);
    return res.rem ? (res.quot + 1) : res.quot;
}

motion::Message getLocalPtoro(motion::Message m )
{
    std::string protofile = "data/data/localproto.txt";
    
    if (file_exist(protofile))
    {
        
        m.Clear();
        std::string backfile = protofile;
        pthread_mutex_lock(&fileMutex);
        string loaded = get_file_contents(backfile);
        pthread_mutex_unlock(&fileMutex);
        std::string oridecoded = base64_decode(loaded);
        m.ParseFromArray(oridecoded.c_str(), oridecoded.size());
        m.set_time(getTimeRasp());
    }
    return m;

}

motion::Message runCommand(motion::Message m)
{
    GOOGLE_PROTOBUF_VERIFY_VERSION;
    
    cout << "runCommand:: " << m.type() << endl;
    
    switch (m.type())
    {
        case motion::Message::ENGAGE:
        {
            cout << "motion::Message::ENGAGE" << endl;
            m = getLocalPtoro(m);
            m.set_type(motion::Message::ENGAGE);
            m.set_cameras(T_PROTO.cameras());
            m.set_starttime(T_PROTO.starttime());
            m.set_activemat(T_PROTO.activemat());
            
        }
        break;
        case motion::Message::REFRESH:
        {
            cout << "motion::Message::REFRESH" << endl;
            m = getLocalPtoro(m);
            m.set_type(motion::Message::REFRESH);

        }
        break;
        case motion::Message::GET_XML:
        {
            cout << "motion::Message::GET_XML" << endl;
            std::string xml_file;
            if (m.has_xmlfilename())
            {
               xml_file  = m.xmlfilename();
            }
            cout << "xml_file : " << xml_file << endl;
            string DIR        = "../../src/motion_web/pics";
            std::string XML_FILE  =  "xml/<import>session.xml";
            std::string xml_path = DIR + "/" + xml_file + "/" + XML_FILE;
            string xml_loaded = get_file_contents(xml_path);
            std:string encoded_xml = base64_encode(reinterpret_cast<const unsigned char*>(xml_loaded.c_str()),xml_loaded.length());
            m.set_type(motion::Message::GET_XML);
            m.set_data(encoded_xml.c_str());
        }
        break;
        case motion::Message::GET_IMAGE:
        {
            cout << "motion::Message::GET_IMAGE" << endl;
            m = getImageToProto(m);
            m.set_type(motion::Message::GET_IMAGE);
            
        }
        break;
        case motion::Message::DISSCONNECT:
        {
            cout << "motion::Message::DISSCONNECT" << endl;
            
        }
        break;
        case motion::Message::REC_START:
        {
            cout << "motion::Message::REC_START" << endl;
            runr = pthread_create(&thread_recognition, NULL, startRecognition, NULL);
            if ( runr  != 0) {
                cerr << "Unable to create thread" << endl;
            }
        }
        break;
        case motion::Message::REC_STOP:
        {
            cout << "motion::Message::REC_STOP" << endl;
            stop_recognizing = true;
        }
        break;
        case motion::Message::TAKE_PICTURE:
        {
            
            cout << "motion::Message::TAKE_PICTURE" << endl;
            m = takePictureToProto(m);
            cout << "activemat:  " << m.activemat() << endl;
            
        }
        break;
        case motion::Message::STRM_START:
        {
            cout << "motion::Message::STRM_START" << endl;
            netcvc();
        }
        break;
        case motion::Message::STRM_STOP:
        {
            cout << "motion::Message::STRM_STOP" << endl;
            stop_capture = true;
            
        }
        break;
        case motion::Message::GET_TIME:
        {
            cout << "motion::Message::GET_TIME" << endl;
            m.set_type(motion::Message::GET_TIME);
            m.set_serverip(PROTO.serverip());
            m.set_time(getTimeRasp());
        }
        break;
        case motion::Message::SET_TIME:
        {
            cout << "motion::Message::SET_TIME" << endl;
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

        }
        break;
    }

    return m;
}

std::string fixedLength(int value, int digits)
{
    unsigned int uvalue = value;
    if (value < 0) {
        uvalue = -uvalue;
    }
    std::string result;
    while (digits-- > 0) {
        result += ('0' + uvalue % 10);
        uvalue /= 10;
    }
    if (value < 0) {
        result += '-';
    }
    std::reverse(result.begin(), result.end());
    return result;
}

void totalsSocket()
{
    //cout << "count_vector_size : " << count_vector_size << " count_sent__split : " << count_sent__split << endl;
    if ( count_sent__split < (count_vector_size -1))
    {
        count_sent__split++;
    } else
    {
        count_sent__split = 0;
    }
    //cout << " count_sent__split : " << count_sent__split << endl;
}

// TCP client handling function
motion::Message::ActionType HandleTCPClient(TCPSocket *sock)
{
    
    motion::Message::ActionType value;
    int bytesReceived = 0;              // Bytes read on each recv()
    int totalBytesReceived = 0;         // Total bytes read
  
try
{
    
  // Send received string and receive again until the end of transmission
  char echoBuffer[motion::Message::SOCKET_BUFFER_NANO_SIZE + 28];
  int recvMsgSize;
  std::string message;
    

    while ((recvMsgSize = sock->recv(echoBuffer, motion::Message::SOCKET_BUFFER_NANO_SIZE + 28)) > 0)
    {

      totalBytesReceived += recvMsgSize;     // Keep tally of total bytes
      echoBuffer[recvMsgSize] = '\0';        // Terminate the string!
        
      GOOGLE_PROTOBUF_VERIFY_VERSION;
        
      stringstream sss;
      sss << echoBuffer;
      string strproto = sss.str();
        
      std::string strdecoded;
      strdecoded.clear();
      strdecoded = base64_decode(strproto);
      
      motion::Message ms;
      ms.ParseFromArray(strdecoded.c_str(), strdecoded.size());
        
      PROTO.Clear();
      PROTO = ms;
      value = ms.type();
      T_PROTO.set_serverip(ms.serverip());
    
      if ( ms.type()==motion::Message::RESPONSE_OK || ms.type()==motion::Message::RESPONSE_END )
      {
          cout << "response : " <<  ms.type() << endl;
          count_sent__split = 0;
          return ms.type();
      }
      else if (ms.type()==motion::Message::RESPONSE_NEXT)
      {
          
          string header =
          "PROSTA" +
            fixedLength(count_vector_size,4)  + "::" +
            fixedLength(count_sent__split,4)  + "::" +
            IntToString(inttype)              +
          "PROSTO";
          
          msg = header + msg_split_vector.at(count_sent__split);

          cout << "header 2 : " << header << endl;
          
          totalsSocket();
          
          sock->send(msg.c_str(), msg.size());
        
          return ms.type();
      }
      
      //Elaborate response.
      motion::Message m;
      
      //Run Command.
      m = runCommand(ms);
        
      cout << "Serializing proto response." << endl;
      
      //Initialize objects to serialize.
      int size = m.ByteSize();
    
      cout << "Proto size   : " << size << endl;
    
      char dataresponse[size];
      string datastr;

      m.SerializeToArray(&dataresponse, size);
     
      cout << "Encoding." << endl;
        
      std:string encoded_proto = base64_encode(reinterpret_cast<const unsigned char*>(dataresponse),sizeof(dataresponse));
        
        string header;
        inttype = ms.type();
        
      if ( size > motion::Message::SOCKET_BUFFER_NANO_SIZE )
      {
          google::protobuf::uint32 chunck_size = motion::Message::SOCKET_BUFFER_NANO_SIZE;
          
          for (unsigned i = 0; i < encoded_proto.length(); i += chunck_size)
          {
              msg_split_vector.push_back(encoded_proto.substr(i, chunck_size));
          }
          
          count_vector_size = msg_split_vector.size();
          
          header =
          "PROSTA" +
            fixedLength(count_vector_size, 4)  + "::" +
            fixedLength(count_sent__split, 4)  + "::" +
            IntToString(inttype)               +
          "PROSTO";
          msg = header + msg_split_vector.at(count_sent__split);

          cout << "header 1 : " << header << endl;
          
      }
      else
      {
          
          header =
          "PROSTA"
          "0001"
          "::"
          "0000"
          "::";
          header += IntToString(inttype);
          header += "PROSTO";
          
          msg = header + encoded_proto;
          
          count_vector_size = 0;
          
          cout << "header 0 : " << header << endl;
          
      }
      
        ms.Clear();
        google::protobuf::ShutdownProtobufLibrary();
    
        totalsSocket();
        
        sock->send(msg.c_str(), msg.size());
        
        return ms.type();
        
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
    // Guarantees that thread resources are deallocated upon return
    pthread_detach(pthread_self());
    resutl_echo = HandleTCPClient((TCPSocket *) clntSock);
    delete (TCPSocket *) clntSock;
    pthread_exit(NULL);
    return (void *) resutl_echo;
}

void * socketThread (void * args)
{
    // TCP
    unsigned short tcp_port = motion::Message::TCP_ECHO_PORT;
    void *status;
    pthread_cond_init(&echo_response, NULL);
    try
    {
        TCPServerSocket servSock(tcp_port);
        for (;;)
        {
            TCPSocket *clntSock = servSock.accept();
            runt =  pthread_create(&thread_echo, NULL, ThreadMain, (void *) clntSock);
            if ( runt  != 0)
            {
                cerr << "Unable to create ThreadMain thread" << endl;
                exit(1);
            }
            pthread_join(    thread_echo,               (void**) &runt);
        }
    } catch (SocketException &e) {
        cerr << e.what() << endl;
        exit(1);
    }
}

std::string getIpAddress ()
{
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
    int countud;
    for (;;) {
        sock.sendTo(sendString, strlen(sendString), destAddress, destPort);
        //cout << "UPD Send: " << countud << " " << getShortTimeRasp() << endl;
        countud++;
        sleep(3);
    }
    delete [] sendString;
  
  } catch (SocketException &e) {
    cerr << e.what() << endl;
    cout  <<  "Error: " << cerr << endl;
    exit(1);
  }
}

std::vector<int> getCameras()
{
    std::vector<int> camsv;
    CvCapture* temp_camera;
    int maxTested = 3;
    for (int i = 0; i < maxTested; i++){
        temp_camera = cvCreateCameraCapture(i);
        if (temp_camera!=NULL)
        {
            camsv.push_back(i);
            cvReleaseCapture(&temp_camera);
        }
    }
    return camsv;
}

// Check if the directory exists, if not create it
// This function will create a new directory if the image is the first
// image taken for a specific day
void directoryExistsOrCreate(const char* pzPath)
{
    DIR *pDir;
    // directory doesn't exists -> create it
    if ( pzPath == NULL || (pDir = opendir (pzPath)) == NULL)
        mkdir(pzPath, 0777);
    // if directory exists we opened it and we
    // have to close the directory again.
    else if(pDir != NULL)
        (void) closedir (pDir);
}


int main (int argc, char * const argv[])
{
    
    struct timeval tr;
    struct tm* ptmr;
    char time_rasp[40];
    gettimeofday (&tr, NULL);
    ptmr = localtime (&tr.tv_sec);
    strftime (time_rasp, sizeof (time_rasp), "%Y-%m-%d %H:%M:%S %z", ptmr);
    cout <<  ":::start time:::: " << time_rasp << endl;

    //Rasp Variables.
    std::vector<int> cams = getCameras();
    stringstream ss;
    copy( cams.begin(), cams.end(), ostream_iterator<int>(ss, " "));
    std::string cameras = ss.str();
    cameras = cameras.substr(0, cameras.length()-1);
    
    T_PROTO.set_cameras(cameras);
    T_PROTO.set_starttime(time_rasp);
    starttime = time_rasp;
    
    std::string basedatafile = "data";
    directoryExistsOrCreate(basedatafile.c_str());
    std::string secdatafile = "data/data";
    directoryExistsOrCreate(secdatafile.c_str());
    std::string matdatafile = "data/mat";
    directoryExistsOrCreate(matdatafile.c_str());
    
    cout << "Start Time:: " << starttime << endl;

    pthread_mutex_init(&protoMutex, 0);
    pthread_mutex_init(&fileMutex, 0);
    
    // UDP
   runb = pthread_create(&thread_broadcast, NULL, broadcastsender, NULL);
   if ( runb  != 0) {
       cerr << "Unable to create thread" << endl;
   }
   
    //Socket
    runs = pthread_create(&thread_socket, NULL, socketThread, NULL);
    if ( runs  != 0) {
        cerr << "Unable to create thread" << endl;
    }
    
    runo = pthread_create(&thread_observer, NULL, startObserver, NULL);
    if ( runo  != 0) {
        cerr << "Unable to create thread" << endl;
    }
    
    //Stream Socket Server.
    //StreamListener * stream_listener = new StreamListener();
    //stream_listener->startListening();

    pthread_join(    thread_broadcast,  (void**) &runb);
    pthread_join(    thread_socket,     (void**) &runs);
    pthread_join(    thread_observer,   (void**) &runr);
    
    cout << "THREAD TERMINATED!!!!!!!!!!!!!!!!!!!!! = " << runs << endl;
    return 0;
}

            
        


