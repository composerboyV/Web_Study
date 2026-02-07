#ifndef ServerManager_HPP
# define ServerManager_HPP

# include <string>
# include <vector>
# include <sys/socket.h>

# include "LocationManager.hpp"
# include "../enum.h"

class ServerManager{
    private:
    
        long long                       clientMaxBodySize;
        
        std::string                     root;
        std::string                     servIp;
        
        std::vector <int>               servPort;
        std::vector <std::string>       serverName;
        std::vector <std::string>       index;
        
        std::map <int, std::string>     errorPage;
        
        std::vector <LocationManager>   locationBlocks;


    public:
        ServerManager();
        ServerManager(const ServerManager &other);
        ~ServerManager();

        ServerManager       &operator = (const ServerManager &other);
        
        int                         	getServerPortWithIdx(int idx);
        
        void                            printParsedServer();
        void                            fillMissingConfigValues();

        // getter
        size_t                      	getServerPortCnt();

        long long                   	getClientMaxBodySize();

        LocationManager             	&getLocationBlock(int idx);
        LocationManager             	&getCurrentLocationBlock();
        
        std::string                 	getRoot();
        std::string                 	getServerIp();
        std::string                 	getIndexWithIdx(int idx);
        std::string                     getErrorPageWithKey(int key);
        std::string                 	getServerNameWithIdx(int idx);
        
        std::vector <int>           	getServerPortVector();
        
        std::vector <std::string>   	getServerNameVector(); 
        std::vector <std::string>   	getIndexVector();
        
        std::map <int, std::string>     getErrorPage();
		
        std::vector <LocationManager>	&getLocationBlocks();
        // setter 
        void                            printErrorPage();
        void                            addServerPort(int _port);
        void                            setRoot(std::string _root); 
        void                            setServerIp(std::string _ip);
        void                            setClientMaxBodySize(long long val);
        void                            setIndexHtml(std::vector <std::string> token);
        void                            addNewErrorHtml(int errCode, std::string page);
        void                            setServerName(std::vector <std::string> token);
        void                            setErrorPage(std::map <int, std::string> _errorPage);
        void                            addNewLocation(std::string route, bool equalModifier);
        void                            fillMissingErrorPages(std::map <int, std::string> parents);


        // class
        
        class       ConflictingDirectiveException : public std::exception {
            public :
                virtual const char          *what() const throw();
        };

        class       DuplicateValueException : public std::exception {
            public :
                virtual const char          *what() const throw();
        };

        class       LocationBlockNotFoundException : public std::exception {
            public :
                virtual const char          *what() const throw();
        };


};

#endif