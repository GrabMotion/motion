/*
 * File:   communications.h
 * Author: jose
 *
 * Created on Dec 12, 2015, 7:48 PM
 */

#include "../protobuffer/motion.pb.h"

#include "../practical/PracticalSocket.h" 

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

#define BUFLEN 2048
#define MSGS 5  /* number of messages to send */
#define SERVICE_PORT    21234   /* hard-coded port number */

using namespace std;
using namespace cv;

//Global
extern motion::Message PROTO;
extern motion::Message R_PROTO;
extern motion::Message T_PROTO;
extern int ruse;
extern pthread_mutex_t databaseMutex;
extern std::string basepath;
extern std::string sourcepath;
extern void startMainRecognition(int camnum);
extern bool stop_capture;
extern void startNotificationThread();

// WP USER AND PASSWORD
extern std::string WP_USER;
extern std::string WP_PASS;

//UDP
int UDPSend(motion::Message m);

motion::Message getRefreshProto(motion::Message m);
motion::Message runCommand(motion::Message m);


//Send to PC Thread
void * sendEcho(motion::Message m);


motion::Message serializeMediaToProto(motion::Message m, cv::Mat mat);

motion::Message::ActionType HandleTCPClient(TCPSocket *sock);
void totalsSocket();
