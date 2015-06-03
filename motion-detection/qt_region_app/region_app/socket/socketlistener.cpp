#include "socket/socketlistener.h"

using namespace std;

SocketListener::SocketListener(QObject *parent): QObject(parent){}

// TCP client handling function
void * SocketListener::HandleTCPClient(TCPSocket *sock, QObject *parent)
{

  const unsigned int RCVBUFSIZE = 32;    // Size of receive buffer

  int value;
  cout << "Handling client ";
  string from;
  try {
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

  while ((recvMsgSize = sock->recv(echoBuffer, RCVBUFSIZE)) > 0) { // Zero means

      totalBytesReceived += recvMsgSize;     // Keep tally of total bytes
      echoBuffer[recvMsgSize] = '\0';        // Terminate the string!
      cout << "Received message: " << echoBuffer << endl;                      // Print the echo buff

      std::stringstream strmm;
      std::string message;
      strmm << echoBuffer;
      message = strmm.str();

      QString q_response = QString::fromUtf8(message.c_str());
      socket_response = message;

      QMetaObject::invokeMethod(parent, "remoteMessage", Q_ARG(QString, q_response));

      // end of transmission
      // Echo message back to client

      //sock->send(echoBuffer, recvMsgSize);
  }
  // Destructor closes socket

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

    delete (TCPSocket *) clntSock;
}

void * SocketListener::socketThread (void * args)
{

    QObject *parent = (QObject *)args;
    const unsigned int TCP_ECHO_PORT                = 5010;
    pthread_t thread_echo;
    int runt, runb;
    void *status;

    try
    {
        TCPServerSocket servSock(TCP_ECHO_PORT);   // Socket descriptor for server
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
