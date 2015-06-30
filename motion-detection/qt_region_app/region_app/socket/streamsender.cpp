#include "streamsender.h"

using namespace std;
using namespace google::protobuf::io;

StreamSender::StreamSender(QObject *parent) : QThread(parent){}

StreamSender::~StreamSender(){}


void StreamSender::sendStream(std::string host, motion::Message payload)
{

        std::cout<<"size after serilizing is "<<payload.ByteSize()<<endl;
        int siz = payload.ByteSize()+4;
        char *pkt = new char [siz];
        google::protobuf::io::ArrayOutputStream aos(pkt,siz);
        CodedOutputStream *coded_output = new CodedOutputStream(&aos);
        coded_output->WriteVarint32(payload.ByteSize());
        payload.SerializeToCodedStream(coded_output);

        int host_port= 1101;

        char * host_name = new char[host.size() + 1];
        std::copy(host.begin(), host.end(), host_name);
        host_name[host.size()] = '\0'; // don't forget the terminating 0

        struct sockaddr_in my_addr;

        char buffer[16384];
        int bytecount;
        int buffer_len=0;

        int hsock;
        int * p_int;
        int err;

        hsock = socket(AF_INET, SOCK_STREAM, 0);
        if(hsock == -1)
        {
            printf("Error initializing socket %d\n",errno);
            goto FINISH;
        }

        p_int = (int*)malloc(sizeof(int));
        *p_int = 1;

        if( (setsockopt(hsock, SOL_SOCKET, SO_REUSEADDR, (char*)p_int, sizeof(int)) == -1 )||
           (setsockopt(hsock, SOL_SOCKET, SO_KEEPALIVE, (char*)p_int, sizeof(int)) == -1 ) )
        {
            printf("Error setting options %d\n",errno);
            free(p_int);
            goto FINISH;
        }
        free(p_int);

        my_addr.sin_family = AF_INET ;
        my_addr.sin_port = htons(host_port);

        memset(&(my_addr.sin_zero), 0, 8);
        my_addr.sin_addr.s_addr = inet_addr(host_name);
        if( ::connect( hsock, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1 )
        {
            if((err = errno) != EINPROGRESS){
                fprintf(stderr, "Error connecting socket %d\n", errno);
                goto FINISH;
            }
        }

        while (1)
        {
            if (bytecount<=siz)
            {
                if( (bytecount=send(hsock, (void *) pkt,siz,0))== -1 ) {
                    fprintf(stderr, "Error sending data %d\n", errno);
                    goto FINISH;
                }
                printf("Sent bytes %d\n", bytecount);
                usleep(5);
            }
            else {
               break;
            }
        }

        delete pkt;

    FINISH:
        close(hsock);

}
