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
 * Source for this method: https://techoverflow.net/2013/08/21/how-to-check-if-file-exists-using-stat/?q=/blog/2013/08/21/how-to-check-if-file-exists-using-stat/
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

    size_t endHeaders = std::string::npos;
    char buf[buffSize];
    std::string message = "";

    std::cout << "Receiving..." << std::endl;

   // Continue receiving until the "\r\n\r\n" delimiting the end of the headers has been received.
    while (endHeaders == std::string::npos) {
        memset(buf, '\0', sizeof(buf));

        ssize_t numBytesReceived = recv(clientSockfd, buf, buffSize, 0);
        if (numBytesReceived == -1) {
            perror("recv");
            return;
        }

        message += buf;

        endHeaders = message.find("\r\n\r\n");

    }

    std::cout << std::endl << "Request received" << std::endl;

    // Handle received request and construct response
    HTTPRequest req;
    std::vector<uint8_t> wire(message.begin(), message.end());
    req.consume(wire);

    std::cout << std::endl << "Creating response" << std::endl;

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
        } else {
            resp.setStatus(404);
            resp.setMessageBody("");
        }
    } else { // Methods other than GET are not supported
        resp.setStatus(400);
        resp.setMessageBody("");
    }


    std::vector<uint8_t> codedResp = resp.encode();
    ssize_t totBytesSent = 0;
    ssize_t bytesToSend = codedResp.size();
    uint8_t* sendBuffer;

    std::cout << "Sending response..." << std::endl;

    // Send data until the complete response is sent.
    while (totBytesSent < bytesToSend) {
        sendBuffer = &codedResp[totBytesSent];
        ssize_t bytesSent = send(clientSockfd, sendBuffer, buffSize, 0);
        if (bytesSent == -1) {
            perror("send");
            return;
        }

        totBytesSent += bytesSent;
    }

    std::cout << std::endl << "Response sent" << std::endl;

    std::cout << "Closing connexion to client" << std::endl;

    close(clientSockfd);
}

int main(int argc, char* argv[]) {

    // Size of the buffers used to receive and send data.
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

    // Obtain the information needed to bind a socket
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

        sockfd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        if (sockfd == -1) {
            perror("socket");
            continue;
        }

        if (bind(sockfd, info->ai_addr, info->ai_addrlen) == -1) {
            close(sockfd);
            perror("bind");
            continue;
        }

        char str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(info->ai_addr->sa_data), str, INET_ADDRSTRLEN);

        std::cout << "Socket created with socket descriptor " << sockfd << ", bound to address " << str << std::endl;

        break; // if we get here, we must have connected successfully
    }



    if (info == NULL) {
        // looped off the end of the list with no successful bind
        fprintf(stderr, "failed to bind socket\n");
        exit(2);
    }

    // free memory allocated to result liked list
    freeaddrinfo(res);

    // Set socket to listen status
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        return 3;
    }

    printf("Listening for clients\n");

    for(;;) {
        // Accept a new connection, then start a new thread for it.
        struct sockaddr_in clientAddr;
        socklen_t clientAddrSize = sizeof(clientAddr);
        int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);

        if (clientSockfd == -1) {
            perror("accept"); //could not accept connection
            return 4;
        }
        std::thread t(handleConnection, clientSockfd, clientAddr, BUFF_SIZE, fileDir);
        t.detach();
    }

    close(sockfd);

    return 0;

}
