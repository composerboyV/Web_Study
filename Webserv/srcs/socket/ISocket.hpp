#ifndef ISOCKET_HPP
# define ISOCKET_HPP

# include "../enum.h"
# include <stdint.h>

class ConnectionManager;

class ISocket {
    protected :
        int                 fd;
        int                 type;
        bool                alive;
    
    public :
        ISocket() : fd(-1), type(-1), alive(false) {}
        virtual ~ISocket() {};
        
        virtual void        handleEpollEvent(uint32_t event, ConnectionManager &conn) = 0;
        virtual void        printSocketInfo() = 0;

        void                setFd(int val);
        void                setAlive(bool val);

        bool                isAlive();
        bool                isThisListeningSocket();
        
        int                 getFd();
};

#endif