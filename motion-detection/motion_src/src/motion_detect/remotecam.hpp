#include <iostream>
#include <string>
#include <cstring>

using namespace std;

int connectStreaming(std::string from_ip);
void initCam(CvCapture* capture);
void* streamVideo(void * arg);


int w = 320;
int h = 240;
int jpegQuality = 95;

pthread_t thread_streaming;
int runs;
string control_computer_ip;

struct stream_thread_args
{
    unsigned int port;
    int cam;
};
struct stream_thread_args StreamingStructThread;


