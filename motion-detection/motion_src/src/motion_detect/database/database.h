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

extern bool to_bool(std::string const& s);

extern motion::Message PROTO, R_PROTO;

void updateCameraDB(int status, char * time, int camera);
static int callback(void *ptr, int argc, char* argv[], char* cols[] );
int db_cpuinfo();
std::vector<int> db_cams(std::vector<int> cams, std::string time);
bool loadStartQuery(std::string camera, std::string recname);

extern std::string getCurrentDayLabel();
extern std::string getCurrentMonthLabel();