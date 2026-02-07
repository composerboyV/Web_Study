#include "LocationManager.hpp"
#include <iostream>
#include <cstdlib>


LocationManager::LocationManager()
{
    this->equalModifier = false;
    this->hasEqualModifier = false;
    this->uploadEnable = false;
    this->hasUploadEnable = false;
    this->AutoIndex = false;
    this->HasAutoIndex = false;
    this->clientMaxBodySize = -1;
}

LocationManager::LocationManager(std::string _route) : route(_route)
{
    this->equalModifier = false;
    this->hasEqualModifier = false;
    this->uploadEnable = false;
    this->hasUploadEnable = false;
    this->AutoIndex = false;
    this->HasAutoIndex = false;
    this->clientMaxBodySize = -1;
}

LocationManager::LocationManager(const LocationManager &other)
{
    *this= other;
}

LocationManager &LocationManager::operator = (const LocationManager &other)
{
    if (this != &other)
    {
        this->route = other.route;
        this->equalModifier = other.equalModifier;
        this->hasEqualModifier = other.hasEqualModifier;
        this->methods = other.methods;
        this->root = other.root;
        this->index = other.index;
        this->uploadEnable = other.uploadEnable;
        this->hasUploadEnable = other.hasUploadEnable;
        this->uploadStore = other.uploadStore;
        this->cgiInfo = other.cgiInfo;
        this->redirect = other.redirect;
        this->AutoIndex = other.AutoIndex;
        this->HasAutoIndex = other.HasAutoIndex;
        this->clientMaxBodySize = other.clientMaxBodySize;
        this->errorPage = other.errorPage;
    }
    return (*this);
}

LocationManager::~LocationManager()
{
}

void                LocationManager::printParsedLocation()
{
    std::cout << "              ╰─── route : " << route << "\n";
    
    if (hasEqualModifier == true && equalModifier == true)
        std::cout << "              ╰─── equalmodifier : set" << "\n";
    else
        std::cout << "              ╰─── equalmodifier : not set" << "\n";

    std::cout << "              ╰─── methods";
    if (methods.empty())
        std::cout << " not set";
    std::cout << "\n";
    for (size_t i = 0; i < methods.size(); ++i)
        std::cout << "                  ╰─── [" << i << "] :" << methods[i] << "\n";
    
    if (root != "")
        std::cout << "              ╰─── root : " << root << "\n";
    else
        std::cout << "              ╰─── [X] root not set " << "\n";

    std::cout << "              ╰─── index";
    if (index.empty())
        std::cout << " not set";
    std::cout << "\n";
    for (size_t i = 0; i < index.size(); ++i)
        std::cout << "            📄  [" << i << "] → " << index[i] << "\n";

    if (hasUploadEnable == true)
        std::cout << "              ╰─── upload enable : " << uploadEnable << "\n";
    else
        std::cout << "              ╰─── [X] upload enable not set " << "\n";

    if (uploadStore != "")
        std::cout << "              ╰─── upload store : " << uploadStore << "\n";
    else
        std::cout << "              ╰─── [X] upload store not set " << "\n";

    std::cout << "              ╰─── CGI INFO";
    if (cgiInfo.empty())
        std::cout << " not set";
    std::cout << "\n";
    for (std::map<std::string, std::string>::const_iterator it = cgiInfo.begin(); it != cgiInfo.end(); ++it)
        std::cout << "                ╰─── [" << it->first << "] → " << it->second << "\n";

    std::cout << "              ╰─── REDIRECT ";
    if (redirect.empty())
        std::cout << " not set";
    std::cout << "\n";
    for (std::map<int, std::string>::const_iterator it = redirect.begin(); it != redirect.end(); ++it)
        std::cout << "                ╰─── [" << it->first << "] → " << it->second << "\n";
    
    std::cout << "              ╰─── ERROR PAGE ";
    if (errorPage.empty())
        std::cout << " not set";
    std::cout << "\n";
    for (std::map<int, std::string>::const_iterator it = errorPage.begin(); it != errorPage.end(); ++it)
        std::cout << "                ╰─── [" << it->first << "] → " << it->second << "\n";

    if (HasAutoIndex == true && AutoIndex == true)
        std::cout << "              ╰─── auto index : is set" << "\n";
    else
        std::cout << "              ╰─── [X] auto index not set " << "\n";

    if (clientMaxBodySize == -1)
        std::cout << "              ╰─── clientMaxBodySize not set" << "\n" << "\n";
    else
        std::cout << "              ╰─── clientMaxBodySize : " << clientMaxBodySize << "\n" << "\n";
}


