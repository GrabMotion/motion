#include "threads/streamingthread.h"
#include "socket/PracticalSocket.h"

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

    //namedWindow("CV Video Client",1);

    int count = 0;

    while (1) { //key != 'q') {

        //streamingMutex.lock();
        pthread_mutex_lock(&streamingMutex);

        try {

            if ((bytes = recv(soket_streaming, iptr, imgSize , MSG_PEEK)) == -1) {
                std::cerr << "recv failed, received bytes = " << bytes << std::endl;
            }

            std::cout << "Streaming : " << imgSize << std::endl;
            std::cout << "bytes: " << bytes << std::endl;

            //image received then break
            if (count==100)
            {
                if (soket_streaming){
                    close(soket_streaming);
                }
                if (!(img.empty())){
                    (~img);
                }

                //pthread_mutex_destroy(&streamingMutex);
                break;
            }

            if (img.empty())
            {
                std::cout << "img.empty() : " << std::endl;
            } else
            {
                //cv::imwrite("test.jpg", img);
                // Declare what you need
                //cv::FileStorage file("test_1.png", cv::FileStorage::WRITE);
                // Write to file!
                //file << img;
            }

            //cv::imshow("CV Video Client", img);

            //frame = Mat2QImage(); //MatToQImage(img);
            frame = Mat2QImage(img);

            pthread_mutex_unlock(&streamingMutex);
            //streamingMutex.unlock();

            emit StreamingUpdateLabelImage(frame, img);

            waitKey(10);

            //if (key = cv::waitKey(10) >= 0)
                //break;

        } catch (SocketException &e) {
            std::cerr << "Socket error!" << std::endl;
        }

        count++;

        std::cout << "count: " << count << std::endl;

    }

}

void StreamingThread::StopStreaming()
{
    run = false;
}
