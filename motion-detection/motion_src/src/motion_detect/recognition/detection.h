/*
 * File:   detection.h
 * Author: jose
 *
 * Created on May 9, 2015, 7:48 PM
 */

#ifndef DETECTION_H
#define	DETECTION_H

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

#include "../protobuffer/motion.pb.h"
#include "../b64/base64.h"

extern CvCapture * camera;
extern cv::VideoCapture * videocam;

//Observer Recogition
extern std::string basepath;
extern bool is_recognizing;
extern bool is_recognizing_flag;
extern int resutl_watch_detected;
extern std::string startrecognitiontime;
extern void directoryExistsOrCreate(const char* pzPath);
double getFramesPerSecond(CvCapture *capture);

extern std::string getXMLFilePathAndName(int cam, motion::Message m, std::string name);
extern std::string getGlobalIntToString(int id);
extern motion::Message PROTO, R_PROTO;
extern pthread_mutex_t protoMutex, databaseMutex;
extern std::string sourcepath;

extern std::string DIR_FORMAT;

void * startRecognition(void * args);

//XML Region
std::vector<cv::Point2f> stringToVectorPoint2f(std::string storedcoord);
std::vector<cv::Point2f> processRegionString(std::string coordstring);

void dumpInstance(int activecamnum,
        motion::Message::Instance * pinstance, 
        std::string DIR, 
        std::string XML_FILE, 
        std::string EXT_DATA, 
        time_t init_time, 
        time_t begin_time, 
        time_t end_time, 
        std::string instance, 
        std::string instancecode,
        std::string dumpfilename,
        std::string recname,
        std::string camera);

#endif	/* DETECTION_H */