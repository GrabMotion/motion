#include "threads/tcpechothread.h"

TCPEchoThread::TCPEchoThread(QObject *parent): QThread(parent){}

using namespace std;

void TCPEchoThread::SendEcho (string svradress, string command)
{
    char * message = new char[command.size() + 1];
    std::copy(command.begin(), command.end(), message);
    message[command.size()] = '\0'; // don't forget the terminating 0
    send(svradress, message);
}

void TCPEchoThread::SendEcho (string svradress, char * message)
{
    send(svradress, message);
}

void TCPEchoThread::send (string svradress, char * message)
{
  string servAddress = svradress;
  char *echoString = message;   // Second arg: string to echo
  int echoStringLen = strlen(echoString);   // Determine input length

  google::protobuf::uint32 pport = motion::Message::TCP_ECHO_PORT;
  int echoServPort = pport;

  char echoBuffer[RCVBUFSIZE + 1];

  try
  {

    // Establish connection with the echo server
    TCPSocket sock(servAddress, echoServPort);

    // Send the string to the echo server
    sock.send(echoString, echoStringLen);

        // Buffer for echo string + \0
    int bytesReceived = 0;              // Bytes read on each recv()
    int totalBytesReceived = 0;         // Total bytes read
    // Receive the same string back from the server
    //cout << "Received: " << endl;                 // Setup to print the echoed string

    while (totalBytesReceived < echoStringLen) {
      // Receive up to the buffer size bytes from the sender
      if ((bytesReceived = (sock.recv(echoBuffer, RCVBUFSIZE))) <= 0) {
        cerr << "Unable to read";
        exit(1);
      }
      totalBytesReceived += bytesReceived;     // Keep tally of total bytes
      echoBuffer[bytesReceived] = '\0';        // Terminate the string!
      //cout << "Received message: " << echoBuffer;                      // Print the echo buffer
    }
    //cout << endl;

    std::stringstream strm;
    strm << echoBuffer;

    const string & data = echoBuffer;

    emit ResultEcho(data);

  } catch(SocketException &e) {
    cerr << e.what() << endl;
    exit(1);
  }

}
