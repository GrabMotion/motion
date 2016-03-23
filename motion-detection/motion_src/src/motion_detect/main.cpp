/* 
 * File:   main.cpp
 * Author: jose
 *
 * Created on April 19, 2015, 11:23 PM
 */

#include <iostream> h 
#include <string>  

#include <stdio.h>
#include <sys/types.h>
#include <pthread.h>
#include <signal.h>

#include <sys/time.h>
#include <vector>

#include "practical/PracticalSocket.h" 
#include "recognition/detection.h"
#include "protobuffer/motion.pb.h"

//#include "protobuffer/server.pb.h"
//#include "socket/streamlistener.h"
//#include "socket/netcvc.h"

#include "database/database.h"
#include "http/post.h"
#include "operations/communications.h"
#include "operations/observer.h"
#include "operations/startup.h"


#include <parse.h>

using namespace std;

// GLOBAL VARIABLES //

//Parse
ParseClient client;

//Paths
std::string basepath;
std::string sourcepath;

//Protobuffer
motion::Message PROTO;
motion::Message R_PROTO;
motion::Message T_PROTO;
std::string starttime;

//TCP
std::string local_ip;
std::string NETWORK_IP;

std::vector<int> cams;
bool observer_running;

std::string dumpinstancefolder;

std::string CLIENT_ID;
std::string SERVER_BASE_URL;

//LOCATION
std::string as;             // 0
std::string city;           // 1
std::string country;        // 2
std::string countryCode;    // 3
std::string isp;            // 4
std::string lat;            // 5
std::string lon;            // 6
std::string public_ip;      // 7
std::string region;         // 8
std::string regionName;     // 9
std::string time_zone;      // 10
std::string zip;            // 11

//xml
std::string XML_FILE = "<import>session";

/// TCP Streaming
int         clientSock;
char*     	server_ip;
int       	server_port;
int       	server_camera;

//WP User and Password
std::string WP_USER;
std::string WP_PASS;

//Threads
int runt, runb, runs, runr, runl, runm, runw, runss, runo, ruse;
int numRecognitionThreads = 4;
std::vector<pthread_t> threads_recognition(4);
std::vector<int> threads_recognizing_pids(4);

// Threading
pthread_t thread_broadcast, thread_echo, thread_socket;
bool observer_thread_running = false;

//Threads
pthread_cond_t echo_response;
pthread_mutex_t protoMutex, fileMutex, databaseMutex;
bool echo_received;
motion::Message::ActionType resutl_echo;
std::string from_ip;

// END GLOBAL VARIABLES //

//Recognition
bool stop_capture;
cv::Mat picture;
int number_of_changes;
int resutl_watch_detected;
std::string startrecognitiontime;
string DIR_FORMAT           = "%d%h%Y"; // 1Jan1970
void startMainRecognition(int camnum);
void * startRecognition(void * arg);

//Run Forever
double t_post_terminal      = 60 * 20; //60 * 24; 
double t_post_location   = 60 * 60 * 12; 
double t_post_camera     = 60 * 60 * 6; 

double t_load_instance   = 10; //60 * 24;      


int main_loop_counter;


#define RCVBUFSIZE 1024
//const unsigned int RCVBUFSIZE = 100000; //4096; //32;     // Size of receive buffer
const int MAXRCVSTRING = 4096;          // Longest string to receive

void * ThreadMain(void *clntSock);

void startMainRecognition(int camnum)
{
    runr = pthread_create(&threads_recognition[camnum], NULL, startRecognition, NULL);
    if ( runr  != 0) 
    {
        cerr << "Unable to create thread" << endl;
    }  
    
    int count = 0;
    bool recognizing;
    while (!recognizing)
    {
        recognizing = isRecognizing();
        cout << "recognizing: " << recognizing  << endl;
        count ++;
        if (count==1000)
        {
           break; 
        }
    }
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
            if (runt  != 0)
            {
                cerr << "Unable to create ThreadMain thread" << endl;
                exit(1);
            }
            pthread_join(    thread_echo,               (void**) &runt);
        }
    } catch (SocketException &e) 
    {
        cerr << e.what() << endl;
        exit(1);
    }
}

