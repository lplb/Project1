#include <stoi>

class HTTPRequest : public HTTPMessage {
private:
        int status;

        std::string messageBody;

        std::string getStatusString(){
                switch(this.status) {
                        case 200 : return "200" + SP + "OK";
                        case 400 : return "400" + SP + "Bad request";
                        case 404 : return "404" + SP + "Not found";
                }
        }


public:
        void consume(std::vector<uint8_t> wire) {
                std::string message(wire->begin(), wire->end());
                size_t curPos = message.find(SP, 0);
                size_t nextPos = message.find(SP, curPos);
                size_t endHeadersPos = message.find(CRLF+CRLF, nextPos);

                this.status = std::stoi(message.substr(curPos, nextPos-curPos));

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

                curPos = nextPos+1;
                this.messageBody = message.substr(curPos);
        }

        void setStatus(int status){
                this.status = status;
        }

        void setMessageBody(string messageBody){
                this.messageBody = messageBody;
        }

        std::vector<uint8_t> encode() {
                std:: string message = VERSION + SP + getStatusString() + CRLF;
                for (tuple<std::string,std::string> header: headers) {
                        message += std::get<0>(header) + ":" + SP + std::get<1>(header) + CRLF;
                }
                message += CRLF;

                message += messageBody;

                std::vector<uint8_t> wire(message.begin(), message.end());

                return wire;
        }

}