#include "socket/MatListener.h"

MatListener::MatListener(QObject *parent): QObject(parent){}

static cv::Mat     img;
static int     is_data_ready;
static int     listenSock, connectSock;
static int 	listenPort;

static pthread_mutex_t mutex;

/////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * This is the streaming server, run as separate thread
 */
void* MatListener::streamServer(void* arg)
{
        struct  sockaddr_in   serverAddr,  clientAddr;
        socklen_t             clientAddrLen = sizeof(clientAddr);

        /* make this thread cancellable using pthread_cancel() */
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

        if ((listenSock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
            quit("socket() failed.", 1);
        }

        int listenPort = 11111;

        serverAddr.sin_family = PF_INET;
        serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        serverAddr.sin_port = htons(listenPort);

        std::cout << "serverAddr: " << serverAddr.sin_addr.s_addr << " listenPort: " << listenPort << std::endl;


        if (::bind(listenSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
        {
            quit("bind() failed", 1);
        }

        if (listen(listenSock, 5) == -1)
        {
            quit("listen() failed.", 1);
        }

        int  imgSize = img.total()*img.elemSize();
        char sockData[imgSize];
        int  bytes=0;

        std::cout << "imgSize: " << imgSize << std::endl;

        /* start receiving images */
        while(1)
        {
            std::cout << "-->Waiting for TCP connection on port " << listenPort << " ...\n\n";

            /* accept a request from a client */
            if ((connectSock = accept(listenSock, (sockaddr*)&clientAddr, &clientAddrLen)) == -1)
            {
                    quit("accept() failed", 1);
            }
            else
            {
                std::cout << "-->Receiving image from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << "..." << endl;
            }

            memset(sockData, 0x0, sizeof(sockData));

            while(1)
            {
                if (is_data_ready==0)
                {
                    for (int i = 0; i < imgSize; i += bytes)
                    {
                        if ((bytes = recv(connectSock, sockData +i, imgSize  - i, 0)) == -1)
                        {
                            quit("recv failed", 1);
                        }
                        std::cout << "bytes received:" << bytes << std::endl;
                    }

                    /* convert the received data to OpenCV's Mat format, thread safe */
                    pthread_mutex_lock(&mutex);
                        for (int i = 0;  i < img.rows; i++)
                        {
                            for (int j = 0; j < img.cols; j++)
                            {
                                (img.row(i)).col(j) = (uchar)sockData[((img.cols)*i)+j];
                            }
                        }
                        is_data_ready = 1;
                        memset(sockData, 0x0, sizeof(sockData));
                    pthread_mutex_unlock(&mutex);

                 }
            }
        }

        /* have we terminated yet? */
        pthread_testcancel();

        /* no, take a rest for a while */
        usleep(1000);

}
/////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * This function provides a way to exit nicely from the system
 */
void MatListener::quit(std::string msg, int retval)
{
        if (retval == 0) {
                std::cout << (msg == "NULL" ? "" : msg) << "\n" <<endl;
        } else {
                std::cerr << (msg == "NULL" ? "" : msg) << "\n" <<endl;
        }

        if (listenSock){
                close(listenSock);
        }

        if (connectSock){
                close(connectSock);
        }

        if (!img.empty()){
                (img.release());
        }

        pthread_mutex_destroy(&mutex);
        //exit(retval);
}

void * MatListener::streamThread(void * args)
{
    QObject *parent = (QObject *)args;

   pthread_t thread_s;
   int width, height, key;

   width = 640;
   height = 480;

   img = cv::Mat::zeros( height,width, CV_8U); //CV_8UC1);

   /* run the streaming server as a separate thread */
   if (pthread_create(&thread_s, NULL, &MatListener::streamServer, NULL))
   {
           quit("pthread_create failed.", 1);
   }

   while(1)
   {
           pthread_mutex_lock(&mutex);
               if (is_data_ready)
               {
                       qRegisterMetaType<cv::Mat>("cv::Mat");
                       QMetaObject::invokeMethod(parent, "setremoteMat", Q_ARG(cv::Mat, img));
                       is_data_ready = 0;
               }
           pthread_mutex_unlock(&mutex);
   }

   if (pthread_cancel(thread_s))
   {
        quit("pthread_cancel failed.", 1);
   }

}

void MatListener::startListening(QObject *parent)
{
    mutex = PTHREAD_MUTEX_INITIALIZER;

    pthread_t thread_socket;
    int runs;
    //Socket
    runs = pthread_create(&thread_socket, NULL, &MatListener::streamThread, parent);
    if ( runs  != 0) {
        std::cerr << "Unable to create thread" << endl;
        std::cout << "BroadcastSender pthread_create failed." << endl;
    }
}
