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

inline int detectMotion(const cv::Mat & motion,
                        cv::Mat & result, cv::Mat & result_cropped,
                        int x_start, int x_stop, int y_start, int y_stop,
                        int max_deviation,
                        cv::Scalar & color);

inline int detectMotionRegion(const cv::Mat & motion,
                              cv::Mat & result,
                              cv::Mat & result_cropped,
                              std::vector<cv::Point2f> & region,
                              int max_deviation,
                              cv::Scalar & color);


//Observer Recogition
extern bool is_recognizing;
extern bool stop_recognizing;
extern int resutl_watch;
extern int resutl_watch_detected;
extern std::string image_file_recognized;
extern std::string getGlobalIntToString(int id);
extern cv::Mat picture;

extern motion::Message R_PROTO;
extern motion::Message PROTO;

void * startRecognition(void * args);

//XML Region
std::vector<cv::Point2f> stringToVectorPoint2f(std::string storedcoord);
std::vector<cv::Point2f> processRegionString(std::string coordstring);


#endif	/* DETECTION_H */

