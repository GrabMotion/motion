#include "threads/broadcastthread.h"


const int MAXRCVSTRING                      = 4096; // Longest string to receive
//const unsigned int UDP_PORT                 = 5020;

BroadcastThread::BroadcastThread(QObject *parent): QThread(parent)
{

}

void BroadcastThread::run()
{

        unsigned short echoServPort = motion::Message::UDP_PORT;     // First arg:  local port
        char recvString[MAXRCVSTRING + 1]; // Buffer for echo string + \0

        try
        {

          UDPSocket sock(echoServPort);

          string sourceAddress;              // Address of datagram source
          unsigned short sourcePort;         // Port of datagram sourc

          int bytesRcvd = sock.recvFrom(recvString, MAXRCVSTRING, sourceAddress, sourcePort);

          recvString[bytesRcvd] = '\0';  // Terminate string
          cout << "Received " << recvString << " from " << sourceAddress << ": "<< sourcePort << endl;

        } catch (SocketException &e)
        {

            if (e.userMessage.find ("Resource temporarily unavailable") != string::npos)
            {
                emit BroadcastThread::BroadcastTimeoutSocketException();
            }
            cerr << e.what() << endl;
            //exit(1);
        }

        stringstream strm;
        strm << recvString;

        QString str = QString::fromUtf8(strm.str().c_str());

        emit BroadcastReceived(str);

}

bool BroadcastThread::Stop()
{

}

