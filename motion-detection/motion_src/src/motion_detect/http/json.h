/*
 * File:   json.h
 * Author: jose
 *
 * Created on Dec 12, 2015, 7:48 PM
 */

#include <json/json.h>
#include <string>
#include <vector>

using namespace std;

std::string parse_json( json_object* j );
std::string parse_json_array( json_object* j, char* key );

//Host Info
vector<std::string> parse_and_store_ipinfo_io(const char *cstr);

//std::string getMediaIdFromJSON(std::string message);