#include "udpserver.h"

#define BUFSIZE 509676
#define SERVICE_PORT	21234	/* hard-coded port number */

using namespace std;

UDPServer::UDPServer(QObject *parent) : QObject(parent){}
UDPServer::~UDPServer(){}


void * UDPServer::listen (void * args)
{

    //https://www.cs.rutgers.edu/~pxk/417/notes/sockets/demo-udp-03.html

    QObject *parent = (QObject *)args;
    struct sockaddr_in myaddr;	/* our address */
    struct sockaddr_in remaddr;	/* remote address */
    socklen_t addrlen = sizeof(remaddr);		/* length of addresses */
    int recvlen;			/* # bytes received */
    int fd;				/* our socket */
    char buf[BUFSIZE];	/* receive buffer */

    /* create a UDP socket */
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("cannot create socket\n");
        //return;
    }

    /* bind the socket to any valid IP address and a specific port */

    memset((char *)&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(SERVICE_PORT);

    if (::bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("bind failed");
        //return;
    }

    /* now loop, receiving data and printing what we received */
    for (;;) {
        printf("waiting on port %d\n", SERVICE_PORT);
        recvlen = recvfrom(fd, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
        printf("received %d bytes\n", recvlen);

        if (recvlen > 0)
        {

            buf[recvlen] = 0;
            printf("received message: \"%s\"\n", buf);

            GOOGLE_PROTOBUF_VERIFY_VERSION;

            motion::Message mm;
            bool array = false;

            if (array)
            {
                mm.ParseFromArray(&buf, sizeof(buf));
            }
            else
            {
                const string & data = buf;
                mm.ParseFromString(data);
            }

            qRegisterMetaType<motion::Message>("motion::Message");
            QMetaObject::invokeMethod(parent, "setremoteProto", Q_ARG(motion::Message, mm));

            google::protobuf::ShutdownProtobufLibrary();

        }
    }
    /* never exits */

}

void UDPServer::startListening(QObject *parent)
{
    pthread_t thread_udp;
    int runs;

    //Socket
    runs = pthread_create(&thread_udp, NULL, &UDPServer::listen, parent);
    if ( runs  != 0)
    {
        cerr << "Unable to create thread" << endl;
        cout << "BroadcastSender pthread_create failed." << endl;
    }

    cout << "join thread_udp" << endl;
    //pthread_join(    thread_broadcast,          (void**) &runb);
    cout << "join thread_udp" << endl;
    //pthread_join(    thread_socket,               (void**) &runs);
}
