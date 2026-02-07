#include "ServerManager.hpp"

#include <iostream>
#include <algorithm>

ServerManager::ServerManager()
{
    this->clientMaxBodySize = -1 ;
}

ServerManager::ServerManager(const ServerManager &other)
{
    *this = other;
}

ServerManager   &ServerManager::operator = (const ServerManager &other)
{
    if (this != &other)
    {
        this->locationBlocks = other.locationBlocks;
        this->serverName = other.serverName;
        this->servPort = other.servPort;
        this->servIp = other.servIp;
        this->index = other.index;
        this->clientMaxBodySize = other.clientMaxBodySize;
    }
    return (*this);
}

ServerManager::~ServerManager()
{
}

void            ServerManager::printParsedServer()
{
    std::cout << "      ╰─── serverName";
    if (serverName.empty())
        std:: cout << " not set";
    std::cout << "\n";
    for (size_t i = 0; i < serverName.size(); ++i)
        std::cout << "           [" << i << "] " << serverName[i] << "\n";
    
    if (!(servIp).empty())
        std::cout << "      ╰─── server IP : " << servIp << "\n";
    else
        std::cout << "      ╰─── server IP not set" << "\n";

    std::cout << "      ╰─── server Port";
    if (servPort.empty())
        std:: cout << " not set";
    std::cout << "\n";
    for (size_t i = 0; i < servPort.size(); ++i)
        std::cout << "           [" << i << "] " << servPort[i] << "\n";

    std::cout << "      ╰─── index";
    if (index.empty())
        std::cout << " not set";
    std::cout << "\n";
    for (size_t i = 0; i < index.size(); ++i)
        std::cout << "            📄  [" << i << "] → " << index[i] << "\n";
    
    if (clientMaxBodySize == -1)
        std::cout << "      ╰───  clientMaxBodySize not set"<< "\n";
    else
        std::cout << "      ╰───  clientMaxBodySize : " << clientMaxBodySize << "\n";

    for (size_t i = 0; i < locationBlocks.size(); ++i)
    {
        std::cout << "      ╰─── 📁 LocationManager [" << i << "]"<< "\n";
        locationBlocks[i].printParsedLocation();
    }

}

