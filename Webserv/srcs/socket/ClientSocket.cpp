#include "ClientSocket.hpp"
#include "ConnectionManager.hpp"
#include "../httpMsg/RequestMsg.hpp"
#include "../enum.h"
#include <unistd.h>
#include <sstream>


ClientSocket::ClientSocket() : ISocket()
{
    this->type = CLIENT;
    this->alive = false;
    this->parent = NULL;

    this->recvOffset = 0;
    this->sendOffset = 0;

    this->isResponseFullySent = false;
    this->isErrorOccurred = false;
    this->isEpolloutSet = false;
}

ClientSocket::ClientSocket(int _fd, sockaddr_in _addr, ListeningSocket *_parent)
: ISocket()
{
    this->type = CLIENT;
    this->alive = true;
    this->fd = _fd;
    this->clntAddr = _addr;
    this->parent = _parent;
    
    this->recvOffset = 0;
    this->sendOffset = 0;

    this->isResponseFullySent = false;
    this->isErrorOccurred = false;
    this->isEpolloutSet = false;
}

ClientSocket::ClientSocket(const ClientSocket &other): ISocket()
{
    this->type = CLIENT;
    *this = other;
}

ClientSocket    &ClientSocket::operator= (const ClientSocket &other)
{
    if (this != &other)
    {
        this->fd = other.fd;
        this->alive = other.alive;
        this->type = other.type;

        this->clntAddr = other.clntAddr;
        this->parent = other.parent;
        this->recvBuff = other.recvBuff;
        this->recvOffset = other.recvOffset;
        this->sendBuff = other.sendBuff;
        this->sendOffset = other.sendOffset;
        
        this->myRequestParser = other.myRequestParser;
        
        this->requestMessages = other.requestMessages;

        this->myResponse = other.myResponse;

        this->isResponseFullySent = other.isResponseFullySent;
        this->isErrorOccurred = other.isErrorOccurred;
        this->isEpolloutSet = other.isEpolloutSet;
    }
    return (*this);
}

ClientSocket::~ClientSocket()
{
}

void            ClientSocket::handleEpollEvent(uint32_t event, ConnectionManager &conn)
{
    if ((event & EPOLLERR) && this->alive == true)
    {
        std::cout << "[DEBUG] client socket EPOLLERR, closing..\n";
        conn.removeFdFromEpoll(this->fd);
        this->alive = false;
        return ;
    }
    if (event & EPOLLIN)
    {
        if (handleRead(conn) == CLOSE)
        {
            std::cout << "[DEBUG] client socket got 0 during recv, closing..\n";
            conn.removeFdFromEpoll(this->fd);
            this->alive = false;
            return ;
        }
    }
    if (event & EPOLLOUT)
    {
        handleWrite(conn);

        if (this->isResponseFullySent == false && this->isEpolloutSet != true)
        {
            struct epoll_event  ev;
            ev.data.ptr = this;
            ev.events = EPOLLIN | EPOLLOUT;
            epoll_ctl(conn.getEpollFd(), EPOLL_CTL_MOD, this->fd, &ev);
            this->isEpolloutSet = true;
        }
    }
    if (event & EPOLLHUP && this->alive != false)
    {
        std::cout << "[DEBUG] client socket EPOLLHUP, closing..\n";
        conn.removeFdFromEpoll(this->fd);
        this->alive = false;
        return ;
    }
}

int         ClientSocket::handleRead(ConnectionManager &conn)
{    
    int     res;
    char    buff[8192];
        
    res = recv(this->fd, buff, 8192, 0);

    if (res > 0)
    {
        this->recvBuff.insert(this->recvBuff.end(), buff, buff + res);
    }

    while (this->myRequestParser.hasCompleteHttpMessage(this->recvBuff)== true)
    { 
        moveCompleteRequest();

		std::vector<ServerManager *>	&servBlocks = this->parent->getServerBlocks();
        this->requestMessages.front().parseHttpRequest(servBlocks);
        
        this->buildHttpResponse();
        sendHttpResponse();
        this->myResponse.initResponse();
        
        this->requestMessages.erase(this->requestMessages.begin());

        if (this->isErrorOccurred == true)
        {
            conn.removeFdFromEpoll(this->fd);
            this->alive = false;
            return (CLOSE);
        }
        
        if (this->isResponseFullySent == false && this->isEpolloutSet == false)
        {
            struct epoll_event  ev;
            ev.data.ptr = this;
            ev.events = EPOLLIN | EPOLLOUT;
            epoll_ctl(conn.getEpollFd(), EPOLL_CTL_MOD, this->fd, &ev);
            this->isEpolloutSet = true;
            return (KEEP_ALIVE);
        }
    }    

    if (res == 0)
        return (CLOSE);
    else if (res < 0)
        return (KEEP_ALIVE);
    else
        return (KEEP_ALIVE);
}

