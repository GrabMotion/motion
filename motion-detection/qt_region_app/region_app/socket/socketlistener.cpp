#include "socket/socketlistener.h"

using namespace std;
using namespace base64;

#define RCVBUFSIZE 500000

SocketListener::SocketListener(QObject *parent): QObject(parent){}

// TCP client handling function
void * SocketListener::HandleTCPClient(TCPSocket *sock, QObject *parent)
{

  //const unsigned int RCVBUFSIZE = 100000; //32;    // Size of receive buffer

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
          //totalBytesReceived += recvMsgSize;     // Keep tally of total bytes
          //echoBuffer[recvMsgSize] = '\0';        // Terminate the string!

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

   int size_init = mm.ByteSize();

   cout << "ByteSize:::::::::::::: " << size_init << endl;

    switch (action)
    {

      case motion::Message::SET_MAT:

          if(mm.ByteSize() > 0)
          {
              if (mm.has_data())
              {

                  std::string mdata = mm.data();

                  std::stringstream input_d;
                  input_d << mdata;

                  // Base64 decode the stringstream
                  base64::decoder D;
                  stringstream decoded;
                  D.decode(input_d, decoded);

                  int width_d = mm.width();
                  int height_d = mm.height();
                  int type_d = mm.typemat();
                  size_t size_d = mm.size(); // = input_d.tellp();

                  cout << "size_d: " << size_d << endl;
                  //input_d.read((char*)(&width_d), sizeof(int));
                  //input_d.read((char*)(&height_d), sizeof(int));
                  //input_d.read((char*)(&type_d), sizeof(int));
                  //input_d.read((char*)(&size_d), sizeof(size_t));

                  char* data_d = new char[size_d];
                  //data_d[size_d] = '\0';
                  decoded.read(data_d, size_d);

                  // Construct the image (clone it so that it won't need our buffer anymore)
                  //cv::Mat m_r = cv::Mat(height_d, width_d, type_d, data_d).clone();

                  cv::Mat m_r = cv::Mat(mm.height(), mm.width(), type_d, data_d).clone();

                  imwrite("/jose/repos/image_2.jpg", m_r);
                  QImage frame = Mat2QImage(m_r);
                  QMetaObject::invokeMethod(parent, "remoteImage", Q_ARG(QImage, frame));
                  delete data_d;

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

    sock->send(bts, strlen(bts));

    google::protobuf::ShutdownProtobufLibrary();

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
