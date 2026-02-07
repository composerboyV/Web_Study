#include "ListeningSocket.hpp"
#include "ConnectionManager.hpp"

#include <unistd.h>
#include <sstream>
#include <cstring>

ListeningSocket::ListeningSocket() : ISocket()
{
    this->type = LISTENING;
    this->alive = false;
    this->fd = -1;

    this->sockPort = -1;
}

ListeningSocket::ListeningSocket(ServerManager *servManagerAddr, int port) : ISocket()
{
    this->type = LISTENING;
    this->alive = false;
    this->fd = -1;

    if (servManagerAddr == NULL)
        throw ServManagerAddrNullException();

    this->servBlocks.push_back(servManagerAddr);
    this->sockIp = (*servManagerAddr).getServerIp();

    if (!(1024 <= port && port <= 49151))
        throw PortInWrongRangeException();
    this->sockPort = port;
}

ListeningSocket::ListeningSocket(const ListeningSocket &other) : ISocket()
{
    this->type = LISTENING;
    this->alive = false;
    
    *this = other;
}

ListeningSocket     &ListeningSocket::operator = (const ListeningSocket &other)
{
    if (this != &other)
    {
        this->fd = other.fd;
        this->alive = other.alive;
        this->type = other.type;

        this->sockIp = other.sockIp;
        this->sockPort = other.sockPort;
        this->servBlocks = other.servBlocks;
    }
    return (*this);
}

ListeningSocket::~ListeningSocket()
{
}

void                ListeningSocket::makeSocketAlive()
{
    struct addrinfo                 sockHints;
    struct addrinfo                 *sockInfo = NULL;
    
    std::memset(&sockHints, 0, sizeof(sockHints));
    sockHints.ai_family = AF_INET;
    sockHints.ai_socktype = SOCK_STREAM;
    sockHints.ai_flags = 0;

    std::stringstream   ss;
    ss << this->sockPort;
    std::string         portStr = ss.str();

    if (getaddrinfo(this->sockIp.c_str(), portStr.c_str(), &(sockHints), &(sockInfo)) != 0)
    {
        std::cerr << "[ERROR] [GETADDRINFO] error while doing getaddrinfo\n";
        throw std::exception();
    }

    this->fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (this->fd == -1)
    {
        std::cerr << "[ERROR] [SOCKET] error while doing socket\n";
        freeaddrinfo(sockInfo);
        throw std::exception();
    }

    int     optval = 1;
    if (setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        std::cerr << "[ERROR] [SETSOCKOPT] error while doing setsockopt\n";
        freeaddrinfo(sockInfo);
        close (this->fd);
        throw std::exception();
    }

    if (bind(this->fd, sockInfo->ai_addr, sockInfo->ai_addrlen) < 0)
    {
        std::cerr << "[ERROR] [BIND] error while doing bind\n";
        freeaddrinfo(sockInfo);
        close (this->fd);
        throw std::exception();
    }

    if (listen(this->fd, SOMAXCONN) < 0)
    {
        std::cerr << "[ERROR] [LISTEN] error while doing listen\n";
        freeaddrinfo(sockInfo);
        close (this->fd);
        throw std::exception();
    }

    this->alive =true;
    freeaddrinfo(sockInfo);
}

void        ListeningSocket::handleEpollEvent(uint32_t event, ConnectionManager &conn)
{
    if (event & EPOLLERR)
    {
        std::cerr << "[ERROR] EPOLLERR event occurred\n";
        close(this->fd);
        this->setAlive(false);
        return ;
    }
    if (event & EPOLLHUP)
    {
        std::cerr << "[ERROR] EPOLLHUP event occured\n";
        close(this->fd);
        this->setAlive(false);
        return ;
    }
    if (event & EPOLLIN)
    {
        int                 clntFd;
        struct sockaddr_in  clntAddr;

        while (true)
        {
            socklen_t       len = sizeof(clntAddr);
            clntFd = accept(this->fd, (sockaddr *)&clntAddr, &len);
            if (clntFd == -1)
                break ;
            conn.addClientSocket(ClientSocket(clntFd, clntAddr, this));
            std::cout << "[DEBUG] new client socket made\n";
        }
    }
}