void * broadcastsender ( void * args )
{
    
   std::string destAddress = NETWORK_IP + ".255";
   unsigned short destPort = motion::Message::UDP_PORT;
   char *sendString = new char[local_ip.length() + 1];
   std::strcpy(sendString, local_ip.c_str());
   char recvString[MAXRCVSTRING + 1];
  
   try 
   {
    UDPSocket sock;
    int countud;
    for (;;) 
    {
        sock.sendTo(sendString, strlen(sendString), destAddress, destPort);
        cout << "UPD Send: " << countud << " " << endl;
        countud++;
        sleep(5);
    }
    delete [] sendString;
  
  } catch (SocketException &e) {
    cerr << e.what() << endl;
    cout  <<  "Error: " << cerr << endl;
    exit(1);
  }
}

void startThreads()
{
    pthread_mutex_init(&protoMutex, 0);
    pthread_mutex_init(&fileMutex, 0);
    pthread_mutex_init(&databaseMutex, 0);
    
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
}

int main (int argc, char * const av[])
{
    
	cout << "CV_MAJOR_VERSION: " << CV_MAJOR_VERSION << endl;
	
    //createBlobTable();
	
    const char **argv = (const char **) av; 
    cout << "argv[0]: " << argv[0] << endl;
    if (argc==2)
            cout << "argv[1]: " << argv[1] << endl;
    if (argc==3)
        cout << "argv[2]: " << argv[2] << endl;
    
    FILE *output;
    if(!(output = popen("/sbin/route -n | grep -c '^0\\.0\\.0\\.0'","r"))){
        return 1;
    }
    unsigned int i;
    fscanf(output,"%u",&i);
    if(i==0){
        cout << "There is no internet connection! \n";
        exit(0);
    }
    pclose(output);
   
    std::string runparam = argv[0];
    if (runparam=="./motion_detect_raspberry") {
        basepath = "";
        sourcepath = "../../src/"; 
    }
    else if (runparam.find("/home/pi/motion/motion-detection/motion_src/src/motion_detect/motion_detect_raspberry") != std::string::npos ){
        basepath    = "src/motion_detect/";
        sourcepath  = "src/";
    }
    
    cout << "+++++++++++++++++++++++++++++++++++++++++++" << endl;
    cout << "runparam: "    << runparam     << endl;
    cout << "+++++++++++++++++++++++++++++++++++++++++++" << endl;
    cout << "basepath: "    << basepath     << endl;
    cout << "sourcepath: "  << sourcepath   << endl;
    cout << "+++++++++++++++++++++++++++++++++++++++++++" << endl;
    
    //Hardware Info 
    hardwareInfo();
    //Network Info
    netWorkInfo();
    //Directories
    createDirectories();
    //StartUp Params
    startUpParams(argc, argv);
    //Start Threads
    startThreads();
    
    //Parse
    client = parseInitialize("fsLv65faQqwqhliCGF7oGqcT8MxPDFjmcxIuonGw", "T3PK1u0NQ36eZm91jM0TslCREDj8LBeKzGCsrudE");
    //parseSendRequest(client, "POST", "/1/classes/TestObject", "{\"foo\":\"bar\"}"  
        
    //Main Loop
    for (;;)
    {
        vector<std::string> user_info = getUserInfo();

        cout << "main loop counter: " << main_loop_counter << endl;

        if (user_info.size()>0)
        {

            //WP USER PASS
            WP_USER = user_info.at(1);
            WP_PASS = user_info.at(2);

            //Post Location
            locationPost(t_post_location);

            //Post Camera
            cameraPost(t_post_camera); 

            time_t now;
            time(&now); 

            CLIENT_ID       = user_info.at(5);
            SERVER_BASE_URL = user_info.at(3);

            //Post Terminal
            terminalPost(t_post_terminal);

            sleep(5);

            //Scan local instances and store
            loadInstancesFromFile();

            sleep(5);

            struct timeval tr;
            struct tm* ptmr;
            char curr_time[40];
            gettimeofday (&tr, NULL);
            ptmr = localtime (&tr.tv_sec);
            strftime (curr_time, sizeof (curr_time), "%H:%M:%S", ptmr);
            std::stringstream timecompare;
            timecompare << curr_time;

            //Run Job if in Interval
            runJobsInterval(timecompare.str(), threads_recognition);

            main_loop_counter ++;
            
        }
        
        sleep(10);
    }
     
    //Stream Socket Server.
    //StreamListener * stream_listener = new StreamListener();
    //stream_listener->startListening();
 
    return 0;
    
}

            
        


