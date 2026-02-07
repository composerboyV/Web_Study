#ifndef ListeningSocket_HPP
# define ListeningSocket_HPP

# include <string>
# include <vector>
# include <iostream>
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>

# include "../configParsing/ServerManager.hpp"
# include "ISocket.hpp"
# include "../enum.h"

class ListeningSocket : public ISocket {
    private :
        int                             sockPort;

        std::string                     sockIp;
        
        std::vector <ServerManager *>   servBlocks;

    public :
        ListeningSocket();
        ListeningSocket(ServerManager *servManagerAddr, int port);
        ListeningSocket(const ListeningSocket &other);
        ~ListeningSocket();

        ListeningSocket     &operator = (const ListeningSocket &other);

        void                            printSocketInfo();
        void                            makeSocketAlive();
        void                            addServBlocks(ServerManager *newServBlock);
        void                            handleEpollEvent(uint32_t event, ConnectionManager &conn);

		// setter
        void                            setSockPort(int _val);
        void                            setSockIP(std::string _val);

		// getter
		std::vector <ServerManager *>	&getServerBlocks();

        // exception
        class       ServManagerAddrNullException : public std::exception {
            public :
                virtual const char          *what() const throw();
        };

        class       PortInWrongRangeException : public std::exception {
            public :
                virtual const char          *what() const throw();
        };

};

#endif