#include "socket/socketlistener.h"

using namespace std;

SocketListener::SocketListener(QObject *parent): QObject(parent){}

// TCP client handling function
void * SocketListener::HandleTCPClient(TCPSocket *sock, QObject *parent)
{

  const unsigned int RCVBUFSIZE = 100000; //32;    // Size of receive buffer

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

  bool parse = false;
  int action;
  motion::Message mm;

  bool array = true;

  //while ((

  recvMsgSize = sock->recv(echoBuffer, RCVBUFSIZE);//)// > 0)
  //{ // Zero means

      cout << "recvMsgSize: " << recvMsgSize << endl;

      //if (!parse)
      //{
          totalBytesReceived += recvMsgSize;     // Keep tally of total bytes
          echoBuffer[recvMsgSize] = '\0';        // Terminate the string!

          const string & data = echoBuffer;

          GOOGLE_PROTOBUF_VERIFY_VERSION;

          if (array)
          {

            mm.ParseFromArray(&echoBuffer, sizeof(echoBuffer));
          }
          else
          {
             mm.ParseFromString(data);
          }

          cout << "Type Received: " << mm.type() << endl;

          action = mm.type();

          if (mm.has_time())
          {
                cout << "VALUE!! " << value << " TIME!! " << mm.time() << endl;
          }

          std::string payload;
          QString qpayload;
          std::string mtime;

          if (mm.has_payload())
          {
              payload = mm.payload();
              qpayload = QString::fromStdString(payload);
          }

          if (mm.has_time())
          {
             mtime  = mm.time();
          }

          std::cout << "Action received:: " << action << " time: " << mtime << std::endl;

          //parse = true;

         //break;
      //}
  //}


  switch (action)
  {
  case motion::Message::SET_MAT:

      //motion::Message mbytes;
      //mbytes.ParseFromArray(response.data(), response.size());

      //cv::Mat data_mat;

      if(mm.ByteSize() > 0)
      {
          if (mm.has_data())
          {
              std::string mdata = mm.data();

              int w = mm.width();
              int h = mm.height();

              //cv::Mat image1(w,h,CV_8UC3,cv::Scalar(255,255,255));

              cv::Mat mat(mm.width(), mm.height(), CV_8UC3, &mdata);

              //typedef unsigned char byte;
              //std::vector<byte> vectordata(mdata.begin(),mdata.end());
              // cv::Mat data_mat(vectordata,true);

              QImage frame = Mat2QImage(mat);

              //QPixmap pixmap((QString::fromStdString(path)));
              //QString q_response = QString::fromUtf8(data.c_str());

              QMetaObject::invokeMethod(parent, "remoteImage", Q_ARG(QImage, frame));
                //ui->output->setPixmap(QPixmap::fromImage(frame));
           }
      }

      break;
  }

  google::protobuf::ShutdownProtobufLibrary();

  motion::Message mr;
  mr.set_type(motion::Message::SET_MAT);

  string datar;
  mr.SerializeToString(&datar);
  char bts[datar.length()];
  strcpy(bts, datar.c_str());

  // Echo message back to client
  sock->send(bts, strlen(bts));

  google::protobuf::ShutdownProtobufLibrary();

  //sock->send(echoBuffer, recvMsgSize);

  //totalBytesReceived += recvMsgSize;     // Keep tally of total bytes
  //echoBuffer[recvMsgSize] = '\0';        // Terminate the string!

  //QString q_response = QString::fromUtf8(message.c_str());
  //socket_response = message;
  //QMetaObject::invokeMethod(parent, "remoteMessage", Q_ARG(QString, q_response));
  //QMetaObject::invokeMethod(parent, "remoteMessage", Q_ARG(const char *, echoBuffer));

  // Destructor closes sockets



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
