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


int main(int argc, char *argv[])
{
	static const ssize_t BUFF_SIZE = 1024;
	static const std::string CONTENT_LENGTH_HEADER = "Content-Length: ";

	size_t currentPos;
	size_t nextPos;
	std::string host_str, port_str, absPath, fileName, url;
	const char *host_ch, *port_ch;
	struct addrinfo hints, *result, *iterator;	
	int sockfd;

	// Check that there are command line argument(s). If not, exit. 
	if (argc < 2)
	{
		std::cout << "usage: web-client [URL] [URL] ..." << std::endl;
		return 1;
		
	}
	
	// loop through each command line argument
	for (int i = 1; i < argc; i++)
	{
		url = argv[i];
		/*	
		parse command line arguments to extract host, port, and absPath from http url
		where  http_URL  = "http:" "//" host [ ":" port ] [ abs_path ]
		
		If port is empty or not given, "80" is assumed
		
		If absolute path is not specified, it is given as "/"
		*/
		if (url.find("http://", 0) != 0)
		{
			std::cout << "Error: " << url << " is not a valid url." << std::endl;
			continue;
		}
		
		currentPos = url.find("//") + 2;
		nextPos = url.find(":", currentPos);
		if (nextPos == std::string::npos)
		{
			nextPos = url.find("/", currentPos);
			host_str = url.substr(currentPos, nextPos-currentPos);
			port_str = "80";
		}
		else
		{
			host_str = url.substr(currentPos, nextPos-currentPos);
			currentPos = nextPos + 1;
			nextPos = url.find("/", currentPos);
			port_str = url.substr(currentPos, nextPos-currentPos);
		}
		if (nextPos == std::string::npos) absPath = "/";
		else absPath = url.substr(nextPos);
		
		if (absPath == "/") fileName = "/index.html";
		else
		{
			nextPos = 0;
			do
			{	
				currentPos = nextPos+1;
				nextPos = absPath.find("/", currentPos);
				std::cout << nextPos;
			}while(nextPos != std::string::npos);
			fileName = absPath.substr(currentPos);
		}
		
	
		// convert host and port from string to const char*
		host_ch = host_str.c_str();
		port_ch = port_str.c_str();
		
		std::cout << "host: " << host_ch << std::endl << "port: " << port_ch << std::endl << "abs_path: " << absPath << std::endl << "filename: " << fileName << std::endl;
		
		
		/*
		 * Connect to the web server as specified in the URL
		 */
		
		// fill hints struct with arguments
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		
		// get address information and store in linked list pointed to by result
		if (getaddrinfo(host_ch, port_ch, &hints, &result) != 0)
		{
			perror("getaddrinfo");
			return 3;
		}
		
		// iterate through resulting linked list until we can connect to host
		for(iterator = result; iterator != NULL; iterator = iterator->ai_next)
		{
			// try to create socket and check for error
			sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
			if (sockfd == -1)
			{
				perror("socket");
				continue;
			}
			printf("socket created\n");
			// try to connect to host and check for error
			if (connect(sockfd, result->ai_addr, result->ai_addrlen) == -1)
			{
				close(sockfd);
				perror("connect");
				continue;
			}
			printf("connected\n");
			break;
		}
		// if we can't connect to host, exit.
		if (iterator == NULL)
		{
			fprintf(stderr, "failed to connect");
			return 4;
		}
		
		// free memory allocated to result liked list
		freeaddrinfo(result);
		
		/*
		 *	Send HTTP Request message
		 */
		
		HTTPRequest request;
		request.setMethod("GET");
		request.setURL(absPath);
		
		
		std::vector<uint8_t> getRequest = request.encode();
		ssize_t totBytesSent = 0;
		ssize_t bytesToSend = getRequest.size();
		uint8_t* sendBuffer;

        // Send data until the complete request is sent.
    	while (totBytesSent < bytesToSend)
    	{
        	sendBuffer = &getRequest[totBytesSent];
        	ssize_t bytesSent = send(sockfd, sendBuffer, BUFF_SIZE, 0);
        	if (bytesSent == -1)
        	{
        	    perror("send");
        	    return 5;
        	}
        	totBytesSent += bytesSent;
    	}

    	std::cout << "Request sent" << std::endl;
    	
    	
    	/*
    	 * Receive HTTP Response message
    	 */

    	size_t endHeaders = std::string::npos;
    	size_t startBody = std::string::npos;
		bool isEnd = false;
		char buf[BUFF_SIZE];

		std::string message = "";
		ssize_t contentLength = std::numeric_limits<ssize_t>::max();

        // Continue receiving until the server ends the connection, or until receiving a content body with the lenght
        // specified in the Content-Lenght header.
		while (!isEnd && message.length() - startBody < contentLength) {
			memset(buf, '\0', sizeof(buf));

			ssize_t numBytesReceived = recv(sockfd, buf, BUFF_SIZE, 0);
			if (numBytesReceived == -1) {
				std::cout << std::endl << "Connection ended by the server, assuming complete message received." << std::endl;
        	    isEnd = true;
        	}

        	message += buf;

        	// Obtain the position in the string of the end of the headers if not already obtained. The position of the
        	// beginning of the message body is then this postion + 4.
        	if (endHeaders == std::string::npos) {
        	    endHeaders = message.find("\r\n\r\n");
        	    startBody = endHeaders + 4;
        	}

            // If the content lenght has not been obtained, search the current available data for the Content-Lenght
            // header. If it exists, obtain the value associated with the header.
        	if (contentLength == std::numeric_limits<ssize_t>::max()) {
        	    size_t contentLenghtHeaderStart = message.find(CONTENT_LENGTH_HEADER);
        	    size_t contentLengthHeaderEnd = message.find("\r\n", contentLenghtHeaderStart);
        	    if (contentLenghtHeaderStart != std::string::npos && contentLengthHeaderEnd != std::string::npos) {
        	        contentLength = std::stoi(message.substr(contentLenghtHeaderStart + CONTENT_LENGTH_HEADER.length(), contentLengthHeaderEnd - contentLenghtHeaderStart));
        	    }
        	}
    	}


    	if (message.length() - startBody >= contentLength) {
    	    std::cout << std::endl << "Complete message received." << std::endl;
    	}

        // Create response object.
    	HTTPResponse response;
    	std::vector<uint8_t> wire(message.begin(), message.end());
    	response.consume(wire);
    	 
    	// Handle response object.
    	std::ofstream outfile;
    	switch (response.getStatus())
    	{
    		case 200 :
    			std::cout << response.getStatusString() << std::endl << std::endl;
    			
    			outfile.open(fileName);
    			outfile << response.getMessageBody();
    			break;
    		case 400:
    		case 404:
    		default:
    			std::cout << response.getStatusString() << std::endl << std::endl;
    			std::cout << "Exiting." << std::endl;
    			break;
    	}
	}
}
