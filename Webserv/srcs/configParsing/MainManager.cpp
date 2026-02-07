#include "MainManager.hpp"
#include <iostream>

MainManager::MainManager()
{
    this->clientMaxBodySize = -1;
}

MainManager::MainManager(const MainManager &other)
{
    *this = other;
}

MainManager::~MainManager()
{

}

MainManager     &MainManager::operator = (const MainManager &other)
{
    if (this != &other)
    {
        this->clientMaxBodySize = other.clientMaxBodySize;
        this->errorPage = other.errorPage;
        this->index = other.index;
        this->serverBlocks = other.serverBlocks;
    }
    return (*this);
}
        
void            MainManager::printParsedConfig()
{
    std::cout << "║▌│█║▌│ █║▌│█│║▌║ print parsed config ║▌│█║▌│ █║▌│█│║▌║" << "\n";

    std::cout << "📁 MainManager" << "\n";

    std::cout << "╰─── errorPage";
    if (errorPage.empty())
        std::cout << " not set";
    std::cout << "\n";
    for (std::map<int, std::string>::const_iterator it = errorPage.begin(); it != errorPage.end(); ++it) 
        std::cout << "      📄  [" << it->first << "] → " << it->second << "\n";

    std::cout << "╰─── index";
    if (index.empty())
        std::cout << " not set";
    std::cout << "\n";
    for (size_t i = 0; i < index.size(); ++i)
        std::cout << "      📄  [" << i << "] → " << index[i] << "\n";

    if (clientMaxBodySize == -1)
        std::cout << "╰─── clientMaxBodySize not set" << "\n";
    else
        std::cout << "╰─── clientMaxBodySize : " << clientMaxBodySize << "\n";

    for (size_t i = 0; i < serverBlocks.size(); ++i)
    {
        std::cout << "╰─── 📁 ServerManager [" << i << "]"<< "\n";
        serverBlocks[i].printParsedServer();
    }

    std::cout << "║▌│█║▌│ █║▌│█│║▌║ ║▌│█║▌│ █║▌│█│║▌║ ║▌│█║▌│ █║▌│█│║▌║▌│" << "\n";
}

void            MainManager::fillMissingConfigValues()
{
    for (size_t i = 0; i < serverBlocks.size(); ++i)
    {
        // server 멤버 변수 채우기
        if (serverBlocks[i].getServerIp() == "")
            serverBlocks[i].setServerIp("localhost");
        if (serverBlocks[i].getServerPortCnt() == 0)
            serverBlocks[i].addServerPort(8080);
        if (serverBlocks[i].getClientMaxBodySize() == -1 && this->clientMaxBodySize != -1)
            serverBlocks[i].setClientMaxBodySize(this->clientMaxBodySize);
        if (serverBlocks[i].getRoot() == "" && this->root != "")
            serverBlocks[i].setRoot(this->root);
        if (serverBlocks[i].getIndexVector().empty() && !this->index.empty())
            serverBlocks[i].setIndexHtml(this->index);
        serverBlocks[i].fillMissingErrorPages(this->errorPage);

        serverBlocks[i].fillMissingConfigValues();
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

long long                   MainManager::getClientMaxBodySize()
{
    return (this->clientMaxBodySize);
}

ServerManager               &MainManager::getServerBlock(int idx)
{
    if (idx < 0 || idx >= (int)serverBlocks.size())
        throw ServerBlockNotFoundException();
    return (this->serverBlocks[idx]);
}

ServerManager               &MainManager::getCurrentServerBlock()
{
    if (this->serverBlocks.empty())
    {
        std::cerr << "[Error] No server block defined. Cannot fetch current server block.\n";
        throw ServerBlockNotFoundException();
    }
    return (this->serverBlocks.back());
}

size_t                      MainManager::getServerBlockCount()
{
    return (this->serverBlocks.size());
}

std::map <int, std::string> MainManager::getErrorPage()
{
    return (this->errorPage);
}

std::string                 MainManager::getErrorPageWithKey(int key)
{
    std::map<int, std::string>::const_iterator it = errorPage.find(key);
    if (it == errorPage.end())
    {
        return ("");
    }
    return it->second;
}

std::vector <std::string>   MainManager::getIndex()
{
    return (this->index);
}

std::string                 MainManager::getIndexWithIdx(int idx)
{
    if (idx < 0 || (int)this->index.size() <= idx)
    {
        std::cerr <<"[ERROR] [getIndexWithIdx] idx is out of range" << "\n";
        throw std::exception();
    }
    return ((this->index)[idx]);
}

std::string                 MainManager::getRoot()
{
    return (this->root);
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

void            MainManager::addNewServer()
{
    ServerManager   tmp;

    this->serverBlocks.push_back(tmp);
}

void            MainManager::addNewErrorHtml(int errCode, std::string page)
{
    this->errorPage[errCode] = page;
}

void            MainManager::setIndexHtml(std::vector <std::string> token)
{
    if (!(this->index.empty()))
        throw ConflictingIndexDirectiveException();
    
    index = token;
}

void            MainManager::setClientMaxBodySize(long long _size)
{
    this->clientMaxBodySize = _size;
}

void            MainManager::setRoot(std::string _root)
{
    this->root = _root;
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

const char      *MainManager::ConflictingIndexDirectiveException::what() const throw()
{
    return "Conflicting 'index' directive found in the same scope.\n";
} 

const char      *MainManager::ServerBlockNotFoundException::what() const throw()
{
    return "[ERROR] Attempted to access a server block that does not exist.\n";
} 
