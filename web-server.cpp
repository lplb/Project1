#include <string.h>
#include <thread>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"


void handleConnection(int clientSockfd, sockaddr_in clientAddr, size_t buffSize){
    char ipstr[INET_ADDRSTRLEN] = {'\0'};
    inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
    std::cout << "Accept a connection from: " << ipstr << ":" <<
        ntohs(clientAddr.sin_port) << std::endl;

    // read/write data from/into the connection
    bool isEnd = false;
    char buf[buffSize] = {0};
//    std::stringstream ss;
    std::string message = "";

    while (!isEnd) {
        memset(buf, '\0', sizeof(buf));

        if (recv(clientSockfd, buf, buffSize, 0) == -1) {
            perror("recv");
            return;
        }

        message += buf;
        std::cout << buf << std::endl;
        if (message.substr(message.length()-2) == "\r\n\r\n")
            isEnd = true;

    }

    HTTPRequest req;
    req.consume(message);


    HTTPResponse resp;

    //switch for resp.setStatus() and setMessageBody()

    std::vector<uint8_t> codedResp = resp.encode();
    int totBytesSent = 0;
    size_t bytesToSend = codedResp.size();

    while (totBytesSent < bytesToSend) {
        buf(codedResp.begin() + totBytesSent, codedResp.begin() + totBytesSent + buffSize);
        size_t bytesSent = send(clientSockfd, buf, buffSize, 0);
        if (bytesSent == -1) {
            perror("send");
            return;
        }

        totBytesSent += bytesSent;
    }

    close(clientSockfd);
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " hostname port file-dir" << std::endl;
        return 1;
    }
    const char* hostname = argv[1];
    const char* port = argv[2];
    const char* fileDir = argv[3];

    struct addrinfo hints, *res, *info;
    int rv;
    int sockfd;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(hostname, port, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }
    // loop through all the results and bind to the first we can
    for(info = res; info != NULL; info = info->ai_next) {
        char str[INET_ADDRSTRLEN];

        // now get it back and print it
        inet_ntop(AF_INET, &(info->ai_addr->sa_data), str, INET_ADDRSTRLEN);
        printf("IP: %s\n", str);
        sockfd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        printf("socket %i\n", sockfd);
        if (sockfd == -1) {
            perror("socket");
            continue;
        }

        if (bind(sockfd, info->ai_addr, info->ai_addrlen) == -1) {
            close(sockfd);
            perror("bind");
            continue;
        }

        break; // if we get here, we must have connected successfully
    }


    if (info == NULL) {
        // looped off the end of the list with no successful bind
        fprintf(stderr, "failed to bind socket\n");
        exit(2);
    }

    freeaddrinfo(res); // all done with this structure

    // set socket to listen status
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        return 3;
    }

    printf("Listening for clients\n");

    for(;;) {
        // accept a new connection
        struct sockaddr_in clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);
        int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);

        if (clientSockfd == -1) {
            perror("accept");
            return 4;
        }
        std::thread t(handleConnection, clientSockfd, clientAddr).detach();
    }

    return 0;

}