void                LocationManager::printErrorPage()
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
   █████████  ██████████ ███████████ ███████████ ██████████ ███████████  
  ███░░░░░███░░███░░░░░█░█░░░███░░░█░█░░░███░░░█░░███░░░░░█░░███░░░░░███ 
 ███     ░░░  ░███  █ ░ ░   ░███  ░ ░   ░███  ░  ░███  █ ░  ░███    ░███ 
░███          ░██████       ░███        ░███     ░██████    ░██████████  
░███    █████ ░███░░█       ░███        ░███     ░███░░█    ░███░░░░░███ 
░░███  ░░███  ░███ ░   █    ░███        ░███     ░███ ░   █ ░███    ░███ 
 ░░█████████  ██████████    █████       █████    ██████████ █████   █████
  ░░░░░░░░░  ░░░░░░░░░░    ░░░░░       ░░░░░    ░░░░░░░░░░ ░░░░░   ░░░░░ 
*/

std::string                 LocationManager::getRoute()
{
    return (this->route);
}

bool                        LocationManager::getEqualModifier()
{
    return (this->equalModifier);
}

bool                        LocationManager::getHasEqualModifier()
{
    return (this->hasEqualModifier);
}

std::vector <std::string>	LocationManager::getMethods()
{
	return (this->methods);
}

std::string                 LocationManager::getMethodsIdx(int idx)
{
    if (idx < 0 || (int)this->methods.size() <= idx)
        throw OutofRangeException();
    return (this->methods[idx]);
}

std::string                 &LocationManager::getRoot()
{
    return (this->root);
}

std::string                 LocationManager::getIndexPageWithIdx(int idx)
{
    if (idx < 0 || (int)this->index.size() <= idx)
        throw OutofRangeException();
    return (this->index[idx]);
}

std::vector <std::string>   LocationManager::getIndex()
{
    return (this->index);
}

bool                        LocationManager::getHasUploadEnable()
{
    return (this->hasUploadEnable);
}

bool                        LocationManager::getUploadEnable()
{
    return (this->uploadEnable);
}

std::string                 LocationManager::getUploadStore()
{
    return (this->uploadStore);
}

std::string                 LocationManager::getCgiInfoWithKey(std::string val)
{
    std::map <std::string, std::string>::iterator   it = this->cgiInfo.find(val);

    if (it == this->cgiInfo.end())
        return ("");
    else
        return (it->second);
}

std::string                 LocationManager::getRedirectPageWithKey(int val)
{
    std::map <int, std::string>::iterator   it = this->redirect.find(val);

    if (it == this->redirect.end())
        return ("");
    else
        return (it->second);
}

int							LocationManager::getRedirectStatusCode()
{
	std::map <int, std::string>::iterator   it = this->redirect.begin();

	return (it->first);	
}


bool                        LocationManager::getAutoIndex()
{
    return (this->AutoIndex);
}

bool                        LocationManager::getHasAutoIndex()
{
    return (this->HasAutoIndex);
}

long long                   LocationManager::getClientMaxBodySize()
{
    return (this->clientMaxBodySize);
}

std::map <int, std::string> LocationManager::getErrorPage()
{
    return (this->errorPage);
}

