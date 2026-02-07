#ifndef ParsingManager_HPP
# define ParsingManager_HPP

# include <sstream>
# include <vector>
# include <string>

# include "MainManager.hpp"
# include "LocationManager.hpp"
# include "ServerManager.hpp"
# include "../enum.h"

class ParsingManager
{
    private :
        
        int             currState;

        MainManager     &myMainManager;

    public :
        ParsingManager(MainManager &_myMainManager);
        ParsingManager(const ParsingManager &other);
        ~ParsingManager();

        ParsingManager                  &operator = (const ParsingManager &other);

        // Main function : parseConfigFile
        void                            processLine(std::string line);
        void                            parseConfigFile(std::string fileName);
        void                            fillGlobalDirective(std::vector <std::string> token);
        void                            fillServerDirective(std::vector <std::string> token);
        void                            fillLocationDirective(std::vector <std::string> token);
        void                            handleGlobalDirective(std::string line, std::vector <std::string> token);
        void                            handleServerDirective(std::string line, std::vector <std::string> token);
        void                            handleLocationDirective(std::string line, std::vector <std::string> token);

        // global directive
        int                             convertErrorCode(const std::string &errCode);
        
        long long                       convertClientMaxBodySize(const std::string &line);

        bool                            canOpenServerBlock(std::vector <std::string> token);
        bool                            canBeParsedAsIndexDirective(std::vector <std::string> token);
        bool                            canBeParsedAsErrorPageDirective(std::vector <std::string> token);
        bool                            canBeParsedAsClientMaxBodySizeDirective(std::vector <std::string> token);

        // server directive
        bool                            checkPortNumber(int port);
        bool                            checkPortNumber(std::string port);
        bool                            checkIpNumber(std::string ip, ServerManager &server);
        bool                            canOpenLocationBlock(std::vector <std::string> token);
        bool                            canBeParsedAsListenDirective(std::vector <std::string> token);
        bool                            canBeParsedAsServerNamesDirective(std::vector <std::string> token);
        
        void                            fillListenDirective(ServerManager &server, std::vector<std::string> token);

        // location directive
        bool                            checkRedirectCode(std::string code);
        bool                            isItSupportedMethod(std::string name);

        void                            fillUploadEnable(LocationManager &loc, const std::string &line);
        void                            fillMethodsDirective(LocationManager &loc, const std::string &line);
        void                            fillAutoIndexDirective(LocationManager &loc, const std::string &line);
        void                            fillUploadStore(LocationManager &loc, std::vector <std::string> token);
        void                            fillCgiDirective(LocationManager &loc, std::vector <std::string> token);
        void                            fillRootDirective(LocationManager &loc, std::vector <std::string> token);
        void                            fillReturnDirective(LocationManager &loc, std::vector <std::string> token);

        bool                            canBeParsedAsRootDirective(std::vector <std::string> token);
        bool                            canBeParsedAsCgiInfoDirective(std::vector <std::string> token);
        bool                            canBeParsedAsMethodsDirective(std::vector <std::string> token);
        bool                            canBeParsedAsRedirectDirective(std::vector <std::string> token);
        bool                            canBeParsedAsAutoIndexDirective(std::vector <std::string> token);
        bool                            canBeParsedAsUploadStoreDirective(std::vector <std::string> token);
        bool                            canBeParsedAsUploadEnableDirective(std::vector <std::string> token);

        // utils
        static bool                     isItAllDigit(std::string line);
        static bool                     isVectorUnique(std::vector <std::string> vector);

        static std::vector<std::string> tokenizer(const std::string &line);
        static std::vector<std::string> tokenizer(const std::string &line, char delimeter);
        
        bool                            containsOpeningBrace(const std::string &line);
        bool                            containsClosingBrace(const std::string &line);

        // exception
        class FileDoesntExistException : public std::exception {
            public :
                virtual const char      *what() const throw();
        };

        class SyntaxErrorException : public std::exception {
            public :
                virtual const char      *what() const throw();
        };

        class DuplicateDirectiveException : public std::exception {
            public :
                virtual const char      *what() const throw();
        };
};

#endif