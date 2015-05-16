#include "threads/streamingthread.h"

StreamingThread::StreamingThread(QObject *parent): QThread(parent)
{

}

void StreamingThread::StartStreaming(char * serverIp,  int port)
{

    //--------------------------------------------------------
    //networking stuff: socket , connect
    //--------------------------------------------------------

    char*       serverIP = serverIp;
    int         serverPort = port;

    struct  sockaddr_in serverAddr;
    socklen_t           addrLen = sizeof(struct sockaddr_in);

    if ((soket_streaming = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "socket() failed" << std::endl;
    }

    serverAddr.sin_family = PF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);
    serverAddr.sin_port = htons(serverPort);

    if (::connect(soket_streaming, (sockaddr*)&serverAddr, addrLen) < 0) {
        std::cerr << "connect() failed!" << std::endl;
    }

    //----------------------------------------------------------
    //OpenCV Code
    //----------------------------------------------------------

    Mat img;
    img = Mat::zeros(480 , 640, CV_8UC1);
    int imgSize = img.total() * img.elemSize();
    uchar *iptr = img.data;
    int bytes = 0;
    int key = 0;

    //make img continuos
    if ( ! img.isContinuous() )
    {
          img = img.clone();
    }

    std::cout << "Image Size:" << imgSize << std::endl;

    namedWindow("CV Video Client",1);

    while (key != 'q') {

        streamingMutex.lock();

        if ((bytes = recv(soket_streaming, iptr, imgSize , MSG_WAITALL)) == -1) {
            std::cerr << "recv failed, received bytes = " << bytes << std::endl;
        }

        std::cout << "Streaming : " << imgSize << std::endl;

        cv::imshow("CV Video Client", img);

        //frame = Mat2QImage(); //MatToQImage(img);
        frame = Mat2QImage(img);

        streamingMutex.unlock();

        emit StreamingUpdateLabelImage(frame, img);

        if (!run)
            break;

        if (key = cv::waitKey(10) >= 0)
            break;
    }

    close(soket_streaming);
}

void StreamingThread::StopStreaming()
{
    run = false;
}
