#ifndef MainManager_HPP
# define MainManager_HPP

# include <string>
# include <map>
# include <vector>
# include <utility>

# include "ServerManager.hpp"
# include "../enum.h"

class MainManager
{
    private :
        
        long long                   clientMaxBodySize;
        
        std::string                 root;
        
        std::vector<std::string>    index;
        std::vector<ServerManager>  serverBlocks;

        std::map<int, std::string>  errorPage;

    public :
        MainManager();
        MainManager(const MainManager &other);
        ~MainManager();

        MainManager     &operator = (const MainManager &other);

        void            pollCtlHelp(int epollFd, int fd, uint32_t events);

        // setter

        void            addNewServer(void);
        void            addNewErrorHtml(int errCode, std::string page);

        void            setClientMaxBodySize(long long _size);
        void            setIndexHtml(std::vector <std::string> token);
        void            setRoot(std::string _root);
        
        //getter
    
        size_t                      getServerBlockCount();
        
        long long                   getClientMaxBodySize();
        
        ServerManager               &getServerBlock(int idx);
        ServerManager               &getCurrentServerBlock();
        

        std::vector <std::string>   getIndex();
        std::map <int, std::string> getErrorPage();

        std::string                 getRoot();
        std::string                 getErrorPageWithKey(int key);
        std::string                 getIndexWithIdx(int idx);

        
        void            printParsedConfig();

        void            fillMissingConfigValues();

        // exception

        class       ConflictingIndexDirectiveException : public std::exception {
            public :
                virtual const char          *what() const throw();
        };

        class       ServerBlockNotFoundException : public std::exception {
            public :
                virtual const char          *what() const throw();
        };

};

#endif