#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include "HTTPMessage.hpp"
#include <sstream>

class HTTPResponse : public HTTPMessage {
private:
        int status;
        
        std::string messageBody;


public:
		std::string getStatusString();
		
        void consume(std::vector<uint8_t> wire);

        void setStatus(int status);
        
        int getStatus();

        void setMessageBody(std::string messageBody);
        
        std::string getMessageBody();

        std::vector<uint8_t> encode();

};
#endif