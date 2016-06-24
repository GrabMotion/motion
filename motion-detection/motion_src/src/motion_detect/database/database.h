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

#include <assert.h>
#include <malloc.h>

#include <sys/time.h>

#include "../tinyxml/tinyxml.h"
#include "../tinyxml/tinystr.h"

#include "../protobuffer/motion.pb.h"
#include "../b64/base64.h"

 #include "../utils/utils.h"

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

using namespace std;

//Global
extern std::string sourcepath;
extern std::string XML_FILE;

void status();
void db_open();
void db_execute(const char *sql);
vector<vector<string> > db_select(const char *sql, int columns);
void db_close();
void createBlobTable();

//blob
static int createBlobTable(sqlite3 *db);
static int writeBlob(const char *zKey, const unsigned char *zBlob, int nBlob);
int readBlob(const char *zKey, unsigned char **pzBlob, int *pnBlob);
static void freeBlob(unsigned char *zBlob);
static void databaseError();

extern std::string basepath;
extern vector<string> splitString(string input, string delimiter);
extern std::string getGlobalIntToString(int id);
extern bool checkFile(const std::string &file);
extern std::string get_file_contents(std::string filename);
extern pthread_mutex_t protoMutex, databaseMutex; 

extern char genRandom();
extern bool to_bool(std::string const& s);

extern motion::Message PROTO, R_PROTO;
extern std::string exec_command(char* cmd);

void updateRecStatusByRecName(int status, std::string recname);
void updateRecStatusByCamera(int status, int camnum);

int getRecRunningByName(std::string name);
static int callback(void *ptr, int argc, char* argv[], char* cols[] );
int db_cpuinfo();
std::vector<int> db_cams(std::vector<int> cams);
bool loadStartQuery(std::string camera, std::string recname);
vector<string> checkJobRunningQuery(std::string camera, char * time);

extern std::string getCurrentDayLabel();
extern std::string getCurrentMonthLabel();

int insertMonthIntoDatabase(std::string str_month, int db_camera_id);
void updateRegionIntoDatabase(std::string rcoords, int recognitionid);
int insertRegionIntoDatabase(std::string rcoords);
int insertUserIntoDatabase(motion::Message::MotionUser * muser); //int clientnumber, std::string clientname, std::string base);

int insertDayIntoDatabase(std::string str_day, int db_month_id);
int insertIntervalIntoDatabase(motion::Message::MotionCamera * pcamera, motion::Message::MotionRec * prec);

int insertIntoRecognitionSetup(motion::Message::MotionRec * prec, 
        int db_day_id, 
        int db_camera_id, 
        int db_coordnates_id, 
        std::string xmlfilepath,
        std::string created);

int insertIntoRelCameraRecognitionSetup(char * time_rasp, int db_recognitionsetup_id, int db_camera_id);

void updateRecognitionSetup(int db_idcamera, motion::Message::MotionRec * prec, motion::Message::MotionDay * pday);
void insertIntoCameraMonth(char * time_rasp, int db_recognitionsetup_id, int db_camera_id );
void updateCameraMonth(char * time_rasp, int db_recognitionsetupid);

vector<string> getIntervalsByCamberaAndRec(std::string camera, std::string recname);

vector<string> getMaxImageByPath(google::protobuf::int32 imageid);
vector<string> getImageByPath(std::string path);
//int insertTracking(int db_instance_id, std::string maximagepath, int db_srv_idmedia, int db_srv_idpost);
vector<int> getNotProccessedInstance();

void insertIntoLocation(vector<std::string> location);
//database
int insertIntoIMage(const motion::Message::Image & img);
void insertIntoCrop(const motion::Message::Crop & crop, int db_image_id);
int insertIntoVideo(motion::Message::Video dvideo);
int insertIntoInstance(std::string number, motion::Message::Instance * pinstance, const char* init_time, const char* end_time, int db_video_id, vector<int> images);
//Dump xml
void build_xml(const char * xmlPath);
void writeXMLInstance (std::string XMLFILE, std::string time_start, std::string time_end, std::string instance, std::string instancecode);

void setActiveCam(int activecam);

vector<std::string> getIpInfo();
vector<std::string> getTerminalInfo();
vector<std::string> getLocationInfo();

int insertIntoPosts(std::string id, std::string date, std::string modified, std::string slug, std::string type, std::string link, std::string api_link, std::string featured_image, std::string post_parent, int db_local);
void updateIntoPost (std::string id, std::string date, std::string modified);

vector<std::string> getTrackPostByType(std::string type);

vector<std::string> getTrackPostByTypeAndIdLocal(std::string type, int db_local);
vector<std::string> getTrackPostByTypeAndIdRemote(std::string type, int db_remote);
vector<std::string> getTrackPostByTypeAndIdParent(std::string type, int post_parent);

vector<std::string> getMatInfoFromId(int db_idmat);
int getPostByIdAndType(int db_idpost);
vector<vector<string> > getTrackPosts(std::string type);
vector<vector<string> > getTrackPostChilds(int id);

time_t getLastPostTimeByType(std::string type);
time_t getLastPostTimeByTypeAndLocal(std::string type, int db_local);

vector<std::string> getUserInfo();


void insertUpdateStatus(std::string uptime, vector<int> camsarray, int db_terminal_id);

motion::Message saveRecognition(motion::Message m);


motion::Message::MotionCamera * getMonthByCameraIdMonthAndDate(
                motion::Message::MotionCamera * mcam,   
                std::string camid, 
                std::string month, 
                std::string day,
                std::string rec);

void updateRecognition(motion::Message m);

vector<vector<string> > getCamerasFromDB();

std::string getDayCreatedById(int dayid);

std::string getIntervalByIdRecognitionSetupId(int db_idrec);

int insertIntoNetwork(std::string public_ip, std::string local_ip, std::string resutl_mac );

std::vector<string> getCameraByCameraDbId(int db_camera);

vector<string> getInstallationUIID();
vector<string> getParseInfoForPush();

int getUserIdByWpUserName(std::string username);

vector<string> getTerminalSerial();

void insertIntoProcess(motion::Message::MotionUser * muser, int db_recogniton_setup);