void                ListeningSocket::printSocketInfo()
{
    std::cout << "  ╰─── Socket IP : " << this->sockIp << "\n";
    std::cout << "  ╰─── Socket PORT : " << this->sockPort << "\n";

    std::cout << "  ╰─── 📑 related ServerBlocks Info \n";
    for (size_t i = 0; i < servBlocks.size(); i++)
    {
        std::cout << "      ╰─── serverBlock number. " << i << "💾\n";
        std::cout << "★。＼｜／。★★。＼｜／。★★。＼｜／。★★。＼｜／。★★。＼｜／。★\n";
        servBlocks[i]->printParsedServer();
        std::cout << "★。／｜＼。★★。／｜＼。★★。／｜＼。★★。／｜＼。★★。／｜＼。★\n";
    }
}

/*
   █████████  ██████████ ███████████ ███████████ ██████████ ███████████  
  ███░░░░░███░░███░░░░░█░█░░░███░░░█░█░░░███░░░█░░███░░░░░█░░███░░░░░███ 
 ███     ░░░  ░███  █ ░ ░   ░███  ░ ░   ░███  ░  ░███  █ ░  ░███    ░███ 
░███          ░██████       ░███        ░███     ░██████    ░██████████  
░███    █████ ░███░░█       ░███        ░███     ░███░░█    ░███░░░░░███ 
░░███  ░░███  ░███ ░   █    ░███        ░███     ░███ ░   █ ░███    ░███ 
 ░░█████████  ██████████    █████       █████    ██████████ █████   █████
  ░░░░░░░░░  ░░░░░░░░░░    ░░░░░       ░░░░░    ░░░░░░░░░░ ░░░░░   ░░░░░ 
*/

std::vector <ServerManager *>	&ListeningSocket::getServerBlocks()
{
	return (this->servBlocks);
}

/*
  █████████  ██████████ ███████████ ███████████ ██████████ ███████████  
 ███░░░░░███░░███░░░░░█░█░░░███░░░█░█░░░███░░░█░░███░░░░░█░░███░░░░░███ 
░███    ░░░  ░███  █ ░ ░   ░███  ░ ░   ░███  ░  ░███  █ ░  ░███    ░███ 
░░█████████  ░██████       ░███        ░███     ░██████    ░██████████  
 ░░░░░░░░███ ░███░░█       ░███        ░███     ░███░░█    ░███░░░░░███ 
 ███    ░███ ░███ ░   █    ░███        ░███     ░███ ░   █ ░███    ░███ 
░░█████████  ██████████    █████       █████    ██████████ █████   █████
 ░░░░░░░░░  ░░░░░░░░░░    ░░░░░       ░░░░░    ░░░░░░░░░░ ░░░░░   ░░░░░ 
*/

void                ListeningSocket::setSockIP(std::string _val)
{
    this->sockIp = _val;
}

void                ListeningSocket::setSockPort(int _val)
{
    this->sockPort = _val;
}

void                ListeningSocket::addServBlocks(ServerManager *newServBlock)
{
    this->servBlocks.push_back(newServBlock);
}

/*
 ██████████ █████ █████   █████████  ██████████ ███████████  ███████████ █████    ███████    ██████   █████
░░███░░░░░█░░███ ░░███   ███░░░░░███░░███░░░░░█░░███░░░░░███░█░░░███░░░█░░███   ███░░░░░███ ░░██████ ░░███ 
 ░███  █ ░  ░░███ ███   ███     ░░░  ░███  █ ░  ░███    ░███░   ░███  ░  ░███  ███     ░░███ ░███░███ ░███ 
 ░██████     ░░█████   ░███          ░██████    ░██████████     ░███     ░███ ░███      ░███ ░███░░███░███ 
 ░███░░█      ███░███  ░███          ░███░░█    ░███░░░░░░      ░███     ░███ ░███      ░███ ░███ ░░██████ 
 ░███ ░   █  ███ ░░███ ░░███     ███ ░███ ░   █ ░███            ░███     ░███ ░░███     ███  ░███  ░░█████ 
 ██████████ █████ █████ ░░█████████  ██████████ █████           █████    █████ ░░░███████░   █████  ░░█████
░░░░░░░░░░ ░░░░░ ░░░░░   ░░░░░░░░░  ░░░░░░░░░░ ░░░░░           ░░░░░    ░░░░░    ░░░░░░░    ░░░░░    ░░░░░ 
*/

const char  *ListeningSocket::ServManagerAddrNullException::what() const throw()
{
    return "[ERROR] the ServerManager Address you're trying to connect is currently NULL\n";
}

const char  *ListeningSocket::PortInWrongRangeException::what() const throw()
{
    return "[ERROR] the port you're trying to connect is out of range\n";
} 