void        ServerManager::fillMissingConfigValues()
{
    for (size_t i = 0; i < locationBlocks.size(); ++i)
    {
        if (locationBlocks[i].getRoot() == "" && this->root != "")
            locationBlocks[i].setRoot(this->root);
        if (locationBlocks[i].getIndex().empty() && !this->index.empty())
            locationBlocks[i].setIndexHtml(this->index);
        if (locationBlocks[i].getClientMaxBodySize() == -1 && this->clientMaxBodySize != -1)
            locationBlocks[i].setClientMaxBodySize(this->clientMaxBodySize);
        locationBlocks[i].fillMissingErrorPages(this->errorPage);
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

std::string     ServerManager::getServerIp()
{
    return (this->servIp);
}

LocationManager &ServerManager::getLocationBlock(int idx)
{
    if (idx < 0 || idx >= (int)this->locationBlocks.size())
        throw LocationBlockNotFoundException();
    return (this->locationBlocks[idx]);
}

LocationManager &ServerManager::getCurrentLocationBlock()
{
    if (this->locationBlocks.empty())
    {
        std::cerr << "[ERROR] No location block defined. Cannot fetch location server block.\n";
        throw LocationBlockNotFoundException();
    }
    return (this->locationBlocks.back());
}

void            ServerManager::addNewLocation(std::string _route, bool _equalModifier)
{
    int         locationSize = this->locationBlocks.size();

    for (int i = 0; i < locationSize; i ++)
    {
        if (this->locationBlocks[i].getRoute() == _route && this->locationBlocks[i].getEqualModifier() == _equalModifier)
            throw DuplicateValueException();
    }

    LocationManager   tmp(_route);

    tmp.setEqualModifier(_equalModifier);
    this->locationBlocks.push_back(tmp);
}

long long       ServerManager::getClientMaxBodySize()
{
    return (this->clientMaxBodySize);
}

std::string                 ServerManager::getServerNameWithIdx(int idx) 
{
    if (idx < 0 || (int)this->serverName.size() <= idx)
    {
        std::cerr <<"[ERROR] [getServerNameWithIdx] idx is out of range" << "\n";
        throw std::exception();
    }
    return (this->serverName[idx]);
} 

std::vector <std::string>   ServerManager::getServerNameVector()  
{
    return (this->serverName);
}

std::vector <int>           ServerManager::getServerPortVector() 
{
    return (this->servPort);
}

size_t                      ServerManager::getServerPortCnt()
{
    return (this->servPort.size());
}

int                         ServerManager::getServerPortWithIdx(int idx) 
{
    if (idx < 0 || (int)this->servPort.size() <= idx)
    {
        std::cerr <<"[ERROR] [getIndexWithIdx] idx is out of range" << "\n";
        throw std::exception();
    }
    return (this->servPort[idx]);
}

std::string                 ServerManager::getIndexWithIdx(int idx)
{
    if (idx < 0 || (int)this->index.size() <= idx)
    {
        std::cerr <<"[ERROR] [getIndexWithIdx] idx is out of range" << "\n";
        throw std::exception();
    }
    return ((this->index)[idx]);
}

std::vector <std::string>   ServerManager::getIndexVector() 
{
    return (this->index);
}
std::string                 ServerManager::getRoot()
{
    return (this->root);
}

std::vector <LocationManager>	&ServerManager::getLocationBlocks()
{
	return (this->locationBlocks);
}

std::map <int, std::string>     ServerManager::getErrorPage()
{
    return (this->errorPage);
}

std::string                     ServerManager::getErrorPageWithKey(int key)
{
    std::map <int, std::string>::const_iterator it = errorPage.find(key);
    if (it == errorPage.end())
    {
        return ("");
    }
    return it->second;
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


void    ServerManager::setServerIp(std::string _ip)
{
    if (_ip == "localhost")
        this->servIp = "127.0.0.1";
    else
        this->servIp = _ip;
}

void    ServerManager::setClientMaxBodySize(long long val)
{
    this->clientMaxBodySize = val;
}

void    ServerManager::setIndexHtml(std::vector <std::string> token)
{
    if (!(this->index.empty()))
        throw ConflictingDirectiveException();
    
    index = token;
}

void    ServerManager::setServerName(std::vector <std::string> token)
{
    if (!(this->serverName.empty()))
        throw ConflictingDirectiveException();
    
    serverName = token;
}

void    ServerManager::addServerPort(int _port)
{
    if (std::find(this->servPort.begin(), this->servPort.end(), _port) \
            != this->servPort.end())
        throw DuplicateValueException();
    this->servPort.push_back(_port);
}

void    ServerManager::setRoot(std::string _root)
{
    this->root = _root;
}

void    ServerManager::setErrorPage(std::map <int, std::string> _errorPage)
{
    this->errorPage = _errorPage;
}

void    ServerManager::addNewErrorHtml(int errCode, std::string page)
{
    this->errorPage[errCode] = page;
}

void    ServerManager::fillMissingErrorPages(std::map <int, std::string> parents)
{
    std::map <int, std::string>::const_iterator     it;

    for (it = parents.begin(); it != parents.end(); it++)
    {
        if (this->errorPage.find(it->first) == this->errorPage.end())
            addNewErrorHtml(it->first, it->second);
    }
}

void    ServerManager::printErrorPage()
{
    if (this->errorPage.empty())
    {
        std::cout << "[DEBUG] 🤔 error page not set\n";
        return ;
    }
    std::cout << "-----------------------\n";
    for (std::map <int, std::string>::const_iterator it = this->errorPage.begin(); it != this->errorPage.end(); ++it)
    {
        std::cout << "[DEBUG] 🤔 " << it->first << " : " << it->second << "\n";
    }
    std::cout << "-----------------------\n";
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

const char  *ServerManager::ConflictingDirectiveException::what() const throw()
{
    return "Conflicting directive line: only one directive line allowed per server block.\n";
}

const char  *ServerManager::DuplicateValueException::what() const throw()
{
    return "Duplicate value detected: this value is already assigned to the server block.\n";
}

const char  *ServerManager::LocationBlockNotFoundException::what() const throw()
{
    return "[ERROR] Attempted to access a location block that does not exist.\n";
}