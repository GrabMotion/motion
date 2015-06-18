#include <iostream>
#include <string>
#include <cstring>


#include "../protobuffer/motion.pb.h"

#include "../practical/PracticalSocket.h"

using namespace std;

extern motion::Message send_proto;
extern motion::Message receive_proto;

extern std::string from_ip;
extern std::string control_computer_ip;
extern int runm;
extern pthread_t thread_message;

void setMessage(motion::Message m, bool array);
void * sendMessage (void * arg);
