#include <string.h>
#include <thread>
#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/stat.h>

#include "HTTPRequest.hpp"
#include "HTTPResponse.hpp"

#define BACKLOG 10



/**
 * Source: https://techoverflow.net/2013/08/21/how-to-check-if-file-exists-using-stat/?q=/blog/2013/08/21/how-to-check-if-file-exists-using-stat/
 *
 * Check if a file exists
 * @return true if and only if the file exists, false else
 */
bool fileExists(const std::string& file) {
    struct stat buf;
    return (stat(file.c_str(), &buf) == 0);
}

void handleConnection(int clientSockfd, sockaddr_in clientAddr, size_t buffSize, const char* fileDir){
    char ipstr[INET_ADDRSTRLEN] = {'\0'};
    inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
    std::cout << "Accept a connection from: " << ipstr << ":" <<
        ntohs(clientAddr.sin_port) << std::endl;

    // read/write data from/into the connection
    ssize_t endHeaders = -1;
    char buf[buffSize];
//    std::stringstream ss;
    std::string message = "";

    std::cout << "Receiving..." << std::endl;
    while (endHeaders < 0) {
        memset(buf, '\0', sizeof(buf));

        ssize_t numBytesReceived = recv(clientSockfd, buf, buffSize, 0);
        if (numBytesReceived == -1) {
            perror("recv");
            return;
        }

        message += buf;
        std::cout << buf << std::endl;

        endHeaders = message.find("\r\n\r\n");

    }

    std::cout << "Request received" << std::endl;

    HTTPRequest req;
    std::vector<uint8_t> wire(message.begin(), message.end());
    req.consume(wire);

    std::cout << "Creating response" << std::endl;

    HTTPResponse resp;

    if (req.getMethod() == "GET") {
        std::string url = req.getURL();
        if (url == "/")
            url = "/index.html";
        std::string file = fileDir + url;

        if (fileExists(file)) {
            resp.setStatus(200);
            std::ifstream ifs(file);
            std::string messageBody((std::istreambuf_iterator<char>(ifs) ), (std::istreambuf_iterator<char>()));
            resp.setMessageBody(messageBody);
            std::cout << messageBody;
        } else {
            resp.setStatus(404);
            resp.setMessageBody("");
        }
    } else {
        resp.setStatus(400);
        resp.setMessageBody("");
    }

    std::cout << resp.getMessageBody();

    std::vector<uint8_t> codedResp = resp.encode();
    ssize_t totBytesSent = 0;
    ssize_t bytesToSend = codedResp.size();
    uint8_t* sendBuffer;

    std::cout << "Sending response..." << std::endl;

    std::cout << &codedResp[0];

    while (totBytesSent < bytesToSend) {
        sendBuffer = &codedResp[totBytesSent];
        ssize_t bytesSent = send(clientSockfd, sendBuffer, buffSize, 0);
        if (bytesSent == -1) {
            perror("send");
            return;
        }
        std::cout << bytesSent << "___" << sendBuffer;

        totBytesSent += bytesSent;
    }

    std::cout << "Response sent" << std::endl;

    close(clientSockfd);
}

int main(int argc, char* argv[]) {

    static const size_t BUFF_SIZE = 1024;

    const char* hostname = "localhost";
    const char* port = "4000";
    const char* fileDir = ".";
    if (argc > 1) {
        hostname  = argv[1];
        if (argc > 2) {
            port = argv[2];
            if (argc > 3) {
                fileDir = argv[3];
            }
        }
    }

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
        std::thread t(handleConnection, clientSockfd, clientAddr, BUFF_SIZE, fileDir);
        t.detach();
    }

    close(sockfd);

    return 0;

}
