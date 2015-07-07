#include "socket/socketlistener.h"
#include "mainwindow.h"

using namespace std;

MainWindow *snwindow;

SocketListener::SocketListener(QObject *parent): QObject(parent)
{
    snwindow = qobject_cast<MainWindow*>(parent);
}

std::string SocketListener::ExtractString( std::string source, std::string start, std::string end )
{
     std::size_t startIndex = source.find( start );
     if( startIndex == std::string::npos )
     {
        return "";
     }
     startIndex += start.length();
     std::string::size_type endIndex = source.find( end, startIndex );
     return source.substr( startIndex, endIndex - startIndex );
}

vector<string> SocketListener::splitString(string input, string delimiter)
{
     vector<string> output;
     char *pch;
     char *str = strdup(input.c_str());
     pch = strtok (str, delimiter.c_str());
     while (pch != NULL)
     {
        output.push_back(pch);
        pch = strtok (NULL,  delimiter.c_str());
     }
     free(str);
     return output;
}

std::vector<std::string> SocketListener::split(const std::string &s, char delim)
{
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}


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
    char echoBuffer[motion::Message::SOCKET_BUFFER_MEDIUM_SIZE];
    int recvMsgSize;

    recvMsgSize = sock->recv(echoBuffer, motion::Message::SOCKET_BUFFER_MEDIUM_SIZE);
    cout << "Bytes Received :::::: " << recvMsgSize << endl;

    stringstream ss;
    ss << echoBuffer;
    string str = ss.str();

    std::string strdecoded = base64_decode(str);

    GOOGLE_PROTOBUF_VERIFY_VERSION;
    motion::Message mm;

    mm.ParseFromArray(strdecoded.c_str(), strdecoded.size());

    //snwindow = qobject_cast<MainWindow*>(parent);

    snwindow->receivedEcho(mm);

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
    //pthread_exit((void *) resutl);

    pthread_exit(NULL);
    return NULL;

}



void * SocketListener::socketThread (void * args)
{

    QObject *parent = (QObject *)args;
    pthread_t thread_echo;
    int runt, runb;
    void *status;

    try
    {
        TCPServerSocket servSock(motion::Message::TCP_ECHO_PORT);   // Socket descriptor for server
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
            }
            cout << "ThreadMain pthread_create created!!!!!." << endl;
            pthread_join(    thread_echo,               (void**) &runt);
            cout << "STATUS!!! = " << runt << endl;
            //return (void *) runt;
        }

    } catch (SocketException &e)
    {
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
