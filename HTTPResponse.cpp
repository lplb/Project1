#include "HTTPResponse.hpp"

std::string HTTPResponse::getStatusString(){
        switch(this->status) {
                case 200 : return "200" + this->SP + "OK";
                case 400 : return "400" + this->SP + "Bad request";
                case 404 : return "404" + this->SP + "Not found";
                default : return "400" + this->SP + "Bad request";
        }
}

void HTTPResponse::consume(std::vector<uint8_t> wire) {
        std::string message(wire.begin(), wire.end());
        ssize_t curPos = message.find(this->SP, 0)+1; //start position of the status number
        ssize_t nextPos = message.find(this->SP, curPos); //end position of the status number
        ssize_t endHeadersPos = message.find(this->CRLF+this->CRLF, nextPos); //end position of the headers

        this->status = std::stoi(message.substr(curPos, nextPos-curPos));

        curPos = message.find(this->CRLF, nextPos) + 1;//start position of the headers
        this->headers = message.substr(curPos, endHeadersPos-curPos);

        //start position of the message body is 4 positions after the end of the headers.
        this->messageBody = message.substr(endHeadersPos+4);

}

void HTTPResponse::setStatus(int status){
        this->status = status;
}

int HTTPResponse::getStatus(){
		return this->status;
}

void HTTPResponse::setMessageBody(std::string messageBody){
        this->messageBody = messageBody;
        std::stringstream temp;
        temp << "Content-Length:" << this->SP << messageBody.length();
        this->headers = temp.str();
}

std::string HTTPResponse::getMessageBody(){
		return this->messageBody;
}

std::vector<uint8_t> HTTPResponse::encode() {
        std:: string message = this->VERSION + this->SP + this->getStatusString() + this->CRLF;

        message += this->headers;
        message += this->CRLF + this->CRLF;

        message += this->messageBody;

        std::vector<uint8_t> wire(message.begin(), message.end());

        return wire;
}