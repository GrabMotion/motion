/*
 * File:   post.h
 * Author: jose
 *
 * Created on Dec 12, 2015, 7:48 PM
 */


#include "../protobuffer/motion.pb.h"

using namespace std;

//Global
extern std::string WP_CLIENT_ID;
extern std::string SERVER_BASE_URL;
extern std::string WP_USER_ID;
extern std::string public_ip;
extern double t_post_day;

// WP USER AND PASSWORD
extern std::string WP_USER;
extern std::string WP_PASS;

std::string get_command_from_wp(std::string command);
vector<std::string> get_command_to_array_wp(std::string url);

int post_command_to_wp(bool update, std::string command, int db_local);
int post_command_to_wp(std::string command);
int post_media_command_to_wp(bool update, std::string command, int db_local);
std::string get_endpoint_from_wp(std::string endpoint);

vector<std::string> parsePost(std::string message, int numArgs, ...);

void postCameraStatus(int db_local_cam);
void postTerminalStatus();

motion::Message postRecognition(motion::Message m);

int insertUpdatePostInfo(bool update, std::string message, int db_local);

void locationPost(vector<std::string> locationinfo );

int instancePost(motion::Message::Instance pinstance, int db_instance_id, int post_parent);

void terminalPost(double timecount);
void locationPost(double timecount);
void cameraPost(double timecount);

void postTerminalStatus();
int dayPost(int db_recid, int db_dayid, std::string label, std::string xml);