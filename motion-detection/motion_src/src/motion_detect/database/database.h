/*
 * File:   database.h
 * Author: jose
 *
 * Created on Julio 22, 2015, 11:23 AM
 */

//#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <cstdlib>   
#include <sys/stat.h>

#include <sqlite3.h>

using namespace std;

void db_open();
void db_execute(const char *sql);
vector<vector<string> > db_select(const char *sql, int columns);
void db_close();

extern vector<string> splitString(string input, string delimiter);
extern std::string getGlobalIntToString(int id);
extern bool checkFile(const std::string &file);
extern std::string get_file_contents(std::string filename);

//static int callback(void *NotUsed, int argc, char **argv, char **azColName);
static int callback(void *ptr, int argc, char* argv[], char* cols[] );
int db_cpuinfo();
std::vector<int> db_cams(std::vector<int> cams);


