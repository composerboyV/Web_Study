#include "ConnectionManager.hpp"

#include <unistd.h>
#include <signal.h>

volatile sig_atomic_t g_shutdown = 0;

void signalHandler(int signal)
{
    (void)signal;
    g_shutdown = 1;
}



ConnectionManager::ConnectionManager(MainManager &_myManager) : myManager(_myManager)
{
    this->epollFd = -1;
}

ConnectionManager::~ConnectionManager()
{
    std::cout << "[DEBUG] ConnectionManager destructor called\n";
    if (this->epollFd != -1 || !this->ClientSockets.empty() || !this->ListeningSockets.empty())
    {
        cleanup();
    }
}

ConnectionManager::ConnectionManager(const ConnectionManager &other) : myManager(other.myManager)
{
    std::cout << "ConnectionManager copy constructor called\n";
    *this = other;
}

ConnectionManager   &ConnectionManager::operator= (const ConnectionManager &other)
{
    if (this != &other)
    {
        this->ListeningSockets = other.ListeningSockets;
        this->ClientSockets = other.ClientSockets;
    }
    return (*this);
}

void                ConnectionManager::epollMainLoop()
{
    int evCnt = 0;
    struct epoll_event  ready_list[MAX_EVENT];
    
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    
    this->epollFd = epoll_create(1);
    if (this->epollFd == -1)
    {
        std::cerr << "[ERROR] [EPOLL_CREATE] error... \n";
        throw std::exception();
    }

    std::cout << "[DEBUG] EPOLL FD 등록 완료\n";
    
    makeListeningSockets();
    registerListeningSockets();

    while (!g_shutdown)
    {
        evCnt = epoll_wait(this->epollFd, ready_list, MAX_EVENT, 1000);
        if (evCnt == -1)
            break;
        else if (evCnt == 0)
            continue ;
        for (int i = 0; i < evCnt; i++)
        {
            ISocket *tmpPtr = static_cast<ISocket *>(ready_list[i].data.ptr);
            tmpPtr->handleEpollEvent(ready_list[i].events, *this);
        }
    }

    std::cout << "[DEBUG] SERVER CLOSE.. \n";
    cleanup();
}

void                ConnectionManager::makeListeningSockets()
{
    std::map <std::pair<std::string, int>, size_t>   ipPortMap;
    size_t      serverCnt = this->myManager.getServerBlockCount();
    
    for (size_t i = 0; i < serverCnt; i++)
    {
        ServerManager   &currServ = this->myManager.getServerBlock(i);
        std::string     currIp = currServ.getServerIp();
        size_t          portCnt = currServ.getServerPortCnt();
        
        for (size_t j = 0; j < portCnt; j++)
        {
            int     currPort = currServ.getServerPortWithIdx(j);    
            std::pair <std::string, int> key = std::make_pair(currIp, currPort);
            std::map <std::pair <std::string, int>, size_t>::iterator it = ipPortMap.find(key);

            if (it != ipPortMap.end())
            {
                ListeningSockets[it->second].addServBlocks(&currServ); 
            }
            else
            {
                this->ListeningSockets.push_back(ListeningSocket(&currServ, currPort));
                this->ListeningSockets.back().makeSocketAlive();
                ipPortMap[key] = this->ListeningSockets.size() - 1;
            }   
        }
    }
}

void                ConnectionManager::registerListeningSockets()
{
    for (size_t i = 0; i < this->ListeningSockets.size(); i++)
    {
        struct epoll_event      ev;
        int                     currFd = this->ListeningSockets[i].getFd();

        ev.data.ptr = (void *)&(this->ListeningSockets[i]);
        ev.events = EPOLLIN;

        epoll_ctl(this->epollFd, EPOLL_CTL_ADD, currFd, &ev);
    }
}

void                ConnectionManager::addClientSocket(ClientSocket tmp)
{
    this->ClientSockets.push_back(tmp);

    ClientSocket    *sockPtr = &(this->ClientSockets.back());

    struct epoll_event  ev;

    ev.data.ptr = sockPtr;
    ev.events = EPOLLIN;

    if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, sockPtr->getFd(), &ev) == -1)
    {
        if (sockPtr->isAlive() == true)
            close (sockPtr->getFd());
        this->ClientSockets.pop_back();
    }
}


void                ConnectionManager::removeFdFromEpoll(int fd)
{
    if (this->epollFd == -1)
        return ;
    epoll_ctl(this->epollFd, EPOLL_CTL_DEL, fd, NULL);
    close (fd);
}


void                ConnectionManager::closeListeningSockets()
{
    for (size_t i = 0; i < this->ListeningSockets.size(); i++)
    {
        if (ListeningSockets[i].isAlive() == true)
        {
            int fd = ListeningSockets[i].getFd();
            if (this->epollFd != -1)
                epoll_ctl(this->epollFd, EPOLL_CTL_DEL, fd, NULL);
            close(fd);
            std::cout << "[DEBUG] Listening socket fd[" << fd << "] closed\n";
        }
    }
    this->ListeningSockets.clear();
}

void                ConnectionManager::closeClientSockets()
{
    for (size_t i = 0; i < this->ClientSockets.size(); i++)
    {
        if (ClientSockets[i].isAlive() == true)
        {
            int fd = ClientSockets[i].getFd();
            if (this->epollFd != -1)
                epoll_ctl(this->epollFd, EPOLL_CTL_DEL, fd, NULL);
            close(fd);
            std::cout << "[DEBUG] Client socket fd[" << fd << "] closed\n";
        }
    }
    this->ClientSockets.clear();
}

void                ConnectionManager::printListeningSockets()
{
    std::cout << "\n█ ✪ █▓▓▓▓▓▓▓▓▓▓▓▓▓▓ListeningSockets▓▓▓▓▓▓▓▓▓▓▓▓▓▓█ ✪ █\n";

    std::cout << "ListeningSockets size : " << this->ListeningSockets.size() << "\n";
    for (size_t i = 0; i < this->ListeningSockets.size(); i++)
    {
        std::cout << " 🪬 ListeningSocket [" << i << "] \n";
        this->ListeningSockets[i].printSocketInfo();
    }

    std::cout << "█ ✪ █▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓█ ✪ █\n";
}



int                 ConnectionManager::getEpollFd()
{
    return (this->epollFd);
}


void                ConnectionManager::cleanup()
{
    std::cout << "[DEBUG] Starting cleanup process...\n";
    
    closeClientSockets();
    std::cout << "[DEBUG] Client sockets closed\n";
    
    closeListeningSockets();
    std::cout << "[DEBUG] Listening sockets closed\n";
    
    if (this->epollFd != -1)
    {
        close(this->epollFd);
        this->epollFd = -1;
        std::cout << "[DEBUG] Epoll fd closed\n";
    }
    
    std::cout << "[DEBUG] Cleanup completed\n";
}
