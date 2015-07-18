#include <iostream>
#include <string>
#include <cstring>

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"

//#include "opencv2/calib3d/calib3d.hpp"
//#include "opencv2/imgproc/imgproc_c.h"

#include "../practical/PracticalSocket.h"
#include "../practical/sendmessage.h"

#include "../protobuffer/motion.pb.h"

using namespace cv;
using namespace std;

extern pthread_t thread_streaming;
extern int runst;

struct screenshot_thread_args
{
    motion::Message message;
};


void* streamCast(void * arg) {
    
    struct screenshot_thread_args *args = (struct screenshot_thread_args *) arg;
    
    motion::Message message = args->message;
    
    CvCapture* capture = cvCreateCameraCapture(0);
    if (capture == NULL) {
        std::cout << "No cam found." << std::endl;
        return 0;
    }
    
    int w = 640; //1280; //320;
    int h = 480; //720; //240;
    
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, w); //max. logitech 1280
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, h); //max. logitech 720
    
    double width = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
    double height = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
    
    Mat mat(h, w, CV_8UC3);
    mat = cvQueryFrame(capture);
    
    cvtColor(mat, mat, CV_RGB2GRAY);

    motion::Message m;
    m.set_serverip(m.serverip());
    m.set_width(640);
    m.set_height(480);
    std::string sData(reinterpret_cast<char*>(mat.data));
    m.set_data(sData);

    setMessage(m, true);
    
    cvReleaseCapture(&capture);

    google::protobuf::ShutdownProtobufLibrary();
    
    return 0;
}

int screenshot(motion::Message m)
{

    // TCP Streaming
    struct screenshot_thread_args ScreenshorStructThread;
    ScreenshorStructThread.message = m;
    
    // run the streaming client as a separate thread
    runst = pthread_create(&thread_streaming, NULL, streamCast, &ScreenshorStructThread);
    if ( runst  != 0) {
        cerr << "Unable to create streamVideo thread" << endl;
        cout << "BroadcastSender pthread_create failed." << endl;
    }
    
    pthread_join(    thread_streaming,          (void**) &runst);
    
    pthread_cancel(thread_streaming);
    
    return 0;
    
    
}

