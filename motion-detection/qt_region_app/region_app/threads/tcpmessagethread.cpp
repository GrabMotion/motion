#include "threads/tcpmessagethread.h"

TcpMessageThread::TcpMessageThread(QObject *parent): QThread(parent)
{

}


void TcpMessageThread::ReceiveMessage(char *c_str_ip, QString ip)
{

    //--------------------------------------------------------
    //networking stuff: socket , connect
    //--------------------------------------------------------

    char*       serverIP = c_str_ip;
    int         serverPort = TCP_MSG_PORT;

    char messageBuffer[RCVBUFSIZE + 1];

    struct  sockaddr_in serverAddr;
    socklen_t           addrLen = sizeof(struct sockaddr_in);

    if ((soket_message = ::socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "socket() failed" << std::endl;
    }

    serverAddr.sin_family = PF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(serverIP);
    serverAddr.sin_port = htons(serverPort);

    if (::connect(soket_message, (sockaddr*)&serverAddr, addrLen) < 0)
    {
        std::cerr << "connect() failed!" << std::endl;
        return;
    }

    // Buffer for echo string + \0
    int bytesReceived = 0;              // Bytes read on each recv()
    int totalBytesReceived = 0;         // Total bytes read

    try
    {

        TCPSocket sock(serverIP, serverPort);

        char echoBuffer[RCVBUFSIZE + 1];

        if ((bytesReceived = (sock.recv(messageBuffer, RCVBUFSIZE))) <= 0) {
            cerr << "Unable to read";
            exit(1);
        }
        totalBytesReceived += bytesReceived;     // Keep tally of total bytes
        echoBuffer[bytesReceived] = '\0';        // Terminate the string!
        cout << "Received message: " << messageBuffer;                      // Print the echo

        std::stringstream strm;
        strm << echoBuffer;

        emit ResultMessage(strm.str());

    } catch (SocketException &e)
    {
        cerr << e.what() << endl;
        exit(1);
    }

}
