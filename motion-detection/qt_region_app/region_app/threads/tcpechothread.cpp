#include "threads/tcpechothread.h"

TCPEchoThread::TCPEchoThread(QObject *parent): QThread(parent){}

using namespace std;

const int RCVBUFSIZE  =  100500;

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

  unsigned short echoServPort = motion::Message::ActionType::Message_ActionType_TCP_ECHO_PORT;

  char echoBuffer[RCVBUFSIZE + 1];

  try
  {

    // Establish connection with the echo server
    TCPSocket sock(servAddress, echoServPort);

    // Send the string to the echo server
    sock.send(echoString, echoStringLen);

  } catch(SocketException &e) {
    cerr << e.what() << endl;
    exit(1);
  }

  // Destructor closes the socket

}
