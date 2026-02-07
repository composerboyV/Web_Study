#include "ParsingManager.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <unistd.h>
#include <cstdlib>

ParsingManager::ParsingManager(MainManager &_myMainManager) : myMainManager(_myMainManager)
{
    this->currState = GLOBAL_DIRECTIVE;
}

ParsingManager::ParsingManager(const ParsingManager &other) : myMainManager(other.myMainManager)
{
    *this = other;
}

ParsingManager  &ParsingManager::operator= (const ParsingManager &other)
{
    if (this != &other)
    {
        this->currState = other.currState;
    }
    return (*this);
}

ParsingManager::~ParsingManager()
{
}

/*
 ██████   ██████   █████████   █████ ██████   █████    ███████████ █████  █████ ██████   █████   █████████  ███████████ █████    ███████    ██████   █████
░░██████ ██████   ███░░░░░███ ░░███ ░░██████ ░░███    ░░███░░░░░░█░░███  ░░███ ░░██████ ░░███   ███░░░░░███░█░░░███░░░█░░███   ███░░░░░███ ░░██████ ░░███ 
 ░███░█████░███  ░███    ░███  ░███  ░███░███ ░███     ░███   █ ░  ░███   ░███  ░███░███ ░███  ███     ░░░ ░   ░███  ░  ░███  ███     ░░███ ░███░███ ░███ 
 ░███░░███ ░███  ░███████████  ░███  ░███░░███░███     ░███████    ░███   ░███  ░███░░███░███ ░███             ░███     ░███ ░███      ░███ ░███░░███░███ 
 ░███ ░░░  ░███  ░███░░░░░███  ░███  ░███ ░░██████     ░███░░░█    ░███   ░███  ░███ ░░██████ ░███             ░███     ░███ ░███      ░███ ░███ ░░██████ 
 ░███      ░███  ░███    ░███  ░███  ░███  ░░█████     ░███  ░     ░███   ░███  ░███  ░░█████ ░░███     ███    ░███     ░███ ░░███     ███  ░███  ░░█████ 
 █████     █████ █████   █████ █████ █████  ░░█████    █████       ░░████████   █████  ░░█████ ░░█████████     █████    █████ ░░░███████░   █████  ░░█████
░░░░░     ░░░░░ ░░░░░   ░░░░░ ░░░░░ ░░░░░    ░░░░░    ░░░░░         ░░░░░░░░   ░░░░░    ░░░░░   ░░░░░░░░░     ░░░░░    ░░░░░    ░░░░░░░    ░░░░░    ░░░░░ 
*/

void    ParsingManager::parseConfigFile(std::string fileName)
{
    std::ifstream   file(fileName.c_str());
    std::string     line;

    if (!(file.is_open())) //file open 검사
        throw FileDoesntExistException();
    
    while (getline(file, line, ';'))
    {
        processLine(line);
        if (this->currState == UNKNOWN_DIRECTIVE)
        {
            std::cerr << "[ERROR] [parseConfigFile] 🥸 Encountered UNKNOWN_DIRECTIVE state after processing line." << "\n";
            throw SyntaxErrorException();
        }
    }

    if (this->currState != GLOBAL_DIRECTIVE) 
    { 
        std::cerr << "[ERROR] [parseConfigFile] 🥸 Unexpected end of config file: unclosed directive block detected." << "\n";
        throw SyntaxErrorException();
    }
}

void    ParsingManager::processLine(std::string line)
{
    std::vector <std::string>   token = tokenizer(line);

    switch (this->currState)
    {
        case GLOBAL_DIRECTIVE :
            handleGlobalDirective(line, token);
            break ;
        case SERVER_DIRECTIVE :
            handleServerDirective(line, token);
            break ;
        case LOCATION_DIRECTIVE :
            handleLocationDirective(line, token);
            break ;
        default:
            std::cerr << "[FATAL] [ERROR] [processLine] 🥸 Unexpected state encountered:" << this->currState << "\n";
            throw SyntaxErrorException();
    }
}

