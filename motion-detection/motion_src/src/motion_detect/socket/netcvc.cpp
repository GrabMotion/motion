#include "../socket/netcvc.h"

VideoCapture    capture;
Mat             img0, img1, img2;
int             is_data_ready = 1;
int             clientSockMat;
char*     	server_ip_mat;
int       	server_port_mat;
extern bool stop_capture;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


int netcvc()
{
        pthread_t   thread_c;
        int         key;

        capture.open(0);

        if (!capture.isOpened()) {
                quit("\n--> cvCapture failed", 1);
        }

        server_ip_mat   = "192.168.1.37";
        server_port_mat = 11111;

        capture >> img0;
        img1 = Mat::zeros(img0.rows, img0.cols ,CV_8U);

        // run the streaming client as a separate thread 
        if (pthread_create(&thread_c, NULL, streamClient, NULL)) {
                quit("\n--> pthread_create failed.", 1);
        }

        //cout << "\n--> Press 'q' to quit. \n\n" << endl;

        /* print the width and height of the frame, needed by the client */
        cout << "\n--> Transferring  (" << img0.cols << "x" << img0.rows << ")  images to the:  " << server_ip_mat << ":" << server_port_mat << endl;

        while(1)
        {
                /* get a frame from camera */
                capture >> img0;
                if (img0.empty()) break;

                pthread_mutex_lock(&mutex);

                        flip(img0, img0, 1);
                        cvtColor(img0, img1, CV_BGR2GRAY);

                        is_data_ready = 1;
                        std::cout << "is_data_ready: " << is_data_ready;

                pthread_mutex_unlock(&mutex);
            
                if (stop_capture)
                    break;
            
        }

        /* user has pressed 'q', terminate the streaming client */
        if (pthread_cancel(thread_c)) {
                quit("\n--> pthread_cancel failed.", 1);
        }
    
        quit("\n--> Close Stream.", 1);
    
return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * This is the streaming client, run as separate thread
 */
void* streamClient(void* arg)
{
        struct  sockaddr_in serverAddr;
	socklen_t           serverAddrLen = sizeof(serverAddr);

        /* make this thread cancellable using pthread_cancel() */
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

        if ((clientSockMat = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
            quit("\n--> socket() failed.", 1);
        }

        cout << "server_ip_mat: " << server_ip_mat << " server_port_mat: " << server_port_mat << endl;

        serverAddr.sin_family = PF_INET;
        serverAddr.sin_addr.s_addr = inet_addr(server_ip_mat);
        serverAddr.sin_port = htons(server_port_mat);

        if (connect(clientSockMat, (sockaddr*)&serverAddr, serverAddrLen) < 0) {
                quit("\n--> connect() failed.", 1);
        }

        int  imgSize = img1.total()*img1.elemSize();
        std::cout << "imgSize: " << imgSize << std::endl;
        int  bytes=0;
        img2 = (img1.reshape(0,1)); // to make it continuous

        /* start sending images */
        while(1)
        {
                /* send the grayscaled frame, thread safe */
                if (is_data_ready)
                {
                        pthread_mutex_lock(&mutex);
                                if ((bytes = send(clientSockMat, img2.data, imgSize, 0)) < 0)
                                {
                                    cerr << "\n--> bytes = " << bytes << endl;
                                	quit("\n--> send() failed", 1);
                                }
                                std::cout << "bytes sent: " << bytes << std::endl;
                                is_data_ready = 0;
                        pthread_mutex_unlock(&mutex);
                        memset(&serverAddr, 0x0, serverAddrLen);
                }
            
                /* if something went wrong, restart the connection */
                /*if (bytes != imgSize) {
                        cerr << "\n-->  Connection closed (bytes != imgSize)" << endl;
                        close(clientSockMat);

                        if (connect(clientSockMat, (sockaddr*) &serverAddr, serverAddrLen) == -1) {
                                quit("\n--> connect() failed", 1);
                        }
                }*/
            
                /* have we terminated yet? */
                pthread_testcancel();
            
                /* no, take a rest for a while */
                usleep(1000);   //1000 Micro Sec
            
        }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * this function provides a way to exit nicely from the system
 */
void quit(string msg, int retval)
{
        if (retval == 0) {
                cout << (msg == "NULL" ? "" : msg) << "\n" << endl;
        } else {
                cerr << (msg == "NULL" ? "" : msg) << "\n" << endl;
        }
        if (clientSockMat){
                close(clientSockMat);
        }
        if (capture.isOpened()){
                capture.release();
        }
        if (!(img0.empty())){
                (~img0);
        }
        if (!(img1.empty())){
                (~img1);
        }
        if (!(img2.empty())){
                (~img2);
        }
        pthread_mutex_destroy(&mutex);
        //exit(retval);
}





