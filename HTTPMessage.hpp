#include <string>
#include <vector>

class HTTPMessage {
protected:
        std::vector<std::tuple<std::string, std::string>> headers;        

        static const std::string VERSION = "HTTP/1.0";
        
        static const std::string SP = " ";
        
        static const std::string CRLF = "\r\n";

public:
        virtual void consume(std::vector<uint8_t> wire);

        virtual std::vector<uint8_t> encode();
}