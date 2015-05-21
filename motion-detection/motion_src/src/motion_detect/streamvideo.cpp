#include "streamvideo.h"
#include "global.h"
#include <pthread.h>
#include <sstream>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstdlib>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <errno.h>


const unsigned int STREAMING_VIDEO_PORT     = 5030;

pthread_t thread_streaming;
int runs;
string control_computer_ip;

struct stream_thread_args
{
    unsigned int port;
    int cam;
};
struct stream_thread_args StreamingStructThread;

void* streamVideo(void * arg)
{
    
    struct stream_thread_args *args = (struct stream_thread_args *) arg;
    
    std::string cip = control_computer_ip;
    char *control_remote_ip = new char[cip.length() + 1];
    std::strcpy(control_remote_ip, cip.c_str());
    
    server_ip = control_remote_ip;
    server_port = args->port;
    server_camera = args->cam;
    
    //--------------------------------------------------------
    //networking stuff: socket, bind, listen
    //--------------------------------------------------------
    int                 localSocket,
    remoteSocket,
    port = server_port;
    
    struct  sockaddr_in localAddr,
    remoteAddr;
    
    
    int addrLen = sizeof(struct sockaddr_in);
    
    localSocket = socket(AF_INET , SOCK_STREAM , 0);
    if (localSocket == -1)
        perror("socket() call failed!!");
    
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = INADDR_ANY;
    localAddr.sin_port = htons( port );
    
    int optval = 1;
    if (setsockopt(localSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) < 0)
    {
        perror("Cannot set SO_REUSEADDR option");
        std::cerr << "on listen socket (%s)\n" << strerror(errno) << endl;
    }
    
    if( bind(localSocket,(struct sockaddr *)&localAddr , sizeof(localAddr)) < 0)
    {
        perror("Can't bind() socket");
        exit(1);
    }
    
    //Listening
    listen(localSocket , 1);
    
    std::cout <<  "Waiting for connections...\n"
    <<  "Server Port:" << port << std::endl;
    
    //accept connection from an incoming client
    remoteSocket = accept(localSocket, (struct sockaddr *)&remoteAddr, (socklen_t*)&addrLen);
    if (remoteSocket < 0) {
        perror("accept failed!");
        exit(1);
    }
    std::cout << "Connection accepted" << std::endl;
    
    
    //----------------------------------------------------------
    //OpenCV Code
    //----------------------------------------------------------
    int capDev = server_camera;
    
    
    cv::VideoCapture cap(capDev); // open the default camera
    cv::Mat img, imgGray;
    img = cv::Mat::zeros(480 , 640, CV_8UC1);
    //img = cv::Mat::zeros(720 , 1280, CV_8UC1);
    std::cout << "rows: " << img.rows << " cols: " << img.cols << endl;
    //img = cv::Mat::zeros(img.rows, img.cols, CV_8UC1);
    
    img = (img.reshape(0,1)); // to make it continuous
    
    if (!cap.isOpened()) {
        cap.release();
    }
    
    //make it continuous
    if (!img.isContinuous()) {
        img = img.clone();
    }
    
    int imgSize = img.total() * img.elemSize();
    int bytes = 0;
    int key;
    
    //make img continuos
    if ( ! img.isContinuous() ) {
        img = img.clone();
        imgGray = img.clone();
    }
    
    std::cout << "Image Size:" << imgSize << std::endl;
    
    int count = 0;
    
    //cv::namedWindow("CV Video Client",1);

    
    while (1) {
        
        /* get a frame from camera */
        cap >> img;
        
        if (img.empty())
        {
            std::cerr << "Something is wrong with the webcam img.empty()." << std::endl;
            break;
        }
        
        //if(imgGray.empty())
        //{
        //    std::cerr << "Something is wrong with the webcam imgGray.isempty()" << std::endl;
        //}
        
        //do video processing here
        flip(img, img, 1);
        cvtColor(img, imgGray, CV_BGR2GRAY);
        
        // string path = "test_" + count + ".jpg";
        // Save the frame into a file
        //imwrite("test.jpg", imgGray); // A JPG FILE IS BEING SAVED
        
        pthread_mutex_lock(&streamingMutex);
        
        //send processed image
        if ((bytes = send(remoteSocket, imgGray.data, imgSize, 0)) < 0)
        {
            std::cerr <<   "\n--> bytes = " << bytes << std::endl;
        }
        cout << "sending streaming data: " << bytes  << endl;
        
        count++;
        cout << "count: "  << count;
        
        
        if (count>10)  //key = cv::waitKey(10) >= 0)
        {
            
            cout << "Cierra Socket!"  << endl;
            
            if (localSocket){
                close(localSocket);
            }
            if (cap.isOpened()){
                cap.release();
            }
            if (!(img.empty())){
                (~img);
            }
            if (!(imgGray.empty())){
                (~imgGray);
            }
            pthread_mutex_destroy(&streamingMutex);
            
            cout << "break "  << endl;
            
            break;
        }
        
        
        //cv::imshow("CV Video Client", img);
        
        pthread_mutex_unlock(&streamingMutex);
        
        
        /* no, take a rest for a while */
        //usleep(1000);   //1000 Micro Sec
        
        //count>30
        
        
        //cvWaitKey(10);
        
        //if (key = cv::waitKey(10) >= 0)
        //break;
        
            }
    
    return 0;
    
}


int connectStreaming(std::string from_ip)
{
    control_computer_ip = from_ip;
    // TCP Streaming
    StreamingStructThread.port = STREAMING_VIDEO_PORT;
    StreamingStructThread.cam = 0;
    
    // run the streaming client as a separate thread
    runs = pthread_create(&thread_streaming, NULL, streamVideo, &StreamingStructThread);
    if ( runs  != 0) {
        cerr << "Unable to create streamVideo thread" << endl;
        cout << "BroadcastSender pthread_create failed." << endl;
    }
    
    pthread_join(    thread_streaming,          (void**) &runs);
    
    pthread_cancel(thread_streaming);
    
    return 0;
    
    
}