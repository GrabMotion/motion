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

    if ((soket_streaming = ::socket(PF_INET, SOCK_STREAM, 0)) < 0) {
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
    img = Mat::zeros(320 , 240, CV_8UC1);
    int imgSize = img.total() * img.elemSize();
    uchar *iptr = img.data;
    int bytes = 0, sentBytes = 0;
    int key = 0;

    //make img continuos
    if ( ! img.isContinuous() )
    {
          img = img.clone();
    }

    std::cout << "Image Size:" << imgSize << std::endl;

    namedWindow("CV Video Client",1);

    int count = 0;

    while (1) { //key != 'q') {

        //streamingMutex.lock();
        pthread_mutex_lock(&streamingMutex);

        try {

            if ((bytes = ::recv(soket_streaming, iptr, imgSize , MSG_PEEK)) == -1) {
                std::cerr << "recv failed, received bytes = " << bytes << std::endl;
            }

            std::cout << "Streaming : " << imgSize << std::endl;
            std::cout << "bytes: " << bytes << std::endl;

            cv::imshow("CV Video Client", img);

            //frame = Mat2QImage(); //MatToQImage(img);
            frame = Mat2QImage(img);

            emit StreamingUpdateLabelImage(frame, img);

            if (!img.empty())
            {

                if (!(img.empty())){
                    (~img);
                }

                std::string  reply = "ok";
                int reply_length = reply.length();


               char *sendString = new char[reply.length() + 1];
               std::strcpy(sendString, reply.c_str());


               std::cout << "Enviando mensaje : " << reply << std::endl;

                //send processed image
                if ((sentBytes = ::send(soket_streaming, sendString, reply_length, 0)) < 0){
                    std::cerr << "bytes = " << reply_length << std::endl;
                }

                break;

            } else
            {

                std::cout << "img.empty(), NO IMAGE COMING:" << std::endl;

                //cv::imwrite("test.jpg", img);
                // Declare what you need
                //cv::FileStorage file("test_1.png", cv::FileStorage::WRITE);
                // Write to file!
                //file << img;
            }


            pthread_mutex_unlock(&streamingMutex);

            waitKey(10);

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
