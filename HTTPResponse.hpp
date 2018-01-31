#include "HTTPMessage.hpp";

class HTTPResponse : public HTTPMessage {
private:
        int status;
        
        std::string messageBody;
        
        std::string getStatusString();


public:
        void consume(std::vector<uint8_t> wire);

        void setStatus(int status);

        void setMessageBody(string messageBody);

        std::vector<uint8_t> encode();

}