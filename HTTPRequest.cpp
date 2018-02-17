#include "HTTPRequest.hpp"

void HTTPRequest::consume(std::vector<uint8_t> wire) {
        std::string message(wire.begin(), wire.end());
        ssize_t curPos = 0;
        ssize_t nextPos = message.find(SP, curPos);
        ssize_t endPos = message.find(CRLF+CRLF, nextPos);

        if (nextPos == -1 || endPos == -1) {
            this->method = "";
            this->url = "";
            this->headers = "";
            return;
        }

        this->method = message.substr(curPos, nextPos-curPos);

        curPos = nextPos + 1;
        nextPos = message.find(SP, curPos);

        if (nextPos == -1) {
            this->url = "";
        }

        this->url = message.substr(curPos, nextPos-curPos);

//        std::vector<std::tuple<std::string, std::string>> newHeaders;
//        std::vector<std::string> newHeaders;
        curPos = message.find(CRLF, nextPos) + 1;

//        while (nextPos != endPos) {
////                nextPos = message.find(":", curPos);
////                std::string headerName = message.substr(curPos, nextPos-curPos);
////
////                curPos = nextPos + 2;
////                nextPos = message.find(CRLF, curPos);
////                std::string headerValue = message.substr(curPos, nextPos-curPos);
////
////                std::tuple<std::string,std::string> header = std::make_tuple(headerName, headerValue);
//
//                nextPos = message.find(CRLF, curPos);
//                std::string header = message.substr(curPos, nextPos-curPos);
//
//                newHeaders.pushBack(header);
//        }
//
//        this->headers = newHeaders;
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
//        for (std::tuple<std::string,std::string> header = headers.begin(); header != headers.end(); header++) {
//                message += std::get<0>(header) + ":" + this->SP + std::get<1>(header) + this->CRLF;
//        }
//        for (std::string header = headers.begin(); header != headers.end(); ++header) {
//            message += header + this->CRLF;
//        }
        message += this->headers;
        message += this->CRLF + this->CRLF;

        std::vector<uint8_t> wire(message.begin(), message.end());

        return wire;
}