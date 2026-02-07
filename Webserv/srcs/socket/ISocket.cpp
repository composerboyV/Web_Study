#include "ISocket.hpp"
#include "ConnectionManager.hpp"

bool    ISocket::isThisListeningSocket()
{
    if (this->type == LISTENING)
        return (true);
    else
        return (false);
}

bool    ISocket::isAlive()
{
    return (this->alive);
}

void    ISocket::setAlive(bool val)
{
    this->alive = val;            
}

void    ISocket::setFd(int val)
{
    this->fd = val;
}

int     ISocket::getFd()
{
    return (this->fd);
}
