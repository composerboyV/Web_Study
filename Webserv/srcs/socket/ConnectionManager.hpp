#ifndef ConnectionManager_HPP
# define ConnectionManager_HPP

# include <sys/epoll.h>
# include <sys/socket.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <signal.h>

# include <deque>
# include <string>

# include "../configParsing/MainManager.hpp"
# include "ListeningSocket.hpp"
# include "ClientSocket.hpp"
# include "../enum.h"

# define    MAX_EVENT       50

extern volatile sig_atomic_t g_shutdown;



class ConnectionManager {
    private :
        int                                 epollFd;

        MainManager                         &myManager;
        
        std::deque <ClientSocket>           ClientSockets;
        std::deque <ListeningSocket>        ListeningSockets;

    public :
        ConnectionManager(MainManager &_myManager);
        ConnectionManager(const ConnectionManager &other);
        ~ConnectionManager();

        ConnectionManager   &operator = (const ConnectionManager &other);
        
        int                 getEpollFd();

        void        cleanup();
        void        epollMainLoop();
        void        closeClientSockets();
        void        makeListeningSockets();
        void        printListeningSockets();
        void        closeListeningSockets();
        void        removeFdFromEpoll(int fd);
        void        registerListeningSockets();
        void        addClientSocket(ClientSocket tmp);

};

#endif