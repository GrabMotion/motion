/*
 * File:   detection.h
 * Author: jose
 *
 * Created on May 9, 2015, 7:48 PM
 */

#ifndef DETECTION_H
#define	DETECTION_H

//Watch Recogition
//extern pthread_t thread_watch_amount;
//extern pthread_mutex_t watch_amount_mutex;
//extern pthread_cond_t watch_amount_detected;
//extern bool watch_received;
extern int resutl_watch;
extern int resutl_watch_detected;

//extern void * sendMessage (void * arg);
//extern void setMessage(char * message_send);
//extern std::string getGlobalIntToString(int id);

extern std::string image_file_recognized;

extern std::string getGlobalIntToString(int id);

void * startRecognition(void * args);

#endif	/* DETECTION_H */