/*
   █████████  █████          ███████    ███████████    █████████   █████      
  ███░░░░░███░░███         ███░░░░░███ ░░███░░░░░███  ███░░░░░███ ░░███       
 ███     ░░░  ░███        ███     ░░███ ░███    ░███ ░███    ░███  ░███       
░███          ░███       ░███      ░███ ░██████████  ░███████████  ░███       
░███    █████ ░███       ░███      ░███ ░███░░░░░███ ░███░░░░░███  ░███       
░░███  ░░███  ░███      █░░███     ███  ░███    ░███ ░███    ░███  ░███      █
 ░░█████████  ███████████ ░░░███████░   ███████████  █████   █████ ███████████
  ░░░░░░░░░  ░░░░░░░░░░░    ░░░░░░░    ░░░░░░░░░░░  ░░░░░   ░░░░░ ░░░░░░░░░░░ 
*/

void    ParsingManager::handleGlobalDirective(std::string line, std::vector <std::string> token)
{
    if (containsOpeningBrace(line) == true)
    {
        if (canOpenServerBlock(token) == true)
        {
            this->myMainManager.addNewServer();
            this->currState = SERVER_DIRECTIVE;

            std::string     newLine = line.substr(line.find('{') + 1);
            token.erase(token.begin(), token.begin() + 2);
            handleServerDirective(newLine, token);
        }
        else
        {
            this->currState = UNKNOWN_DIRECTIVE;
            std::cerr << "[ERROR] [handleGlobalDirective] 🥸 GLOBAL_DIRECTIVE received '{' without a valid directive like 'server'." << "\n";
            throw SyntaxErrorException();
        }
    }
    else if (containsClosingBrace(line) == true)
    {
        this->currState = UNKNOWN_DIRECTIVE;
        std::cerr << "[ERROR] [handleGlobalDirective] 🥸 Closing brace '}' found in GLOBAL_DIRECTIVE state — no block to close." << "\n";
        throw SyntaxErrorException();
    }
    else
    {
        fillGlobalDirective(token);
    }
}

void    ParsingManager::fillGlobalDirective(std::vector <std::string> token)
{
    if (canBeParsedAsClientMaxBodySizeDirective(token))
    {
        if (this->myMainManager.getClientMaxBodySize() != -1)
            throw DuplicateDirectiveException();
        this->myMainManager.setClientMaxBodySize(convertClientMaxBodySize(token[1]));
    }
    else if (canBeParsedAsErrorPageDirective(token))
    {
        this->myMainManager.addNewErrorHtml(convertErrorCode(token[1]), token[2]);
    }
    else if (canBeParsedAsIndexDirective(token))
    {
        token.erase(token.begin());
        if (isVectorUnique(token) == false)
        {
            std::cerr << "[ERROR] [fillGlobalDirective] 🥸 Duplicate values found in index directive — values must be unique." << "\n";
            throw SyntaxErrorException();
        }
        this->myMainManager.setIndexHtml(token);
    }
    else if (canBeParsedAsRootDirective(token))
    {
        if (this->myMainManager.getRoot() != "")
        {
            std::cerr << "[ERROR] [fillGlobalDirective] 🥸 Duplicate 'root' directive — only one allowed per location block." << "\n";
            throw SyntaxErrorException();
        }
        this->myMainManager.setRoot(token[1]);
    }
    else
    {
        std::cerr << "[ERROR] [fillGlobalDirective] 🥸 Failed to parse line — unsupported or malformed directive in GLOBAL scope.\n";
        throw SyntaxErrorException();
    }
}

