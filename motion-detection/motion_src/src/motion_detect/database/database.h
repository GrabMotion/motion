/*
 * File:   database.h
 * Author: jose
 *
 * Created on Julio 22, 2015, 11:23 AM
 */

#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <cstdlib>   
#include <sys/stat.h>
#include <dirent.h>
#include <algorithm>

#include <sqlite3.h>

#include <sys/time.h>


#include "../protobuffer/motion.pb.h"
#include "../b64/base64.h"

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

using namespace std;

void status();
void db_open();
void db_execute(const char *sql);
vector<vector<string> > db_select(const char *sql, int columns);
void db_close();

extern std::string basepath;
extern vector<string> splitString(string input, string delimiter);
extern std::string getGlobalIntToString(int id);
extern bool checkFile(const std::string &file);
extern std::string get_file_contents(std::string filename);
extern pthread_mutex_t databaseMutex;
extern std::string getXMLFilePathAndName(int cam, std::string recname, std::string currday, std::string name);

extern bool to_bool(std::string const& s);

extern motion::Message PROTO, R_PROTO;
extern std::string exec_command(char* cmd);

void updateRecStatus(int status, int camera, std::string recname);
static int callback(void *ptr, int argc, char* argv[], char* cols[] );
int db_cpuinfo();
std::vector<int> db_cams(std::vector<int> cams);
bool loadStartQuery(std::string camera, std::string recname);
vector<string> startIfNotRunningQuery(std::string camera, char * time);

extern std::string getCurrentDayLabel();
extern std::string getCurrentMonthLabel();

int insertMonthIntoDatabase(std::string str_month, int db_camera_id);
void updateRegionIntoDatabase(std::string rcoords, int recognitionid);
int insertRegionIntoDatabase(std::string rcoords);

int insertDayIntoDatabase(std::string str_day, int db_month_id);
int insertIntervalCrontabIntoDatabase(motion::Message::MotionCamera * pcamera, int db_camera_recognition_setupl_array);

//int updateIntervalCrontabIntoDatabase(motion::Message::MotionCamera * pcamera);

int insertIntoRecognitionSetup(motion::Message::MotionCamera * pcamera, int db_day_id, int db_camera_id, int db_coordnates_id, std::string xmlfilepath);

int insertIntoRelCameraRecognitionSetup(char * time_rasp, int db_recognitionsetup_id, int db_camera_id);

void updateRecognitionSetup(motion::Message::MotionCamera * pcamera, motion::Message::MotionDay * pday);
void insertIntoCameraMonth(char * time_rasp, int db_recognitionsetup_id, int db_camera_id );
void updateCameraMonth(char * time_rasp, int db_recognitionsetupid);

vector<string> getIntervalsByCamberaAndRec(std::string camera, std::string recname);

    