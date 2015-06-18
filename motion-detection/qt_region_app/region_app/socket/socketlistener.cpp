#include "socket/socketlistener.h"

using namespace std;

#define RCVBUFSIZE 4096

SocketListener::SocketListener(QObject *parent): QObject(parent){}

// TCP client handling function
void * SocketListener::HandleTCPClient(TCPSocket *sock, QObject *parent)
{

    int value;
    cout << "Handling client ";
    string from;
    try
    {
        from = sock->getForeignAddress();
        cout << from << ":";
    } catch (SocketException &e) {
        cerr << "Unable to get foreign address" << endl;
    }
    try
    {
        cout << sock->getForeignPort();
    } catch (SocketException &e) {
        cerr << "Unable to get foreign port" << endl;
    }
    cout << " with thread " << pthread_self() << endl;

    int bytesReceived = 0;              // Bytes read on each recv()
    int totalBytesReceived = 0;         // Total bytes read

    // Send received string and receive again until the end of transmission
    char echoBuffer[RCVBUFSIZE];
    int recvMsgSize;

    recvMsgSize = sock->recv(echoBuffer, RCVBUFSIZE);
    cout << "recvMsgSize: " << recvMsgSize << endl;


    GOOGLE_PROTOBUF_VERIFY_VERSION;

        motion::Message mm;
        bool array = true;

        if (array)
        {
            mm.ParseFromArray(&echoBuffer, sizeof(echoBuffer));
        }
        else
        {
            mm.ParseFromString(echoBuffer);
        }

        int action = mm.type();
        int size_init = mm.ByteSize();
        std::string mdata = mm.data();

        //Decode from base64
        std::string oridecoded = base64_decode(mdata);
        int ori_size = oridecoded.size();

        //Write base64 to file for checking.
        std::string basefile = "/jose/repos/base64oish_MAC.txt";
        std::ofstream out;
        out.open (basefile.c_str());
        out << oridecoded << "\n";
        out.close();

        //cast to stringstream to read data.
        std::stringstream decoded;
        decoded << oridecoded;

        // The data we need to deserialize.
        int width_d = 0;
        int height_d = 0;
        int type_d = 0;
        int size_d = 0;

        // Read the width, height, type and size of the buffer
        decoded.read((char*)(&width_d), sizeof(int));
        decoded.read((char*)(&height_d), sizeof(int));
        decoded.read((char*)(&type_d), sizeof(int));
        decoded.read((char*)(&size_d), sizeof(int));

        // Allocate a buffer for the pixels
        char* data_d = new char[size_d];
        // Read the pixels from the stringstream
        decoded.read(data_d, size_d);

        cout << "+++++++++++++++++RECEIVING PROTO+++++++++++++++++++"          << endl;
        cout << "width      : " << width_d      << endl;
        cout << "rows       : " << mm.rows()    << endl;
        cout << "height     : " << height_d     << endl;
        cout << "cols       : " << mm.cols()    << endl;
        cout << "Mat type   : " << type_d       << endl;
        cout << "Mat size   : " << size_d       << endl;
        cout << "Proto size : " << size_init    << endl;
        cout << "ori_size   : " << ori_size     << endl;
        cout <<  endl;

        // Construct the image (clone it so that it won't need our buffer anymore)
        cv::Mat deserialized = cv::Mat(height_d, width_d, type_d, data_d).clone();

        // Delete our buffer
        delete[]data_d;

        //Render image.
        imwrite("/jose/repos/image_2.jpg", deserialized);
        QImage frame = Mat2QImage(deserialized);
        QMetaObject::invokeMethod(parent, "remoteImage", Q_ARG(QImage, frame));

        //Build andswer proto.
        motion::Message mr;
        mr.set_type(motion::Message::SET_MAT);
        string datar;
        mr.SerializeToString(&datar);
        char bts[datar.length()];
        strcpy(bts, datar.c_str());
        google::protobuf::ShutdownProtobufLibrary();

        //Send reply.
        sock->send(bts, strlen(bts));

}

struct message_thread_args
{
    TCPSocket *clntSock;
    QObject *parent;

};
struct message_thread_args MessageStructThread;

void * SocketListener::threadMain (void *arg) //void *clntSock)
{
    // Guarantees that thread resources are deallocated upon return
    pthread_detach(pthread_self());

    struct message_thread_args *args = (struct message_thread_args *) arg;

    TCPSocket *clntSock = args->clntSock;
    QObject *parent = args->parent;

    SocketListener sl;
    sl.HandleTCPClient((TCPSocket *) clntSock, parent);

    //delete (TCPSocket *) clntSock;
}


void * SocketListener::socketThread (void * args)
{

    QObject *parent = (QObject *)args;
    pthread_t thread_echo;
    int runt, runb;
    void *status;

    try
    {
        TCPServerSocket servSock(motion::Message::TCP_MSG_PORT);   // Socket descriptor for server
        for (;;) {      // Run forever

            cout << "new TCPServerSocket() runt::" << runt << endl;

            // Create separate memory for client argument
            TCPSocket *clntSock = servSock.accept();

            MessageStructThread.clntSock    = clntSock;
            MessageStructThread.parent      = parent;

            runt =  pthread_create(&thread_echo, NULL, &SocketListener::threadMain, &MessageStructThread); //(void *) clntSock);
            if ( runt  != 0)
            {
                cerr << "Unable to create ThreadMain thread" << endl;
                cout << "ThreadM:::.in pthread_create failed." << endl;
                //exit(1);
            }

            cout << "ThreadMain pthread_create created!!!!!." << endl;
            pthread_join(    thread_echo,               (void**) &runt);
            //cout << "STATUS!!! = " << runt << endl;
            //return (void *) runt;
        }

    } catch (SocketException &e) {
        cerr << e.what() << endl;
        //exit(1);
        QString error = e.what();
        QMetaObject::invokeMethod(parent, "remoteError", Q_ARG(QString&, error));
    }

}

void SocketListener::startListening(QObject *parent)
{
    pthread_t thread_socket;
    int runs;

    //Socket
    runs = pthread_create(&thread_socket, NULL, &SocketListener::socketThread, parent);
    if ( runs  != 0) {
        cerr << "Unable to create thread" << endl;
        cout << "BroadcastSender pthread_create failed." << endl;
    }

    cout << "join thread_broadcast" << endl;
    //pthread_join(    thread_broadcast,          (void**) &runb);
    cout << "join thread_echo" << endl;
    //pthread_join(    thread_socket,               (void**) &runs);
}