long long   ParsingManager::convertClientMaxBodySize(const std::string &line)
{
    unsigned long           i = 0;
    unsigned long           lineLen = line.length();

    while (i < lineLen && ('0' <= line[i] && line[i] <= '9'))
        i++;

    if (i == 0)
    {
        std::cerr << "[ERROR] [convertClientMaxBodySize] 🥸 No digits found in input: \"" << line << "\"" << "\n";
        throw SyntaxErrorException();
    }

    std::string             digitPart = line.substr(0, i);
    std::string             nonDigitPart = line.substr(i);

    std::istringstream      iss(digitPart);
    long long               res = 0;

    if (!(iss >> res) || !iss.eof())
    {
        std::cerr << "[ERROR] [convertClientMaxBodySize] 🥸 Failed to parse numeric part from: \"" << digitPart << "\"" << "\n";
        throw SyntaxErrorException();
    }

    if (nonDigitPart == "K" || nonDigitPart == "k")
        res *= 1024;
    else if (nonDigitPart == "M" || nonDigitPart == "m")
        res *= (1024 * 1024);
    else if (nonDigitPart == "G" || nonDigitPart == "g")
        res *= (1024 * 1024 *1024);
    else if (!nonDigitPart.empty())
    {
        std::cerr << "[ERROR] [convertClientMaxBodySize] 🥸 Invalid size unit suffix: \"" << nonDigitPart << "\"" << "\n";
        throw SyntaxErrorException();
    }

    return (res);
}

int         ParsingManager::convertErrorCode(const std::string &errCode)
{
    if (isItAllDigit(errCode) == false)
    {
        std::cerr << "[ERROR] [convertErrorCode] 🥸 Error code must consist only of digits. Got: \"" << errCode << "\"" << "\n";
        throw SyntaxErrorException();
    }
    
    std::istringstream  iss(errCode);
    int                 res;

    if (!(iss >> res) || !iss.eof()) 
    {
        std::cerr << "[ERROR] [convertErrorCode] 🥸 Failed to parse numeric part from: \"" << errCode << "\"" << "\n";
        throw SyntaxErrorException();
    }

    if (100 <= res && res <= 599)
        return (res);
    else
    {
        std::cerr << "[ERROR] [convertErrorCode] 🥸 Error code out of valid HTTP range(100~599): " << res << "\n";
        throw SyntaxErrorException();
    }
}

bool    ParsingManager::canOpenServerBlock(std::vector <std::string> token)
{
    return (token.size() >= 4 && token[0] == "server" && token[1] == "{");
}


bool    ParsingManager::canBeParsedAsClientMaxBodySizeDirective(std::vector <std::string> token)
{
    return (token.size() == 2 && token[0] == "client_max_body_size");
}

bool    ParsingManager::canBeParsedAsIndexDirective(std::vector <std::string> token)
{
    return (token.size() >= 2 && token[0] == "index");
}

bool    ParsingManager::canBeParsedAsErrorPageDirective(std::vector <std::string> token)
{
    return (token.size() == 3 && token[0] == "error_page");
}

/*
  █████████  ██████████ ███████████   █████   █████ ██████████ ███████████  
 ███░░░░░███░░███░░░░░█░░███░░░░░███ ░░███   ░░███ ░░███░░░░░█░░███░░░░░███ 
░███    ░░░  ░███  █ ░  ░███    ░███  ░███    ░███  ░███  █ ░  ░███    ░███ 
░░█████████  ░██████    ░██████████   ░███    ░███  ░██████    ░██████████  
 ░░░░░░░░███ ░███░░█    ░███░░░░░███  ░░███   ███   ░███░░█    ░███░░░░░███ 
 ███    ░███ ░███ ░   █ ░███    ░███   ░░░█████░    ░███ ░   █ ░███    ░███ 
░░█████████  ██████████ █████   █████    ░░███      ██████████ █████   █████
 ░░░░░░░░░  ░░░░░░░░░░ ░░░░░   ░░░░░      ░░░      ░░░░░░░░░░ ░░░░░   ░░░░░ 
*/

