#include <iostream>
#include <string>
#include <cstring>


#include "../protobuffer/motion.pb.h"

#include "../practical/PracticalSocket.h"

using namespace std;

extern motion::Message T_PROTO;

extern std::string from_ip;
extern std::string control_computer_ip;
extern int runm;
extern pthread_t thread_message;

void sendMessage(motion::Message m, motion::Message::SocketType type);

extern char *getTimeRasp();
