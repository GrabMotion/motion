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

//Observer Recogition
extern bool is_recognizing;
extern bool stop_recognizing;
extern int resutl_watch_detected;
extern std::string startrecognitiontime;

extern std::string getGlobalIntToString(int id);
extern cv::Mat picture;
extern motion::Message PROTO, R_PROTO;
extern pthread_mutex_t protoMutex;;

extern char *getTimeRasp();
char *getShortTimeRasp();

void * startRecognition(void * args);

//XML Region
std::vector<cv::Point2f> stringToVectorPoint2f(std::string storedcoord);
std::vector<cv::Point2f> processRegionString(std::string coordstring);

#endif	/* DETECTION_H */

