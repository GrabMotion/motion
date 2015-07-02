#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <iostream>
#include <pthread.h>

#include "../protobuffer/motion.pb.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace cv;

void* streamClient(void* arg);
void  quit(string msg, int retval);
int netcvc();

extern bool stop_capture;