void    ParsingManager::handleServerDirective(std::string line, std::vector <std::string> token)
{
    if (containsClosingBrace(line))
    {
        if (token[0] != "}")
        {
            std::cerr << "[ERROR] [handleServerDirective] 🥸 Unexpected closing brace. First token must be '}' to close server block." << "\n";
            throw SyntaxErrorException();
        }

        this->currState = GLOBAL_DIRECTIVE;

        std::string     newLine = line.substr(line.find('}') + 1);
        token.erase(token.begin());
        if (!token.empty())
            handleGlobalDirective(newLine, token);
    }
    else if (containsOpeningBrace(line))
    {
        if (canOpenLocationBlock(token))
        {
            this->currState = LOCATION_DIRECTIVE;
            std::string     newLine = line.substr(line.find('{') + 1);

            if (token[1] == "=")
            {
                this->myMainManager.getCurrentServerBlock().addNewLocation(token[2], true);
                token.erase(token.begin(), token.begin() + 4);
                
            }
            else
            {
                this->myMainManager.getCurrentServerBlock().addNewLocation(token[1], false);
                token.erase(token.begin(), token.begin() + 3);
            }

            handleLocationDirective(newLine, token);
        }
        else
        {
            this->currState = UNKNOWN_DIRECTIVE;
            std::cerr << "[ERROR] [handleServerDirective] 🥸 Attempted to open a location block, but directive format is invalid." << "\n";
            throw SyntaxErrorException();
        }
    }
    else
    {
        this->currState = SERVER_DIRECTIVE;
        fillServerDirective(token);
    }
}

bool    ParsingManager::canOpenLocationBlock(std::vector <std::string> token)
{
    if (token.size() < 5 || token[0] != "location")
        return (false);
    
    if (token[2] == "{" && !containsOpeningBrace(token[1]) && token[1] != "=")
        return (true);
    else if (token[3] == "{" && token[1] == "=" && !containsOpeningBrace(token[2]))
        return (true);
    else
        return (false);
}

void    ParsingManager::fillServerDirective(std::vector <std::string> token)
{
    ServerManager   &currServ = this->myMainManager.getCurrentServerBlock();

    if (canBeParsedAsListenDirective(token))
    {
        fillListenDirective(currServ, token);
    }
    else if (canBeParsedAsServerNamesDirective(token))
    {
        token.erase(token.begin());

        if (isVectorUnique(token) == false)
        {
            std::cerr << "[ERROR] [fillServerDirective] 🥸 Duplicate values in 'server_name' directive — all names must be unique." << "\n";
            throw SyntaxErrorException();
        }
        currServ.setServerName(token);
    }
    else if (canBeParsedAsIndexDirective(token))
    {
        token.erase(token.begin());
        if (isVectorUnique(token) == false)
        {
            std::cerr << "[ERROR] [fillServerDirective] 🥸 Duplicate entries in 'index' directive — values must be unique." << "\n";
            throw SyntaxErrorException();
        }
        currServ.setIndexHtml(token);
    }
    else if (canBeParsedAsClientMaxBodySizeDirective(token))
    {
        if (currServ.getClientMaxBodySize() != -1)
            throw DuplicateDirectiveException();
        currServ.setClientMaxBodySize(convertClientMaxBodySize(token[1]));
    }
    else if (canBeParsedAsRootDirective(token))
    {
        if (currServ.getRoot() != "")
        {
            std::cerr << "[ERROR] [fillServerDirective] 🥸 Duplicate 'root' directive — only one allowed per location block." << "\n";
            throw SyntaxErrorException();
        }
        currServ.setRoot(token[1]);
    }
    else if (canBeParsedAsErrorPageDirective(token))
    {
        currServ.addNewErrorHtml(convertErrorCode(token[1]), token[2]);
    }
    else
    {
        std::cerr << "[ERROR] [fillServerDirective] 🥸 Unrecognized or unsupported directive inside 'server' block." << "\n";
        throw SyntaxErrorException();
    }
}

