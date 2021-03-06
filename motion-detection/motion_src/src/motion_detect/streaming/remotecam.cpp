#include <stdio.h>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include  <ctime>
#include <cstring>
#include <iostream>

#include "remotecam.hpp"
#include "send_data.hpp"


#include <iostream>
#include <vector>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace cv;
using namespace std;

const unsigned int STREAMING_VIDEO_PORT         = 5030;

int w = 640; //1280; //320;
int h = 480; //720; //240;
int jpegQuality = 200;

//int w = 320;
//int h = 240;
//int jpegQuality = 95;

pthread_t thread_streaming;
int runst;
string control_computer_ip;

struct stream_thread_args
{
    unsigned int port;
    int cam;
};
struct stream_thread_args StreamingStructThread;


void initCam(CvCapture* capture) {
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, w); //max. logitech 1280
    cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, h); //max. logitech 720

    double width = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH);
    double height = cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT);
    printf("cam w x h: %.0f x %.0f\n", width, height);
}

void* streamVideo(void * arg) {
    
    
    struct stream_thread_args *args = (struct stream_thread_args *) arg;
    
    std::string cip = control_computer_ip;
    char *control_remote_ip = new char[cip.length() + 1];
    std::strcpy(control_remote_ip, cip.c_str());

    
    std::cout << " Ready to stream image :: " << control_remote_ip << '\n';
    
    //paramInfo(argc, argv);
    
    CvCapture* capture = cvCreateCameraCapture(0);
    if (capture == NULL) {
        printf("No cam found.\n");
        return 0;
    }
    
    initCam(capture);
    
#ifndef linux
    cvNamedWindow("Cam");
#endif
    IplImage* image = cvQueryFrame(capture);
    printf("%d x %d (%d bit)\n", image->width, image->height, image->depth);
    
    Mat mat(h, w, CV_8UC3);
    mat = cvQueryFrame(capture);
    clock_t t0 = clock();
    printf("t0: %d\n", t0);
    int counter = 0;
    
    vector<uchar> buff;
    vector<int> param = vector<int>(2);
    param[0] = CV_IMWRITE_JPEG_QUALITY;
    param[1] = jpegQuality;
    
    initRemote();
    
    while (true) {
        counter++;
#ifndef linux
        cvShowImage("Cam", image);
#endif
        
        //mat = image;
                printf("writing jpg %d..", clock());
                imwrite("../motion_web/pics/screenshots/screen.jpg", mat);
                printf("%d\n", clock());
        
        printf("encode to jpg %d.. ", clock());
        cv::imencode(".jpg", mat, buff, param);
        printf("%d\n", clock());
        printf("jpg data: %d bytes\n", buff.size());
        
        //        for (int ii = 0; ii < 10; ii++) {
        //            printf("%d ", buff.at(ii)); //255 216 ...
        //        }
        //        printf("\n");
        
        //int receivedBytes = remote(buff);
        //if (receivedBytes <= 0) {
        //    closeSock();
        //    initRemote();
        // }
        
        int receivedBytes = remote(buff);
        
        if (receivedBytes <= 0)
        {
            
            closeSock();
            //initRemote();
            
            clock_t t1 = clock();
            printf("t1: %d\n", t1);
            printf("time %.1f fps\n", 1000.0 * counter / (t1 - t0));
            
            cvReleaseCapture(&capture);
            
            cerr << "WEBCAM CLOSED!!" << endl;
            
            break;
        }
        
        image = cvQueryFrame(capture);
        int c = cvWaitKey(1);
        if (c == 27) {
            break;
        }
    }
    
    clock_t t1 = clock();
    printf("t1: %d\n", t1);
    printf("time %.1f fps\n", 1000.0 * counter / (t1 - t0));
    
    cvReleaseCapture(&capture);

    return 0;
}

int connectStreaming(std::string from_ip)
{
    control_computer_ip = from_ip;
    // TCP Streaming
    StreamingStructThread.port = STREAMING_VIDEO_PORT;
    StreamingStructThread.cam = 0;
    
    // run the streaming client as a separate thread
    runst = pthread_create(&thread_streaming, NULL, streamVideo, &StreamingStructThread);
    if ( runst  != 0) {
        cerr << "Unable to create streamVideo thread" << endl;
        cout << "BroadcastSender pthread_create failed." << endl;
    }
    
    pthread_join(    thread_streaming,          (void**) &runst);
    
    pthread_cancel(thread_streaming);
    
    return 0;
    
    
}

