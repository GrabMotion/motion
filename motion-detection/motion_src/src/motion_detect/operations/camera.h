/*
 * File:   camera.h
 * Author: jose
 *
 * Created on Dec 12, 2015, 7:48 PM
 */

#include <vector> 
#include "../protobuffer/motion.pb.h"

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

//Cameras
std::vector<int> getCameras();
motion::Message::MotionCamera * takePictureToProto(int camera, motion::Message::MotionCamera * mcam);
std::string takeThumbnailFromCamera(int camera);

//Global 
extern cv::Mat picture;
extern std::string basepath;
extern std::string sourcepath;