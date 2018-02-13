#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "HTTPMessage.hpp"

class HTTPRequest : public HTTPMessage {
private:
        std::string method;

        std::string url;


public:
        void consume(std::vector<uint8_t> wire);

        void setURL(std::string url);

        std::string getURL();

        void setMethod(std::string method);

        std::vector<uint8_t> encode();

};
#endif