/*
 * File:   startup.h
 * Author: jose
 *
 * Created on Dec 12, 2015, 7:48 PM
 */


#include <string>
#include <vector>

using namespace std;

int hardwareInfo();
int netWorkInfo();
int createDirectories();
int startUpParams(int argc, const char **argv);

//Network
std::string getIpAddress(std::string iface);

extern std::string starttime;

//Global 
extern std::string basepath;
extern std::string sourcepath;
extern std::vector<int> cams;
extern std::string local_ip;
extern std::string NETWORK_IP;
extern std::string public_ip;
extern std::string dumpinstancefolder;
extern void startMainRecognition();