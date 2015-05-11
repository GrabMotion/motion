/* 
 * File:   global.h
 * Author: jose
 *
 * Created on May 4, 2015, 4:07 AM
 */

#ifndef GLOBAL_H
#define	GLOBAL_H

const std::string NETWORK_IP = "192.168.1"; 

const unsigned int TCP_PORT                 = 5010;
const unsigned int UDP_PORT                 = 5020;
const unsigned int STREAMING_VIDEO_PORT     = 5030;

const unsigned int CONNECT              = 1000;
const unsigned int STOP_STREAMING       = 1002;
const unsigned int PAUSE_STREAMING      = 1003;

const unsigned int START_RECOGNITION        = 1004;
const unsigned int STOP_RECOGNITION         = 1005;


std::string getGlobalIntToString(int id){
    std::stringstream strm;
    strm << id;
    return strm.str();
}

int getGlobalStringToInt(std::string id){
   return atoi( id.c_str() );
}

#endif	/* GLOBAL_H */