void    ParsingManager::fillListenDirective(ServerManager &server, std::vector<std::string> token)
{
    size_t          pos_from_start;
    size_t          pos_from_back;

    pos_from_start = token[1].find(':');
    if (pos_from_start != std::string::npos)
    {
        pos_from_back = token[1].rfind(':');
        if (pos_from_start != pos_from_back)
        {
            std::cerr << "[ERROR] [fillListenDirective] 🥸 Invalid listen format — multiple ':' found in \"" << token[1] << "\"" << "\n";
            throw SyntaxErrorException();
        }
        
        std::string ipStr = token[1].substr(0, pos_from_start);
        std::string portStr = token[1].substr(pos_from_start + 1);

        if (checkIpNumber(ipStr, server) == false)
        {
            std::cerr << "[ERROR] [fillListenDirective] 🥸 Invalid IP address: \"" << ipStr << "\"" << "\n";
            throw SyntaxErrorException();
        }
        if (isItAllDigit(portStr) == false || checkPortNumber(portStr) == false)
        {
            std::cerr << "[ERROR] [fillListenDirective] 🥸 Invalid port number: \"" << portStr << "\"" << "\n";
            throw SyntaxErrorException();
        }
        server.setServerIp(ipStr);
        server.addServerPort(atoi(portStr.c_str()));
    }
    else
    {
        if (isItAllDigit(token[1]) == false)
        {
            std::cerr << "[ERROR] [fillListenDirective] 🥸 Invalid listen directive: non-numeric port: \"" << token[1] << "\"" << "\n";
            throw SyntaxErrorException();
        }
        if (checkPortNumber(token[1]) == false)
        {
            std::cerr << "[ERROR] [fillListenDirective] 🥸 Invalid listen directive: non-numeric port: \"" << token[1] << "\"" << "\n";
            throw SyntaxErrorException();
        }

        server.addServerPort(atoi(token[1].c_str()));
    }
}

bool    ParsingManager::checkIpNumber(std::string ip, ServerManager &server)
{
    std::vector <std::string>   ipVector;

    if (server.getServerIp() != "")
    {
        if (server.getServerIp() == ip)
            return (true);
        if (ip == "localhost" && server.getServerIp() == "127.0.0.1")
            return (true);
        return (false);
    }
    else if (ip == "localhost")
        return (true);

    ipVector = tokenizer(ip, '.');
    if (ipVector.size() != 4)
        return (false);

    for (int i = 0; i < 4; i++)
    {
        if (isItAllDigit(ipVector[i]) == false)
            return (false);
        int octet = atoi(ipVector[i].c_str());
        if (!(0 <= octet && octet <= 255))
            return (false);
    }
    return (true);
}

bool     ParsingManager::checkPortNumber(std::string port)
{
    std::istringstream      iss(port);
    int                     tmp;

    if (!(iss >> tmp) || !iss.eof())
        return (false);

    if (1024 <= tmp && tmp <= 49151)
        return (true);
    return (false);
}

bool    ParsingManager::canBeParsedAsListenDirective(std::vector <std::string> token)
{
    return (token.size() == 2 && token[0] == "listen");
}

bool    ParsingManager::canBeParsedAsServerNamesDirective(std::vector <std::string> token)
{
    return (token.size() >= 2 && token[0] == "server_names");
}

/*
 █████          ███████      █████████    █████████   ███████████ █████    ███████    ██████   █████
░░███         ███░░░░░███   ███░░░░░███  ███░░░░░███ ░█░░░███░░░█░░███   ███░░░░░███ ░░██████ ░░███ 
 ░███        ███     ░░███ ███     ░░░  ░███    ░███ ░   ░███  ░  ░███  ███     ░░███ ░███░███ ░███ 
 ░███       ░███      ░███░███          ░███████████     ░███     ░███ ░███      ░███ ░███░░███░███ 
 ░███       ░███      ░███░███          ░███░░░░░███     ░███     ░███ ░███      ░███ ░███ ░░██████ 
 ░███      █░░███     ███ ░░███     ███ ░███    ░███     ░███     ░███ ░░███     ███  ░███  ░░█████ 
 ███████████ ░░░███████░   ░░█████████  █████   █████    █████    █████ ░░░███████░   █████  ░░█████
░░░░░░░░░░░    ░░░░░░░      ░░░░░░░░░  ░░░░░   ░░░░░    ░░░░░    ░░░░░    ░░░░░░░    ░░░░░    ░░░░░ 
                                                                                                    
*/

