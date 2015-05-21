#include <stdio.h>
//#include "send_data.hpp"
#include <vector>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/imgproc/imgproc_c.h"
#include<string>

#ifdef linux 
#include<string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#else 
#include <winsock2.h> 
#endif

using namespace std;

int socket01;
int socket02;

int initRemote() {
    
    struct sockaddr_in server, client;

    //    printf("function remote\n  ");
    //    for (int ii = 0; ii < 10; ii++) {
    //        printf("%d ", buff.at(ii)); //255 216 ...
    //    }
    //    printf("buffer size: %d", buff.size());
    //    printf("\n");

#ifndef linux 
    printf("\ninitialising Winsock...");
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("failed. error Code: %d", WSAGetLastError());
        return 1;
    }
    printf("done.\n");
#endif

    printf("create socket...");
    if ((socket01 = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("could not create socket");
    }
    printf("done.\n");

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(5030);

    printf("binding...");
    int bindRet = bind(socket01, (struct sockaddr *) &server, sizeof (server));
    if (bindRet < 0) {
        printf("bind failed with error code %d\n", bindRet);
        return 1;
    }
    printf("done.\n");

    printf("start listening...");
    fflush(stdout);
    listen(socket01, 3);

#ifdef linux
    unsigned int c = sizeof (struct sockaddr_in);
    socket02 = accept(socket01, (struct sockaddr *) &client, (unsigned int*) &c);
    printf("connected (linux).\n");

#else
    int c = sizeof (struct sockaddr_in);
    socket02 = accept(socket01, (struct sockaddr *) &client, (int*) &c);
    printf("connected (windows).\n");
#endif

    return 0;
}

void closeSock() {
#ifndef linux
    printf("socket01: %d\n", socket01);
    printf("socket02: %d\n", socket02);
    printf("closing socket01...");
    closesocket(socket01);
    printf("done.\n");

    printf("WSACleanup...");
    WSACleanup();
    printf("done.\n");
#else
    close(socket01);
#endif
}

int remote(vector<uchar>& buff) {
    printf("sending...");

    int sizeInfo = buff.size();

#ifndef linux
    send(socket02, (char*) (&sizeInfo), 4, 0);
    send(socket02, (char*) (&buff[0]), buff.size(), 0);
#else
    write(socket02, (char*) (&sizeInfo), 4);
    write(socket02, (char*) (&buff[0]), buff.size());
#endif
    printf("done.\n");

    char recvbuf[10] = "";
    printf("receiving...");
    int x = recv(socket02, recvbuf, 10, 0);
    printf("received %s (%d bytes).\n", recvbuf, x);

    return x;
}