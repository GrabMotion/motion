#include "socket/MatListener.h"

#include <google/protobuf/stubs/common.h>

using namespace std;
using namespace cv;
using namespace google::protobuf::io;

#define RCVBUFSIZE 200000

MatListener::MatListener(QObject *parent): QObject(parent){}

struct mat_thread_args
{
    int * lp;
    QObject *parent;
};
struct mat_thread_args MatThread;

void * MatListener::socketHandler (void* lp)
{

  struct mat_thread_args *args = (struct mat_thread_args *) lp;
  int *csock = args->lp;
  QObject *parent = args->parent;

    int bytes=0;

    cv::Mat img = Mat::zeros( 720, 1280, CV_8UC3);
    int  imgSize = img.total()*img.elemSize();
    uchar sockData[imgSize];

    for (int i = 0; i < imgSize; i += bytes)
    {
        if ((bytes = recv(*csock, sockData +i, imgSize  - i, 0)) == -1)
        {
            //goto FINISH;
            cout << "recv failed" << endl;
        }
    }

    cout << "Mat received" << endl;

     // Assign pixel value to img
    int ptr=0;
    for (int i = 0;  i < img.rows; i++)
    {
        for (int j = 0; j < img.cols; j++)
        {
            img.at<cv::Vec3b>(i,j) = cv::Vec3b(sockData[ptr+ 0],sockData[ptr+1],sockData[ptr+2]);
            ptr=ptr+3;
        }
    }

    cout <<  "Mat built..." << endl;

    qRegisterMetaType<motion::Message>("cv::Mat");
    QMetaObject::invokeMethod(parent, "setremoteMat", Q_ARG(cv::Mat, img));


FINISH:
    close(*csock);

    /*while (1)
    {
        //Peek into the socket and get the packet size
        if((bytecount = recv(*csock, buffer,4, MSG_PEEK))== -1){
            fprintf(stderr, "Error receiving data %d\n", errno);
        }
        else if (bytecount == 0)
            break;

        cout<<"First read byte count is "<<bytecount<<endl;
    }*/

    return 0;
}


void * MatListener::socketThread(void * args)
{

    struct mat_thread_args *arg = (struct mat_thread_args *) args;
    //int *csock = arg->lp;
    QObject *parent = arg->parent;

    int host_port= 1101;
    struct sockaddr_in my_addr;

    int hsock;
    int * p_int ;
    int err;

    socklen_t addr_size = 0;
    int* csock;
    sockaddr_in sadr;
    pthread_t thread_id=0;

    hsock = socket(AF_INET, SOCK_STREAM, 0);
    if(hsock == -1){
            printf("Error initializing socket %d\n", errno);
    }

    p_int = (int*)malloc(sizeof(int));
    *p_int = 1;

    if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
            (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
            printf("Error setting options %d\n", errno);
            free(p_int);
    }
    free(p_int);

    my_addr.sin_family = AF_INET ;
    my_addr.sin_port = htons(host_port);

    memset(&(my_addr.sin_zero), 0, 8);
    my_addr.sin_addr.s_addr = INADDR_ANY ;

    if( ::bind( hsock, (sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
            fprintf(stderr,"Error binding to socket, make sure nothing else is listening on this port %d\n",errno);
    }
    if(listen( hsock, 10) == -1 ){
            fprintf(stderr, "Error listening %d\n",errno);
    }

    while(true)
    {
            printf("waiting for a connection\n");
            csock = (int*)malloc(sizeof(int));
            MatThread.lp = csock;
            MatThread.parent = parent;
            if((*csock = accept( hsock, (sockaddr*)&sadr, &addr_size))!= -1)
            {
                    printf("---------------------\nReceived connection from %s\n",inet_ntoa(sadr.sin_addr));
                    pthread_create(&thread_id, 0, &MatListener::socketHandler, &MatThread); //(void*) csock );
                    pthread_detach(thread_id);
            }
            else
            {
                    fprintf(stderr, "Error accepting %d\n", errno);
            }
    }

}

void MatListener::startListening(QObject *parent)
{
    pthread_t thread_socket;
    int runs;
    MatThread.parent = parent;
    //Socket
    runs = pthread_create(&thread_socket, NULL, &MatListener::socketThread, &MatThread);
    if ( runs  != 0) {
        cerr << "Unable to create thread" << endl;
        cout << "BroadcastSender pthread_create failed." << endl;
    }
}
