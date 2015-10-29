#include "threads/tcpechothread.h"
#include "mainwindow.h"

MainWindow *mnwindow;

const int RCVBUFSIZE = 100000;

std::vector<std::string> TCPEchoThread::splitProto(const std::string &s, char delim)
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

TCPEchoThread::TCPEchoThread(QObject *parent): QThread(parent)
{
   //parent = parent;
   mnwindow = qobject_cast<MainWindow*>(parent);
}

using namespace std;

void TCPEchoThread::SendEcho (int packagesize, QObject *parent, string svradress, string command)
{
    char * message = new char[command.size() + 1];
    std::copy(command.begin(), command.end(), message);
    message[command.size()] = '\0'; // don't forget the terminating 0
    send(packagesize, parent, svradress, message);
}

void TCPEchoThread::SendEcho (int packagesize, QObject *parent, string svradress, char * message)
{
    send(packagesize, parent, svradress, message);
}

void TCPEchoThread::send (int packagesize, QObject *parent, string svradress, char * message)
{

  string servAddress = svradress;
  char *echoString = message;   // Second arg: string to echo
  int echoStringLen = strlen(echoString);   // Determine input length

  google::protobuf::uint32 pport = motion::Message::TCP_ECHO_PORT;

  google::protobuf::uint32 buffersize = packagesize + 40; //motion::Message::SOCKET_BUFFER_MICRO_SIZE + 40;
  int echoServPort = pport;

  char echoBuffer[buffersize];

  try
  {

    // Establish connection with the echo server
    TCPSocket sock(servAddress, echoServPort);

    // Send the string to the echo server
    sock.send(echoString, echoStringLen);

    delete echoString;

    // Buffer for echo string + \0
    int bytesReceived = 0;              // Bytes read on each recv()
    int totalBytesReceived = 0;         // Total bytes read
    // Receive the same string back from the server
    //cout << "Received: " << endl;                 // Setup to print the echoed string

    stringstream se;
    se.clear();

    std::string del_1  = "PROSTA";
    std::string del_2  = "PROSTO";

    std::string strdecoded;
    int package____size = 1;
    int total__packages = 0;
    int current_package = 0;

    bool limitfound = false;

    string strbuffer;
    strbuffer.clear();

    int counttotal = 0;
    int countwhile = 0;

    int strbuffersize = 0;

    while (1)
    {

        // Receive up to the buffer size bytes from the sender
        if ((bytesReceived = (sock.recv(echoBuffer, buffersize))) <= 0)
        {
           break;
        }

        totalBytesReceived += bytesReceived;     // Keep tally of total bytes
        echoBuffer[bytesReceived] = '\0';        // Terminate the string!

        stringstream zz;
        zz << echoBuffer;
        std::size_t found = zz.str().find(del_1);

        if (found!=std::string::npos)
        {

          std::string lpay = mnwindow->ExtractString(zz.str(), del_1, del_2);
          vector<string> vpay = TCPEchoThread::splitProto(lpay, '::');

          package____size = atoi(vpay.at(0).c_str());
          total__packages = atoi(vpay.at(2).c_str());
          current_package = atoi(vpay.at(4).c_str());

          strbuffer.clear();
          strbuffersize=0;

        }

        stringstream ss;
        ss << echoBuffer;

        strbuffer += ss.str();

        strbuffersize = strbuffer.size();

        cout << "strbuffer: " << strbuffer << endl;
        cout << "******************************" << endl;

    }

    QMetaObject::invokeMethod(parent, "socketMessage", Q_ARG(std::string, strbuffer));


  } catch(SocketException &e)
  {
    cerr << e.what() << endl;
    exit(1);
  }

}