std::string                 LocationManager::getErrorPageWithKey(int key)
{
    std::map <int, std::string>::const_iterator it = this->errorPage.find(key);
    if (it == this->errorPage.end())
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

void                LocationManager::setRoute(std::string _route)
{
    this->route = _route;
}

void                LocationManager::setEqualModifier(bool _val)
{
    this->equalModifier = _val;
    setHasEqualModifier(true);
}

void                LocationManager::setHasEqualModifier(bool _val)
{
    this->hasEqualModifier = _val;
}

void                LocationManager::setRoot(std::string _root)
{
    this->root = _root;
}

void                LocationManager::setIndexHtml(std::vector <std::string> token)
{
    if (!(this->index.empty()))
        throw ConflictingDirectiveException();

    this->index = token;
}

void                LocationManager::setMethods(std::vector <std::string> token)
{
    if (!(this->methods.empty()))
        throw ConflictingDirectiveException();

    this->methods = token;
}

void                LocationManager::setHasUploadEnable(bool _val)
{
    this->hasUploadEnable = _val;
}

void                LocationManager::setUploadEnable(bool _val)
{
    this->uploadEnable = _val;
    setHasUploadEnable(true);
}

void                LocationManager::setUploadStore(std::string _val)
{
    this->uploadStore = _val;
}

void                LocationManager::setRedirect(std::vector <std::string> token)
{
    int     code = atoi(token[1].c_str());

    this->redirect[code] = token[2];
}

void                LocationManager::setAutoIndex(bool _val)
{
    this->AutoIndex = _val;
    setHasAutoIndex(true);
}

void                LocationManager::setHasAutoIndex(bool _val)
{
    this->HasAutoIndex = _val;
}

void                LocationManager::addCgiInfo(std::vector <std::string> token)
{
    this->cgiInfo[token[1]] = token[2];
}

void                LocationManager::setClientMaxBodySize(long long _val)
{
    this->clientMaxBodySize = _val;
}

void                LocationManager::setErrorPage(std::map <int, std::string> _errorPage)
{
    this->errorPage = _errorPage;
}

void                LocationManager::addNewErrorHtml(int errCode, std::string page)
{
    this->errorPage[errCode] = page;
}

void                LocationManager::fillMissingErrorPages(std::map <int, std::string> parents)
{
    std::map <int, std::string>::const_iterator     it;

    for (it = parents.begin(); it != parents.end(); it++)
    {
        if (this->errorPage.find(it->first) == this->errorPage.end())
            addNewErrorHtml(it->first, it->second);
    }
}



/*
   █████████  █████   █████ ██████████   █████████  █████   ████ ██████████ ███████████  
  ███░░░░░███░░███   ░░███ ░░███░░░░░█  ███░░░░░███░░███   ███░ ░░███░░░░░█░░███░░░░░███ 
 ███     ░░░  ░███    ░███  ░███  █ ░  ███     ░░░  ░███  ███    ░███  █ ░  ░███    ░███ 
░███          ░███████████  ░██████   ░███          ░███████     ░██████    ░██████████  
░███          ░███░░░░░███  ░███░░█   ░███          ░███░░███    ░███░░█    ░███░░░░░███ 
░░███     ███ ░███    ░███  ░███ ░   █░░███     ███ ░███ ░░███   ░███ ░   █ ░███    ░███ 
 ░░█████████  █████   █████ ██████████ ░░█████████  █████ ░░████ ██████████ █████   █████
  ░░░░░░░░░  ░░░░░   ░░░░░ ░░░░░░░░░░   ░░░░░░░░░  ░░░░░   ░░░░ ░░░░░░░░░░ ░░░░░   ░░░░░ 
*/

bool                LocationManager::isMethodSet()
{
    if (this->methods.empty())
        return (false);
    else
        return (true);
}

bool                LocationManager::isAutoIndexSet()
{
    return (HasAutoIndex);
}

bool                LocationManager::isUploadEnableSet()
{
    return (hasUploadEnable);
}

bool                LocationManager::isRedirectSet()
{
    if (this->redirect.empty())
        return (false);
    else
        return (true);
}

bool				LocationManager::isExistingCGI(std::string &extension)
{
	if (this->cgiInfo.count(extension) == 0)
		return (false);
	else
		return (true);
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

const char  *LocationManager::ConflictingDirectiveException::what() const throw()
{
    return "Conflicting directive line: only one directive line allowed per server block.\n";
}

const char  *LocationManager::OutofRangeException::what() const throw()
{
    return "The thing you asked for is out of range\n";
}