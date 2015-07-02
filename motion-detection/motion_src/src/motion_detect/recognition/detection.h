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

//Observer Recogition
extern bool is_recognizing;
extern int resutl_watch;
extern int resutl_watch_detected;
extern std::string image_file_recognized;
extern std::string getGlobalIntToString(int id);
extern cv::Mat picture;

extern motion::Message R_PROTO;

void * startRecognition(void * args);

//XML Region
std::string savePointsAsXMLString(std::vector<cv::Point2f> &contour);
std::vector<cv::Point2f> stringToVectorPoint2f(std::string storedcoord);
std::string processRegionString(std::string coordstring);


#endif	/* DETECTION_H */

