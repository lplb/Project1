#include "HTTPRequest.hpp";

class HTTPRequest : public HTTPMessage {
private:
        std::string method;

        std::string url;


public:
        void consume(std::vector<uint8_t> wire) {
                std::string message(wire->begin(), wire->end());
                size_t curPos = 0;
                size_t nextPos = message.find(SP, curPos);
                size_t endPos = message.find(CRLF+CRLF, nextPos);

                this.method = message.substr(curPos, nextPos-curPos);

                curPos = nextPos + 1;
                size_t nextPos = message.find(SP, curPos);
                this.url = message.substr(curPos, nextPos-curPos);

                std::vector<std::string> newHeaders;
                curPos = message.find(CRLF, nextPos) + 1;
                while (nextPos != endPos) {
                        nextPos = message.find(":", curPos);
                        std::string headerName = message.substr(curPos, nextPos-curPos);

                        curPos = nextPos + 2;
                        nextPos = message.find(CRLF, curPos);
                        std::string headerValue = message.substr(curPos, nextPos-curPos);

                        tuple<std::string,std::string> header = std::make_tuple(headerName, headerValue);
                        
                        newHeaders.pushBack(header);
                }

                this.headers = newHeaders;
        }

        void setURL(std::string url){
                this.url = url;
        }

        void setMethod(std::string method){
                this.method = method;
        }

        std::vector<uint8_t> encode() {
                std:: string message = this.method + SP + this.url + SP + VERSION + CRLF;
                for (tuple<std::string,std::string> header: headers) {
                        message += std::get<0>(header) + ":" + SP + std::get<1>(header) + CRLF;
                }
                message += CRLF;

                std::vector<uint8_t> wire(message.begin(), message.end());

                return wire;
        }

}