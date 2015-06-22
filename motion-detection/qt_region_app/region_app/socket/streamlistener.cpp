#include "socket/streamlistener.h"

#include <google/protobuf/stubs/common.h>

using namespace std;
using namespace google::protobuf::io;

#define RCVBUFSIZE 200000

StreamListener::StreamListener(QObject *parent): QObject(parent){}

struct message_thread_args
{
    int * lp;
    QObject *parent;
};
struct message_thread_args StructThread;

google::protobuf::uint32 StreamListener::readHdr(char *buf)
{
  google::protobuf::uint32 size;
  google::protobuf::io::ArrayInputStream ais(buf,4);
  CodedInputStream coded_input(&ais);
  coded_input.ReadVarint32(&size);//Decode the HDR and get the size
  cout<<"size of payload is "<<size<<endl;
  return size;
}

void StreamListener::readBody(int csock,google::protobuf::uint32 siz, QObject *parent)
{
  int bytecount;
  motion::Message payload;
  char buffer [siz+4];//size of the payload and hdr
  //Read the entire buffer including the hdr
  if((bytecount = recv(csock, (void *)buffer, 4+siz, MSG_WAITALL))== -1){
                fprintf(stderr, "Error receiving data %d\n", errno);
        }
  cout<<"Second read byte count is "<<bytecount<<endl;
  //Assign ArrayInputStream with enough memory
  google::protobuf::io::ArrayInputStream ais(buffer,siz+4);
  CodedInputStream coded_input(&ais);
  //Read an unsigned integer with Varint encoding, truncating to 32 bits.
  coded_input.ReadVarint32(&siz);
  //After the message's length is read, PushLimit() is used to prevent the CodedInputStream
  //from reading beyond that length.Limits are used when parsing length-delimited
  //embedded messages
  google::protobuf::io::CodedInputStream::Limit msgLimit = coded_input.PushLimit(siz);

  //De-Serialize
  payload.ParseFromCodedStream(&coded_input);
  //Once the embedded message has been parsed, PopLimit() is called to undo the limit
  coded_input.PopLimit(msgLimit);
  //Print the message
  //cout<<"Message is "<<payload.DebugString();

  qRegisterMetaType<motion::Message>("motion::Message");
  QMetaObject::invokeMethod(parent, "remoteProto", Q_ARG(motion::Message, payload));

  /*int action = payload.type();
  int size_init = payload.ByteSize();
  int size_data_primitive = payload.data().size();
  std::string mdata = payload.data();
  int size_encoded = mdata.size();

  //Write base64 to file for checking.
  std::string basefile = "/jose/repos/base64oish_MAC.txt";
  std::ofstream out;
  out.open (basefile.c_str());
  out << mdata << "\n";
  out.close();

  //Decode from base64
  std::string oridecoded = base64_decode(mdata);
  int ori_size = oridecoded.size();

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

  // Construct the image (clone it so that it won't need our buffer anymore)
  cv::Mat deserialized = cv::Mat(height_d, width_d, type_d, data_d).clone();

  //Render image.
  imwrite("/jose/repos/image_2.jpg", deserialized);
  QImage frame = Mat2QImage(deserialized);
  QMetaObject::invokeMethod(parent, "remoteImage", Q_ARG(QImage, frame));

  cout << "+++++++++++++++++RECEIVING PROTO+++++++++++++++++++"   << endl;
  cout << "Mat size   : " << deserialized.size                            << endl;
  cout << "Char type  : " << type_d                               << endl;
  cout << "Proto size : " << payload.size()                         << endl;
  cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++"  << endl;
  cout << "size_encoded           : " << size_encoded             << endl;
  cout << "ori_size               : " << ori_size                 << endl;
  cout << "++++++++++++++++++++++++++++++++++++++++++++++++++++"  << endl;
  cout <<  endl;

  // Delete our buffer
  delete[]data_d;*/
}

void * StreamListener::socketHandler (void* lp)
{

  struct message_thread_args *args = (struct message_thread_args *) lp;
  int *csock = args->lp;
  QObject *parent = args->parent;

    char buffer[4];
    int bytecount=0;
    string output,pl;
    //log_packet logp;

    memset(buffer, '\0', 4);

    while (1)
    {
        //Peek into the socket and get the packet size
        if((bytecount = recv(*csock, buffer,4, MSG_PEEK))== -1){
            fprintf(stderr, "Error receiving data %d\n", errno);
        }
        else if (bytecount == 0)
            break;

        cout<<"First read byte count is "<<bytecount<<endl;
        StreamListener::readBody(*csock,StreamListener::readHdr(buffer), parent);
    }



FINISH:
        free(csock);
    return 0;
}

void * StreamListener::socketThread(void * args)
{

    struct message_thread_args *arg = (struct message_thread_args *) args;
    //int *csock = arg->lp;
    QObject *parent = arg->parent;

    int host_port= 1101;
    struct sockaddr_in my_addr;

    int hsock;
    int * p_int ;
    int err;

    socklen_t addr_size = 0;
    int* csock;
    sockaddr_in sadr;
    pthread_t thread_id=0;

    hsock = socket(AF_INET, SOCK_STREAM, 0);
    if(hsock == -1){
            printf("Error initializing socket %d\n", errno);
            goto FINISH;
    }

    p_int = (int*)malloc(sizeof(int));
    *p_int = 1;

    if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
            (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) ){
            printf("Error setting options %d\n", errno);
            free(p_int);
            goto FINISH;
    }
    free(p_int);

    my_addr.sin_family = AF_INET ;
    my_addr.sin_port = htons(host_port);

    memset(&(my_addr.sin_zero), 0, 8);
    my_addr.sin_addr.s_addr = INADDR_ANY ;

    if( ::bind( hsock, (sockaddr*)&my_addr, sizeof(my_addr)) == -1 ){
            fprintf(stderr,"Error binding to socket, make sure nothing else is listening on this port %d\n",errno);
            goto FINISH;
    }
    if(listen( hsock, 10) == -1 ){
            fprintf(stderr, "Error listening %d\n",errno);
            goto FINISH;
    }

    //Now lets do the server stuff

    addr_size = sizeof(sockaddr_in);

    while(true)
    {
            printf("waiting for a connection\n");
            csock = (int*)malloc(sizeof(int));
            StructThread.lp = csock;
            StructThread.parent = parent;
            if((*csock = accept( hsock, (sockaddr*)&sadr, &addr_size))!= -1)
            {
                    printf("---------------------\nReceived connection from %s\n",inet_ntoa(sadr.sin_addr));
                    pthread_create(&thread_id, 0, &StreamListener::socketHandler, &StructThread); //(void*) csock );
                    pthread_detach(thread_id);
            }
            else{
                    fprintf(stderr, "Error accepting %d\n", errno);
            }
    }

    FINISH:
    ;//oops
}

void StreamListener::startListening(QObject *parent)
{
    pthread_t thread_socket;
    int runs;
    StructThread.parent = parent;
    //Socket
    runs = pthread_create(&thread_socket, NULL, &StreamListener::socketThread, &StructThread);
    if ( runs  != 0) {
        cerr << "Unable to create thread" << endl;
        cout << "BroadcastSender pthread_create failed." << endl;
    }
}
