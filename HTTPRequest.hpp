#include "HTTPMessage.hpp"

class HTTPRequest : public HTTPMessage {
private:
        std::string method;

        std::string url;


public:
        void consume(std::vector<uint8_t> wire);

        void setURL(std::string url);

        void setMethod(std::string method);

        std::vector<uint8_t> encode();

}