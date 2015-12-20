/*
 * File:   post.h
 * Author: jose
 *
 * Created on Dec 12, 2015, 7:48 PM
 */


#include "../protobuffer/motion.pb.h"

using namespace std;

//Global
extern std::string CLIENT_ID;
extern std::string SERVER_BASE_URL;
extern std::string public_ip;

double distanceEarth(double lat1d, double lon1d, double lat2d, double lon2d);

std::string get_command_from_wp(std::string command);

int post_command_to_wp(bool update, std::string command, int db_local);
int post_command_to_wp(std::string command);

vector<std::string> parsePost(std::string message, int numArgs, ...);

int postInstance(int db_instance_id, std::string content);

void postCameraStatus();
void postTerminalStatus();

motion::Message postRecognition(motion::Message m);

int insertUpdatePostInfo(bool update, std::string message, int db_local);

void locationPost(bool update, vector<std::string> locationinfo );

void terminalPost(double timecount);
void locationPost(double timecount);

void postTerminalStatus();