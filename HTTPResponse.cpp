#include "HTTPResponse.hpp"

std::string HTTPResponse::getStatusString(){
        switch(this->status) {
                case 200 : return "200" + this->SP + "OK";
                case 400 : return "400" + this->SP + "Bad request";
                case 404 : return "404" + this->SP + "Not found";
        }
}

void HTTPResponse::consume(std::vector<uint8_t> wire) {
        std::string message(wire.begin(), wire.end());
        size_t curPos = message.find(this->SP, 0)+1;
        size_t nextPos = message.find(this->SP, curPos);
        size_t endHeadersPos = message.find(this->CRLF+this->CRLF, nextPos);

        this->status = std::stoi(message.substr(curPos, nextPos-curPos));

//        std::vector<std::string> newHeaders;
        curPos = message.find(this->CRLF, nextPos) + 1;
//        while (nextPos != endHeadersPos) {
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
        this->headers = message.substr(curPos, endHeadersPos-curPos);

        this->messageBody = message.substr(endHeadersPos);
}

void HTTPResponse::setStatus(int status){
        this->status = status;
}

int HTTPResponse::getStatus(){
		return this->status;
}

void HTTPResponse::setMessageBody(std::string messageBody){
        this->messageBody = messageBody;
}

std::string HTTPResponse::getMessageBody(){
		return this->messageBody;
}

std::vector<uint8_t> HTTPResponse::encode() {
        std:: string message = this->VERSION + this->SP + this->getStatusString() + this->CRLF;

//        for (std::tuple<std::string,std::string> header = headers.begin(); header != headers.end(); header++) {
//                message += std::get<0>(header) + ":" + this->SP + std::get<1>(header) + this->CRLF;
//        }
//        for (std::string header = headers.begin(); header != headers.end(); ++header) {
//            message += header + this->CRLF;
//        }

        message += this->headers;
        message += this->CRLF + this->CRLF;

        message += this->messageBody;

        message += this->CRLF + this->CRLF;

        std::vector<uint8_t> wire(message.begin(), message.end());

        return wire;
}