void    ParsingManager::handleLocationDirective(std::string line, std::vector <std::string> token)
{
    if (containsClosingBrace(line))
    {
        if (token.size() < 1 || token[0] != "}")
        {
            std::cerr << "[ERROR] [handleLocationDirective] 🥸 Closing brace '}' must appear as the first token to close location block." << "\n";
            throw SyntaxErrorException();
        }

        this->currState = SERVER_DIRECTIVE;

        std::string     newLine = line.substr(line.find('}') + 1);
        token.erase(token.begin());
        handleServerDirective(newLine, token);
    }
    else if (containsOpeningBrace(line))
    {
        this->currState = UNKNOWN_DIRECTIVE;
        std::cerr << "[ERROR] [handleLocationDirective] 🥸 Unexpected opening brace '{' found in location directive." << "\n";
        throw SyntaxErrorException();
    }
    else 
    {
        this->currState = LOCATION_DIRECTIVE;
        fillLocationDirective(token);
    }
}

void    ParsingManager::fillLocationDirective(std::vector <std::string> token)
{
    ServerManager       &currServ = this->myMainManager.getCurrentServerBlock();
    LocationManager     &currLoc = currServ.getCurrentLocationBlock();

    if (canBeParsedAsRootDirective(token))
        fillRootDirective(currLoc, token);
    else if (canBeParsedAsIndexDirective(token))
    {
        token.erase(token.begin());
        if (isVectorUnique(token) == false)
        {
            std::cerr << "[ERROR] [fillLocationDirective] 🥸 Location <" << currLoc.getRoute()
              << "> => Duplicate values in 'index' directive — entries must be unique." << "\n";
            throw SyntaxErrorException();
        }
        currLoc.setIndexHtml(token);
    }
    else if (canBeParsedAsClientMaxBodySizeDirective(token))
    {
        if (currLoc.getClientMaxBodySize() != -1)
            throw DuplicateDirectiveException();
        currLoc.setClientMaxBodySize(convertClientMaxBodySize(token[1]));
    }
    else if (canBeParsedAsMethodsDirective(token))
        fillMethodsDirective(currLoc, token[1]);
    else if (canBeParsedAsAutoIndexDirective(token))
        fillAutoIndexDirective(currLoc, token[1]);
    else if (canBeParsedAsUploadEnableDirective(token))
        fillUploadEnable(currLoc, token[1]);
    else if (canBeParsedAsUploadStoreDirective(token))
        fillUploadStore(currLoc, token);
    else if (canBeParsedAsRedirectDirective(token))
        fillReturnDirective(currLoc, token);
    else if (canBeParsedAsCgiInfoDirective(token))
        fillCgiDirective(currLoc, token);
    else if (canBeParsedAsErrorPageDirective(token))
    {
        currLoc.addNewErrorHtml(convertErrorCode(token[1]), token[2]);
    }
    else
    {
        std::cerr << "[ERROR] [fillLocationDirective] 🥸 Unknown or unsupported directive in location block." << "\n";
        std::cerr << "[SOURCE] : ";
        for (int i = 0; i < (int)token.size(); i++)
            std::cerr << token[i] << "-";
        std::cerr << "\n";
        throw SyntaxErrorException();
    }
}

void        ParsingManager::fillRootDirective(LocationManager &loc, std::vector <std::string> token)
{
    if (loc.getRoot() != "")
    {
        std::cerr << "[ERROR] [fillRootDirective] 🥸 Location <" << loc.getRoute()
                  << "> => Duplicate 'root' directive — only one allowed per location block." << "\n";
        throw SyntaxErrorException();
    }
    loc.setRoot(token[1]);
}

void        ParsingManager::fillMethodsDirective(LocationManager &loc, const std::string &line)
{
    if (loc.isMethodSet() == true)
    {
        std::cerr << "[ERROR] [fillMethodsDirective] 🥸 Location <" << loc.getRoute()
                  << "> => Duplicate 'methods' directive — already set." << "\n";
        throw SyntaxErrorException();
    }

    std::istringstream          iss(line);
    std::string                 tmp;
    std::vector <std::string>   methods;

    while (std::getline(iss, tmp, ','))
    {
        if (isItSupportedMethod(tmp) == false)
        {
            std::cerr << "[ERROR] [fillMethodsDirective] 🥸 Location <" << loc.getRoute()
                      << "> => Unsupported HTTP method: \"" << tmp << "\"" << "\n";
            throw SyntaxErrorException();
        }
        methods.push_back(tmp);
    }
    if (isVectorUnique(methods) == false)
    {
        std::cerr << "[ERROR] [fillMethodsDirective] 🥸 Location <" << loc.getRoute()
                  << "> => Duplicate HTTP methods in directive: \"" << line << "\"" << "\n";
        throw SyntaxErrorException();
    }
    loc.setMethods(methods);
}

