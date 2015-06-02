/* 
 * File:   global.h
 * Author: jose
 *
 * Created on May 4, 2015, 4:07 AM
 */ 

#ifndef GLOBAL_H 
#define	GLOBAL_H 

#include <string>
#include <sstream>
#include <cstdlib>

using namespace std;

//Threads
//extern int runt, runb, runs, runr;

/// TCP Streaming
extern int             clientSock;
extern char*     	server_ip;
extern int       	server_port;
extern int       	server_camera;
extern std::string control_computer_ip;


#endif	/* GLOBAL_H */

