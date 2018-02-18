#include "HTTPRequest.hpp"

void HTTPRequest::consume(std::vector<uint8_t> wire) {
        std::string message(wire.begin(), wire.end());
        ssize_t curPos = 0;
        ssize_t nextPos = message.find(SP, curPos); // End position of the part of the wire corresponding to the method
        ssize_t endPos = message.find(CRLF+CRLF, nextPos); // End position of the request (since there is no message body)

        if (nextPos == -1 || endPos == -1) { // If none of these positions are found
            this->method = "";
            this->url = "";
            this->headers = "";
            return;
        }

        this->method = message.substr(curPos, nextPos-curPos);

        curPos = nextPos + 1; // Start position of the part of the wire corresponding to the URL
        nextPos = message.find(SP, curPos); // End position of the part of the wire corresponding to the URL

        if (nextPos == -1) { // If the end position of the URL is not found
            this->url = "";
        }

        this->url = message.substr(curPos, nextPos-curPos);

        curPos = message.find(CRLF, nextPos) + 1; // Start position of the part of the wire corresponding to the headers

        this->headers = message.substr(curPos, endPos-curPos);
}

void HTTPRequest::setURL(std::string url){
        this->url = url;
}

void HTTPRequest::setMethod(std::string method){
        this->method = method;
}

std::string HTTPRequest::getMethod(){
        return this->method;
}

std::string HTTPRequest::getURL(){
        return this->url;
}

std::vector<uint8_t> HTTPRequest::encode() {
        std:: string message = this->method + this->SP + this->url + this->SP + this->VERSION + this->CRLF;
        message += this->headers;
        message += this->CRLF + this->CRLF;

        std::vector<uint8_t> wire(message.begin(), message.end());

        return wire;
}