bool        ParsingManager::isItSupportedMethod(std::string name)
{
    if (name == "GET" || name == "POST" || name == "DELETE")
        return (true);
    else
        return (false);
}

void        ParsingManager::fillAutoIndexDirective(LocationManager &loc, const std::string &line)
{
    if (loc.isAutoIndexSet() == true)
    {
        std::cerr << "[ERROR] [fillAutoIndexDirective] 🥸 Location <" << loc.getRoute()
                  << "> => Duplicate 'autoindex' directive — already defined." << "\n";
        throw SyntaxErrorException();
    }
    if (line == "on")
        loc.setAutoIndex(true);
    else if (line == "off")
        loc.setAutoIndex(false);
    else
    {
        std::cerr << "[ERROR] [fillAutoIndexDirective] 🥸 Location <" << loc.getRoute()
                  << "> => Invalid value for 'autoindex': \"" << line << "\"" << "\n";
        throw SyntaxErrorException();
    }
}

void        ParsingManager::fillUploadEnable(LocationManager &loc, const std::string &line)
{
    if (loc.isUploadEnableSet() == true)
    {
        throw SyntaxErrorException();
    }
    if (line == "on")
        loc.setUploadEnable(true);
    else if (line == "off")
        loc.setUploadEnable(false);
    else
    {
        std::cerr << "[ERROR] [fillUploadEnable] 🥸 Invalid value for 'upload_enable': \"" << line << "\" in location <" << loc.getRoute() << ">" << "\n";
        throw SyntaxErrorException();
    }
}

void        ParsingManager::fillUploadStore(LocationManager &loc, std::vector <std::string> token)
{
    if (loc.getUploadStore() != "")
    {
        std::cerr << "[ERROR] [fillUploadStore] 🥸 Location <" << loc.getRoute()
                  << "> => Duplicate 'upload_store' directive — already defined." << "\n";
        throw SyntaxErrorException();
    }
    
    loc.setUploadStore(token[1]);
}

void        ParsingManager::fillReturnDirective(LocationManager &loc, std::vector <std::string> token)
{
    if (loc.isRedirectSet() == true)
    {
        std::cerr << "[ERROR] [fillReturnDirective] 🥸 Location <" << loc.getRoute()
                  << "> => Duplicate 'return' directive — already defined." << "\n";
        throw SyntaxErrorException();
    }
    if (checkRedirectCode(token[1]) == false)
    {
        std::cerr << "[ERROR] [fillReturnDirective] 🥸 Location <" << loc.getRoute()
                  << "> => Invalid redirect status code: " << token[1] << "\n";
        throw SyntaxErrorException();
    }
    loc.setRedirect(token);
}

bool        ParsingManager::checkRedirectCode(std::string code)
{
    std::istringstream      iss(code);
    int                     res = 0;

    if (!(iss >> res) || !iss.eof())
        return (false);
    
    if (300 <= res && res <= 399)
        return (true);
    else
        return (false);
}

void        ParsingManager::fillCgiDirective(LocationManager &loc, std::vector <std::string> token)
{
    if (token[1] != ".py" && token[1] != ".php")
    {
        std::cerr << "[ERROR] [fillCgiDirective] 🥸 Location <" << loc.getRoute()
          << "> => Unsupported CGI extension: \"" << token[1] << "\"" << "\n";
        throw SyntaxErrorException();
    }
    if (loc.getCgiInfoWithKey(token[1]) != "")
    {
        std::cerr << "[ERROR] [fillCgiDirective] 🥸 Location <" << loc.getRoute()
          << "> => CGI for extension '" << token[1] << "' is already defined." << "\n";
        throw SyntaxErrorException();
    }
    if (access(token[2].c_str(), X_OK) != 0)
    {
        std::cerr << "[ERROR] [fillCgiDirective] 🥸 Location <" << loc.getRoute()
          << "> => CGI handler is not executable: \"" << token[2] << "\"" << "\n";
        throw SyntaxErrorException();
    }
    loc.addCgiInfo(token);
}

