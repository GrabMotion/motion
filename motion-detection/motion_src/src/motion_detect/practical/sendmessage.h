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

void protoSocket(motion::Message m, bool array);
void * protoSend (void * arg);
