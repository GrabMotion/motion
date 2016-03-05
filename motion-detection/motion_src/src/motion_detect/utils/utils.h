/*
 * File:   utils.h
 * Author: jose
 *
 * Created on Dec 12, 2015, 7:48 PM
 */


#include "../protobuffer/motion.pb.h"

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;

//Files and Directories
void set_file_permission(std::string file, std::string permission);
bool checkFile(const std::string &file);
std::string get_file_contents(std::string filename);
bool file_exists (const std::string& name);
void directoryExistsOrCreate(const char* pzPath);
std::string getXMLFilePathAndName(std::string sourcepath, int cam, std::string recname, std::string currday, std::string name);

//String Operations
vector<string> splitString(string input, string delimiter);
int getGlobalStringToInt(std::string id);
std::string getGlobalIntToString(int id);
std::string IntToString ( int number );
char * setMessageValueBody(int value, std::string body);
void split(const string& s, char c, vector<string>& v);
std::string escape(const std::string& input);
std::string fixedLength(int value, int digits);

//Date Time
std::string getCurrentDayLabel();
std::string getCurrentDayTitle();
std::string getCurrentMonthLabel();
char *getShortTimeRasp();
char * getTimeRasp();
char * setTimeToRaspBerry(struct tm tmremote, int timezone_adjust);

 
//Commands
std::string exec_command(char* cmd);

//Maths
int div_ceil(int numerator, int denominator);
double rad2deg(double rad);

//OpenCv
cv::Mat getImageWithTextByPath(std::string imagefilepath);
cv::Mat extractMat(string loadedmat);
cv::Mat drawRectFromCoordinate(std::string coords, cv::Mat mat, cv::Scalar color);