bool        ParsingManager::canBeParsedAsRootDirective(std::vector <std::string> token)
{
    return (token.size() == 2 && token[0] == "root");
}

bool        ParsingManager::canBeParsedAsMethodsDirective(std::vector <std::string> token)
{
    return (token.size() == 2 && token[0] == "methods");
}

bool        ParsingManager::canBeParsedAsAutoIndexDirective(std::vector <std::string> token)
{
    return (token.size() == 2 && token[0] == "autoindex");
}

bool        ParsingManager::canBeParsedAsUploadEnableDirective(std::vector <std::string> token)
{
    return (token.size() == 2 && token[0] == "upload_enable");
}

bool        ParsingManager::canBeParsedAsUploadStoreDirective(std::vector <std::string> token)
{
    return (token.size() == 2 && token[0] == "upload_store");
}

bool        ParsingManager::canBeParsedAsRedirectDirective(std::vector <std::string> token)
{
    return (token.size() == 3 && token[0] == "return");
}

bool        ParsingManager::canBeParsedAsCgiInfoDirective(std::vector <std::string> token)
{
    return (token.size() == 3 && token[0] == "cgi_info");
}

/*
 █████  █████ ███████████ █████ █████        █████████ 
░░███  ░░███ ░█░░░███░░░█░░███ ░░███        ███░░░░░███
 ░███   ░███ ░   ░███  ░  ░███  ░███       ░███    ░░░ 
 ░███   ░███     ░███     ░███  ░███       ░░█████████ 
 ░███   ░███     ░███     ░███  ░███        ░░░░░░░░███
 ░███   ░███     ░███     ░███  ░███      █ ███    ░███
 ░░████████      █████    █████ ███████████░░█████████ 
  ░░░░░░░░      ░░░░░    ░░░░░ ░░░░░░░░░░░  ░░░░░░░░░  
                                                       
*/

bool    ParsingManager::containsOpeningBrace(const std::string &line)
{
    return (line.find('{') != std::string::npos);
}

bool    ParsingManager::containsClosingBrace(const std::string &line)
{
    return (line.find('}') != std::string::npos);
}

bool     ParsingManager::isItAllDigit(std::string line)
{
    unsigned long   idx = 0;
    
    if (line.length() == 0)
        return (false);
    while (idx < line.length())
    {
        if ('0' <= line[idx] && line[idx] <= '9')
            idx++;
        else
            return (false);
    }
    return (true);
}

bool    ParsingManager::isVectorUnique(std::vector <std::string> token)
{
    std::set <std::string> tmp;

    for (int i = 0; i < (int)token.size(); i++)
    {
        if (!(tmp.insert(token[i]).second))
            return (false);
    }
    return (true);
}

std::vector<std::string>    ParsingManager::tokenizer(const std::string &line, char delimeter)
{
    std::vector <std::string>   res;
    std::istringstream          sst(line);
    std::string                 tmp;

    while (std::getline(sst, tmp, delimeter))
    {
        if (tmp.empty() == false)
            res.push_back(tmp);
    }

    return (res);
}

std::vector<std::string>    ParsingManager::tokenizer(const std::string &line)
{
    std::vector <std::string>   res;
    std::istringstream          sst(line);
    std::string                 tmp;

    while (sst >> tmp)
        res.push_back(tmp);

    return (res);
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
const char  *ParsingManager::FileDoesntExistException::what() const throw()
{
    return "file doesn't exists. 🤪\n";
}

const char  *ParsingManager::SyntaxErrorException::what() const throw()
{
    return "Configuration Syntax Error: This config file is breaking the rules. Fix it.\n";
}

const char  *ParsingManager::DuplicateDirectiveException::what() const throw()
{
    return "Configuration error: directive cannot be defined more than once.\n";
}
