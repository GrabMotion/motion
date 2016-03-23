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

//Global 
extern std::string basepath;
extern std::string sourcepath;
extern std::vector<int> cams;
extern std::string local_ip;
extern std::string NETWORK_IP;

extern std::string as;             // 0
extern std::string city;           // 1
extern std::string country;        // 2
extern std::string countryCode;    // 3
extern std::string isp;            // 4
extern std::string lat;            // 5
extern std::string lon;            // 6
extern std::string public_ip;      // 7
extern std::string region;         // 8
extern std::string regionName;     // 9
extern std::string time_zone;       // 10
extern std::string zip;            // 11

extern std::string dumpinstancefolder;
extern void startMainRecognition(int camnum);
extern std::string starttime;

void runJobsInterval(std::string timecompare, std::vector<pthread_t> threads_recognition);