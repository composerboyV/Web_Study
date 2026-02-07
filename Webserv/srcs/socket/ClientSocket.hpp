#ifndef ClientSocket_HPP
# define ClientSocket_HPP

# include <netinet/in.h>
# include <vector>

# include "ISocket.hpp"
# include "ListeningSocket.hpp"
# include "../httpMsg/RequestParser.hpp"
# include "../httpMsg/RequestMsg.hpp"
# include "../httpMsg/ResponseMsg.hpp"
# include "../enum.h"

class ClientSocket : public ISocket{
    private :
        struct sockaddr_in          clntAddr;

        bool                        isEpolloutSet;
        bool                        isErrorOccurred;
        bool                        isResponseFullySent;
        
        size_t                      recvOffset; 
        size_t                      sendOffset;

        ResponseMsg                 myResponse;

        ListeningSocket             *parent;

        RequestParser               myRequestParser;

        std::vector <char>          recvBuff;
        std::vector <char>          sendBuff;

        std::vector <RequestMsg>    requestMessages;
    public :
        ClientSocket();
        ClientSocket(int _fd, sockaddr_in _addr, ListeningSocket *_parent);
        ClientSocket(const ClientSocket &other);
        ~ClientSocket();

        ClientSocket    &operator = (const ClientSocket &other);
        
        int                         handleRead(ConnectionManager &conn);

        void                        printSocketInfo();
        void                        sendHttpResponse();
        void                        addRequestMessage();
        void                        buildHttpResponse();
        void                        moveCompleteRequest();
        void                        sendFallbackResponse();
        void                        handleWrite(ConnectionManager &conn);
        void                        handleEpollEvent(uint32_t event, ConnectionManager &conn);
};

#endif