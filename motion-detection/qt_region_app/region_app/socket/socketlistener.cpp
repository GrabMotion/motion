#include "socket/socketlistener.h"
#include "mainwindow.h"

using namespace std;

MainWindow *mainwindow;

SocketListener::SocketListener(QObject *parent): QObject(parent)
{
    mainwindow = qobject_cast<MainWindow*>(parent);
}

std::string SocketListener::ExtractString( std::string source, std::string start, std::string end )
{
     std::size_t startIndex = source.find( start );
     if( startIndex == std::string::npos )
     {
        return "";
     }
     startIndex += start.length();
     std::string::size_type endIndex = source.find( end, startIndex );
     return source.substr( startIndex, endIndex - startIndex );
}

vector<string> SocketListener::splitString(string input, string delimiter)
{
     vector<string> output;
     char *pch;
     char *str = strdup(input.c_str());
     pch = strtok (str, delimiter.c_str());
     while (pch != NULL)
     {
        output.push_back(pch);
        pch = strtok (NULL,  delimiter.c_str());
     }
     free(str);
     return output;
}

std::vector<std::string> SocketListener::split(const std::string &s, char delim)
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


// TCP client handling function
void * SocketListener::HandleTCPClient(TCPSocket *sock, QObject *parent)
{

    int value;
    cout << "Handling client ";
    string from;
    try
    {
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
    char echoBuffer[motion::Message::SOCKET_BUFFER_MEDIUM_SIZE];
    int recvMsgSize;

    cout << "Ready to receive!" << endl;

    bool finished=false;

    while (!complete)
    {

        cout << "Receiving # : " << pcount << " at: " << mainwindow->getTime() << endl;

        recvMsgSize = sock->recv(echoBuffer, motion::Message::SOCKET_BUFFER_MEDIUM_SIZE);
        cout << "Bytes Received :::::: " << recvMsgSize << endl;

         stringstream ss;
         ss << echoBuffer;
         string str = ss.str();

         std::string strdecoded;

         strdecoded.clear();

         std::string del_1  = "PROTO_START_DELIMETER";
         std::string del_2  = "PROTO_STOP_DELIMETER";

         int total_size = str.size();
         std::size_t found = str.find(del_1);

         if (found!=std::string::npos)
         {
            std::string lpay = SocketListener::ExtractString(str, del_1, del_2);
            vector<string> vpay = SocketListener::split(lpay, ':');
            type = atoi(vpay.at(0).c_str());
            mode = atoi(vpay.at(1).c_str());
            int del_pos = str.find(del_2);

            if (type==motion::Message::SINGLE_MESSAGE)
            {
                std:string splsp = str.substr((del_pos+del_2.size()),(str.size()-del_2.size()));
                int payload_size = splsp.size();
                strdecoded = base64_decode(splsp);
                complete=true;
                cout << "SINGLE_MESSAGE" << endl;
            }
            else if (type==motion::Message::SPLITTED_MESSAGE)
            {
                msg_split_vector_size = atoi(vpay.at(2).c_str());
                realsize = atoi(vpay.at(3).c_str());
                string splsi = str.substr((del_pos+del_2.size()),(str.size()-del_2.size()));
                int payload_size = splsi.size();
                payload_holder.push_back(splsi);
                pcount++;
                cout << "SPLITTED_MESSAGE" << endl;
                cout << "Size: " << splsi.size() << endl;
            }
         }


         if (pcount>1)
         {
             payload_holder.push_back(str);
             pcount++;
             cout << "SPLITTED::" << pcount << endl;
             cout << "Size: " << str.size() << endl;
             if (payload_holder.size()==msg_split_vector_size)
             {
                 cout << "COMPLETE!!" << endl;
                 for (int j=0; j<payload_holder.size(); j++)
                 {
                     payload += payload_holder.at(j);
                 }
                 strdecoded = base64_decode(payload);
                 pcount=0;
                 finished=true;
             }
         }


         if (finished)
         {

             GOOGLE_PROTOBUF_VERIFY_VERSION;

             motion::Message mm;
             switch (mode)
             {
                 case motion::Message::SOCKET_PROTO_TOARRAY:
                     mm.ParseFromArray(strdecoded.c_str(), strdecoded.size());
                     break;

                 case motion::Message::SOCKET_PROTO_TOSTRING:
                     mm.ParseFromString(strdecoded);
                     break;
             }

             //Set response to the mainwindow.
             mainwindow->remoteProto(mm);

             google::protobuf::ShutdownProtobufLibrary();

         }
         else
         {
             cout << "something went wrong" << endl;
         }

         motion::Message mr;
         string dataconnect;
         int echoStringLen;
         if (type==motion::Message::SINGLE_MESSAGE)
         {
             mr.set_type(motion::Message::ActionType::Message_ActionType_RESPONSE_OK);
         }
         else if (type==motion::Message::SPLITTED_MESSAGE)
         {
             if (!complete)
             {
                mr.set_type(motion::Message::ActionType::Message_ActionType_RESPONSE_NEXT);
             }
             else
             {
                mr.set_type(motion::Message::ActionType::Message_ActionType_RESPONSE_END);
             }
        }
         mr.set_serverip(mainwindow->getIpAddress());
         mr.set_time(mainwindow->getTime());
         mr.SerializeToString(&dataconnect);
         char bts[dataconnect.length()];
         strcpy(bts, dataconnect.c_str());
         echoStringLen = sizeof(bts);

         //Respond socket.
         sock->send(bts, sizeof(bts));
         if (finished)
                 complete=false;
    }
    cout << "EOT" << endl;
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
    //pthread_exit((void *) resutl);

    pthread_exit(NULL);
    return NULL;

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
            }
            cout << "ThreadMain pthread_create created!!!!!." << endl;
            pthread_join(    thread_echo,               (void**) &runt);
            cout << "STATUS!!! = " << runt << endl;
            //return (void *) runt;
        }

    } catch (SocketException &e)
    {
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