void            ClientSocket::handleWrite(ConnectionManager &conn)
{
    if (this->sendBuff.empty() == true)
    {
        struct epoll_event  ev;
        ev.data.ptr = this;
        ev.events = EPOLLIN;
        epoll_ctl(conn.getEpollFd(), EPOLL_CTL_MOD, this->fd, &ev);
        this->isEpolloutSet = false;
        return ;
    }

    size_t              res = send(this->fd, &(this->sendBuff[0]), this->sendBuff.size(), 0);


    if ((int)res < 0)
    {
        this->isResponseFullySent = false;
        return ;
    }
    else if (res == 0)
    {
        conn.removeFdFromEpoll(this->fd);
        this->alive = false;
        return ;
    }
    else if (res == this->sendBuff.size())
    {
        this->isResponseFullySent = true;
        this->sendBuff.clear();

        struct epoll_event  ev;
        ev.data.ptr = this;
        ev.events = EPOLLIN;
        epoll_ctl(conn.getEpollFd(), EPOLL_CTL_MOD, this->fd, &ev);
        this->isEpolloutSet = false;
        return ;
    }
    else
    {
        this->isResponseFullySent = false;
        this->sendBuff.erase(this->sendBuff.begin(), this->sendBuff.begin() + res);
        return ;
    }
}

void            ClientSocket::moveCompleteRequest()
{
    RequestMsg      tmp;
    size_t          tmpMsgEndOffset;

    this->myRequestParser.fillRequestMsg(this->recvBuff, tmp);
    this->requestMessages.push_back(tmp);

    if (this->myRequestParser.getErrCode() != -1)
    {
        tmpMsgEndOffset = this->myRequestParser.getMsgEndOffset();
        this->recvBuff.erase(this->recvBuff.begin(), this->recvBuff.begin() + tmpMsgEndOffset + 1);
    }
    else
        this->recvBuff.clear();
    
    this->myRequestParser.resetParser();
}


void            ClientSocket::printSocketInfo()
{
    std::cout << "\n";
}

void            ClientSocket::buildHttpResponse()
{
    std::cout << "[DEBUG] buildHttpResponse() in\n";
    if (this->requestMessages.empty())
    {
        std::cerr << "[ERROR] no request Message received in this socket\n";
        return ;
    }

    this->myResponse.setRequestMsg(&(this->requestMessages.front()));
    this->myResponse.setLocation();
    this->myResponse.makeResponse();

    std::cout << "[DEBUG] HTTP response built succesfully\n";
}

void            ClientSocket::sendHttpResponse()
{
    std::vector <char>  responseData = this->myResponse.getRawResponse();
    size_t              res;

    if (responseData.empty())
    {
        std::cout << "[DEBUG] Response Message is empty, sending fallback response\n";

        sendFallbackResponse();
    }
    else
    {
        std::cout << "[DEBUG] trying to send HTTP response (" << responseData.size() << " bytes)\n";
        if (!this->sendBuff.empty() && this->isEpolloutSet == true)
        {
            std::cout << "[DEBUG] there is something at sendBuff. just storing new response\n";
            this->sendBuff.insert(this->sendBuff.end(), responseData.begin(), responseData.end());
            return ;
        }
        res = send(this->fd, &(responseData[0]), responseData.size(), 0);

		if ((int)res < 0)
        {
            this->isResponseFullySent = false;
            this->sendBuff.insert(this->sendBuff.end(), responseData.begin(), responseData.end());
            return ;
        }
        else if (res == 0)
        {
            this->isResponseFullySent = false;
            this->isErrorOccurred = true;
            return ;
        }
        else if (res == responseData.size())
        {
            this->isResponseFullySent = true;
            return ;
        }
        else
        {
            this->isResponseFullySent = false;
            this->sendBuff.insert(sendBuff.end(), responseData.begin() + res, responseData.end());
            return ;
        }
    }
}

void            ClientSocket::sendFallbackResponse()
{
    std::string         fallbackBody = 
        "<!DOCTYPE html>\n"
        "<html>\n"
        "    <head>\n"
        "        <meta charset=\"UTF-8\">\n"
        "        <title>WebServ - 기본 페이지</title>\n"
        "    </head>\n"
        "    <body>\n"
        "        <h1>WebServ 통합 웹서버</h1>\n"
        "        <p>ResponseMsg가 비어있어 기본 페이지를 표시합니다.</p>\n"
        "        <p><a href=\"/www/index.html\">메인 페이지로 이동</a></p>\n"
        "    </body>\n"
        "</html>\n";
    
    std::stringstream   contentLength;
    contentLength << fallbackBody.size();

    std::string         fallbackHeader = 
        "Content-Type: text/html\r\n"
        "Content-Length: " + contentLength.str() + "\r\n\r\n";

    std::string         fallbackStartLine = "HTTP/1.1 200 OK\r\n";

    std::string         fallbackResponse = fallbackStartLine + fallbackHeader + fallbackBody;

    if (!(this->sendBuff.empty()))
    {
        std::cout << "[DEBUG] there is something at sendBuff. just storing new response\n";
        this->sendBuff.insert(this->sendBuff.end(), fallbackResponse.begin(), fallbackResponse.end());
        this->isResponseFullySent = true;
        return ;
    }
    else 
    {
        size_t              res = send(this->fd, fallbackResponse.c_str(), fallbackResponse.size(), 0);

        if ((int)res < 0)
        {
            this->isResponseFullySent = false;
            this->sendBuff.insert(sendBuff.end(), fallbackResponse.begin(), fallbackResponse.end());
            return ;
        }
        else if (res == 0)
        {
            this->isResponseFullySent = false;
            this->isErrorOccurred = true;
            return ;
        }
        else if (res == fallbackResponse.size())
        {
            this->isResponseFullySent = true;
            return ;
        }
        else
        {
            this->isResponseFullySent = false;
            this->sendBuff.insert(sendBuff.end(), fallbackResponse.begin() + res, fallbackResponse.end()); 
            return ;
        }
    }    
}
