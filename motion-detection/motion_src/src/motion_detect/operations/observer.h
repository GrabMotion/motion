/*
 * File:   observer.h
 * Author: jose
 *
 * Created on Dec 12, 2015, 7:48 PM
 */

#include "../protobuffer/motion.pb.h"

void loadInstancesFromFile();
motion::Message getLocalPtoro();

//Global
extern std::string dumpinstancefolder;
extern pthread_mutex_t fileMutex;
extern std::vector<int> cams;