#ifndef HTTP_MESSAGE_H
#define HTTP_MESSAGE_H

#include <string>
#include <vector>

class HTTPMessage {
protected:
        std::string headers;

        static const std::string VERSION;
        
        static const std::string SP;
        
        static const std::string CRLF;

public:
        virtual void consume(std::vector<uint8_t> wire) = 0;

        virtual std::vector<uint8_t> encode() = 0;
};


#endif