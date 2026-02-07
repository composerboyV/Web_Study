#include "RequestMsg.hpp"
#include "../configParsing/ServerManager.hpp"
#include <iostream>

RequestMsg::RequestMsg()
{
    this->statusCode = -1;
    this->isThischunked = false;
    this->isThisError = false;
    this->methodVal = DEFAULT;
    this->connectionVal = NOTHING_YET;
	this->myLocationBlock = NULL;
}

RequestMsg::RequestMsg(
    std::string _rawStartLine, 
    std::string _rawHeader,
    std::string _rawBody,
    int         _statusCode,
    std::string _statusLine, 
    long long   _contentLength,
    bool        _isThisChunked,
    bool        _isThisError
)
: rawStartLine(_rawStartLine), rawHeader(_rawHeader), rawBody(_rawBody),
statusCode(_statusCode), statusLine(_statusLine), contentLength(_contentLength),
isThischunked(_isThisChunked), isThisError(_isThisError)
{
    this->methodVal = DEFAULT;
    this->connectionVal = NOTHING_YET;
	this->myLocationBlock = NULL;
}

RequestMsg::RequestMsg(const RequestMsg &other)
: rawStartLine(other.rawStartLine), rawHeader(other.rawHeader), rawBody(other.rawBody),
statusCode(other.statusCode), statusLine(other.statusLine), contentLength(other.contentLength),
isThischunked(other.isThischunked), isThisError(other.isThisError)
{
    this->methodVal = other.methodVal;
    this->connectionVal = other.connectionVal;
}

RequestMsg      &RequestMsg::operator = (const RequestMsg &other)
{
    if (this != &other)
    {
        this->rawStartLine = other.rawStartLine;
        this->rawHeader = other.rawHeader;
        this->rawBody = other.rawBody;
        this->statusCode = other.statusCode;
        this->statusLine = other.statusLine;
        this->contentLength = other.contentLength;
        this->isThischunked = other.isThischunked;
        this->isThisError = other.isThisError;
    }
    return (*this);
}

RequestMsg::~RequestMsg()
{
    
}

/*
 ██████   ██████   █████████   █████ ██████   █████
░░██████ ██████   ███░░░░░███ ░░███ ░░██████ ░░███ 
 ░███░█████░███  ░███    ░███  ░███  ░███░███ ░███ 
 ░███░░███ ░███  ░███████████  ░███  ░███░░███░███ 
 ░███ ░░░  ░███  ░███░░░░░███  ░███  ░███ ░░██████ 
 ░███      ░███  ░███    ░███  ░███  ░███  ░░█████ 
 █████     █████ █████   █████ █████ █████  ░░█████
░░░░░     ░░░░░ ░░░░░   ░░░░░ ░░░░░ ░░░░░    ░░░░░ 
*/

void            RequestMsg::parseHttpRequest(std::vector<ServerManager *> &servBlocks)
{
	parseRawStartLine();
	parseRawHeader();

	routeLocationBlock(servBlocks);
	validateRequest();
}

void			RequestMsg::validateRequest()
{
	if (checkBodySize() == false)
		return ;
	checkMethod();
}

bool			RequestMsg::checkBodySize()
{
	long long		maxSize = this->myLocationBlock->getClientMaxBodySize();

	if (maxSize < (long long)this->rawBody.length())
	{
		std::cout << "[DEBUG] body size 한계 돌파\n";
		setErrorInfo(413, "Payload Too Large");
		return (false);
	}
	return (true);
}

void		RequestMsg::checkMethod()
{
	std::vector <std::string>	allowedMethods = this->myLocationBlock->getMethods();

	if (allowedMethods.size() == 0)
		return ;
	for (size_t i = 0; i < allowedMethods.size(); ++i)
	{
		if (allowedMethods[i] == this->method)
			return ;
	}
	setErrorInfo(405, "Method Not Allowed");
}


void			RequestMsg::parseRawStartLine()
{
	size_t		start, end;
	size_t		len = this->rawStartLine.size();

	if (this->rawStartLine.empty() == true)
	{
		setStatusInfo(400, "BAD REQUEST");
		return ;
	}
	
	end = findWhiteSpace(this->rawStartLine, len, 0);
	this->method = std::string(this->rawStartLine.begin(), this->rawStartLine.begin() + end);
	std::cout << "[INFO] 🥊 parsed method: " << this->method << "\n";
	assignHttpMethodEnum();
	if (this->methodVal == UNIMPLEMENTED)
		return ;
	
	start = skipWhiteSpace(this->rawStartLine, len, end);
	end = findWhiteSpace(this->rawStartLine, len, start);
	this->requestTarget = std::string(this->rawStartLine.begin() + start, this->rawStartLine.begin() + end);
	std::cout << "[INFO] 🥊 parsed requestTarget: " << this->requestTarget << "\n";
	
	start = skipWhiteSpace(this->rawStartLine, len, end);
	this->httpVersion = this->rawStartLine.substr(start);
	std::cout << "[INFO] 🥊 parsed HTTP VERSION: " << this->httpVersion << "\n";
	if (this->httpVersion != "HTTP/1.1")
	{
		setStatusInfo(505, "HTTP Version Not Supported");
		return ;
	}
}

void			RequestMsg::assignHttpMethodEnum()
{
	if (this->method == "GET")
		this->methodVal = GET;
	else if (this->method == "POST")
		this->methodVal = POST;
	else if (this->method == "DELETE")
		this->methodVal = DELETE;
	else
	{
		setStatusInfo(501, "method [" + this->method + "] unimplemented");
		return ;
	}
}


void	RequestMsg::parseRawHeader()
{
	size_t			currOffset = 0;
	std::string		fieldName, fieldValue;

	while (currOffset < this->rawHeader.size())
	{
		getFieldNameAndValue(currOffset, fieldName, fieldValue);
		if (isValidName(fieldName) == false)
			return ;
		if (isValidValue(fieldValue) == false)
			return ;
		if (isExistingHeader(fieldName) == true)
		{
			std::cout << "[DEBUG] isExistingHeader\n";
			setErrorInfo(400, "bad request");
			return ;
		}
		else
		{
			this->header[fieldName] = fieldValue;
		}
	}
	parseHost();
	parseContentType();
	parseConnection();
}

void RequestMsg::getFieldNameAndValue(size_t &currOffset, std::string &fieldName, std::string &fieldValue)
{
	std::string	line;
	size_t 		colonOffset;

	
	line = getOneLineByCRLF(currOffset);
	colonOffset = line.find(":");
	if (colonOffset == std::string::npos)
	{
		std::cout << "[DEBUG] no colon in header line\n";
		setErrorInfo(400, "bad request");
		return ;
	}
	fieldName = line.substr(0, colonOffset);
	fieldValue = line.substr(colonOffset + 1); 
}

std::string	RequestMsg::getOneLineByCRLF(size_t &currOffset)
{

	std::string		line;
	size_t			CRLFStartOffset;

	
	CRLFStartOffset = this->rawHeader.find("\r\n", currOffset);			
	if (CRLFStartOffset == std::string::npos)
	{
		line = this->rawHeader.substr(currOffset);
		currOffset = this->rawHeader.size();
	}
	else
	{		
		line = this->rawHeader.substr(currOffset, CRLFStartOffset - currOffset);
		currOffset = CRLFStartOffset + 2;
	}
	return (line);
}

bool	RequestMsg::isValidName(std::string &name)
{
	if (name.empty() == true)
	{	
		std::cout << "[DEBUG] no name\n";
		setErrorInfo(400, "bad request");
		return (false);
	}
	for (size_t i = 0; i < name.size(); ++i)
	{
		if (name[i] == ' ' || name[i] == '\t')
		{
			std::cout << "[DEBUG] white space in name\n";
			setErrorInfo(400, "bad request");
			return (false);
		}
	}
	return (true);
}

bool	RequestMsg::isValidValue(std::string &value)
{
	size_t	start = 0;
	size_t	end = value.size() - 1;

	while (start < end &&  isThisWhiteSpace(value, start) == true)
		start++;
	while (start < end && isThisWhiteSpace(value, end) == true)
		end--;
	value = std::string(value.begin() + start, value.begin() + end + 1);
	if (value.empty() == true)
	{	
		std::cout << "[DEBUG] no val in header\n";
		setErrorInfo(400, "bad request");
		return (false);
	}
	return (true);
}

bool	RequestMsg::isExistingHeader(std::string name)
{
	if (this->header.find(name) != this->header.end())
		return true;
	else
		return false;
}

void	RequestMsg::parseHost()
{
	if (isExistingHeader("Host") == false)
	{
		std::cout << "[DEBUG] no host header\n";
		setErrorInfo(400, "bad request");
		return ;
	}
	else
	{
		std::string value = this->header["Host"];

		if (hasWhiteSpace(value) == true)
		{
			setErrorInfo(400, "bad reqeust");
			return ;
		}
		this->hostVal = value;
	}
}

void	RequestMsg::parseContentType()
{
	if (isExistingHeader("Content-Type") == true)
	{
		std::string value = this->header["Content-Type"];
		this->contentTypeVal = value;
	}
}

void	RequestMsg::parseConnection()
{
	if (isExistingHeader("Connection") == false)
	{
		this->connectionVal = KEEP_ALIVE;
		return ;
	}
	else
	{
		std::string	value = this->header["Connection"];

		if (hasWhiteSpace(value) == true)
		{
			setErrorInfo(400, "bad reqeust");
			return ;
		}
		if (value == "keep-alive" || value == "Keep-Alive" || value == "Keep-alive" || value == "KEEP-ALIVE")
			this->connectionVal = KEEP_ALIVE;
		else if (value == "close" || value == "Close")
			this->connectionVal = CLOSE;
		else
		{
			std::cout << "[DEBUG] connection value different(only close, and keep alive allowed\n";
			setErrorInfo(501, "not implemented");
		}
	}
}


/*
 █████         ███████     █████████  █████████ ████████████████  ███████  ██████   █████
░░███        ███░░░░░███  ███░░░░░██████░░░░░██░█░░░███░░░░░███ ███░░░░░██░░██████ ░░███ 
 ░███       ███     ░░██████     ░░░░███    ░██░   ░███  ░ ░██████     ░░██░███░███ ░███ 
 ░███      ░███      ░██░███        ░███████████   ░███    ░██░███      ░██░███░░███░███ 
 ░███      ░███      ░██░███        ░███░░░░░███   ░███    ░██░███      ░██░███ ░░██████ 
 ░███      ░░███     ███░░███     ██░███    ░███   ░███    ░██░░███     ███░███  ░░█████ 
 ███████████░░░███████░  ░░██████████████   █████  █████   ████░░░███████░ █████  ░░█████
░░░░░░░░░░░   ░░░░░░░     ░░░░░░░░░░░░░░   ░░░░░  ░░░░░   ░░░░░  ░░░░░░░  ░░░░░    ░░░░░ 
 ███████████    ███████  █████  █████████████████████████   █████ █████████              
░░███░░░░░███ ███░░░░░██░░███  ░░██░█░░░███░░░░░██░░██████ ░░███ ███░░░░░███             
 ░███    ░██████     ░░██░███   ░██░   ░███  ░ ░███░███░███ ░██████     ░░░              
 ░██████████░███      ░██░███   ░███   ░███    ░███░███░░███░██░███                      
 ░███░░░░░██░███      ░██░███   ░███   ░███    ░███░███ ░░█████░███    █████             
 ░███    ░██░░███     ███░███   ░███   ░███    ░███░███  ░░████░░███  ░░███              
 █████   ████░░░███████░ ░░████████    █████   █████████  ░░████░░█████████              
░░░░░   ░░░░░  ░░░░░░░    ░░░░░░░░    ░░░░░   ░░░░░░░░░    ░░░░░ ░░░░░░░░░               
*/

void	RequestMsg::routeLocationBlock(std::vector<ServerManager *> &servBlocks)
{
	ServerManager		*matchedServPtr;
	LocationManager		*matchedLocationPtr;
	
	matchedServPtr = matchServBlock(servBlocks);
	std::cout << "[DEBUG] matched Server Pointer : " << matchedServPtr->getServerNameWithIdx(0) << "\n";
	matchedLocationPtr = matchLocationBlock(matchedServPtr);
	
	if (matchedLocationPtr == NULL)
	{
		setStatusInfo(404, "Not Found");
		this->locationFound = false;
	}
	else
	{
		this->myLocationBlock = matchedLocationPtr;
		this->locationFound = true;
	}
}

ServerManager	*RequestMsg::matchServBlock(std::vector<ServerManager *> &servBlocks)
{	
	int				servBlocksCnt = servBlocks.size();
	ServerManager	*servBlockPtr = NULL;
	
	if (servBlocksCnt == 1)
	{
		return servBlocks.front();
	}
	else if (servBlocksCnt > 1)
	{
		servBlockPtr = matchServBlockByHost(servBlocks, this->hostVal);
		if (servBlockPtr == NULL)
		{
			std::cout << "[DEBUG] matching serverblock by host failed\n";
			return servBlocks.front();
		}
	}
	return servBlockPtr;		
}

ServerManager	*RequestMsg::matchServBlockByHost(std::vector<ServerManager *> &servBlocks, std::string hostVal)
{
	for (size_t i = 0; i < servBlocks.size(); ++i)
	{
		ServerManager				*currServBlockPtr = servBlocks[i];
		std::vector<std::string>	servNames = currServBlockPtr->getServerNameVector();

		for (size_t j = 0; j < servNames.size(); ++j)
		{
			if (servNames[j] == hostVal)
			{
				return servBlocks[i];
			}
		}
	}
	return NULL;
}

LocationManager *RequestMsg::matchLocationBlock(ServerManager *matchedServPtr)
{
	LocationManager					*matchedLocationPtr = NULL;
	
	std::vector<LocationManager>	&locationBlocks = matchedServPtr->getLocationBlocks();

	matchedLocationPtr = matchLocationByEqual(locationBlocks, this->requestTarget);
	if (matchedLocationPtr == NULL)
	{
		matchedLocationPtr = matchLocationByPrefix(locationBlocks, this->requestTarget);
	}

	if (matchedLocationPtr == NULL && locationBlocks.empty() == false)
	{
		LocationManager		*defaultLocationPtr = NULL;

		defaultLocationPtr = getDefaultLocationPtr(locationBlocks);
		if (defaultLocationPtr == NULL)
			matchedLocationPtr = &(locationBlocks.front());
	}

	return matchedLocationPtr;
}

LocationManager *RequestMsg::getDefaultLocationPtr(std::vector<LocationManager> &locationBlocks)
{
	for (size_t i = 0; i < locationBlocks.size(); ++i)
	{
		if (locationBlocks[i].getRoute() == "/")
			return (&locationBlocks[i]);
	}
	return (NULL);
}

LocationManager *RequestMsg::matchLocationByEqual(std::vector<LocationManager> &locationBlocks, std::string requestTarget)
{
	for (size_t i = 0; i < locationBlocks.size(); ++i)
	{
		if (locationBlocks[i].getHasEqualModifier() && locationBlocks[i].getEqualModifier())
		{ 
								
			std::string	locationRoute = locationBlocks[i].getRoute();
			
			if (locationRoute.compare(requestTarget) == 0)
			{
				return (&locationBlocks[i]);
			}
		}	
	}
	
	return NULL;
}

LocationManager *RequestMsg::matchLocationByPrefix(std::vector<LocationManager> &locationBlocks, std::string &requestTarget)
{
	LocationManager	*matchedLocationPtr = NULL;
	int				maxLength = 0;
	
	for (size_t i = 0; i < locationBlocks.size(); ++i)
	{
		LocationManager	*currLocationPtr = &locationBlocks[i];
		
		if (currLocationPtr->getHasEqualModifier() && !currLocationPtr->getEqualModifier())
		{
			std::string	locationRoute = currLocationPtr->getRoute();
			int			length = getPrefixMatchingLength(locationRoute, requestTarget);
			
			if (length == maxLength && currLocationPtr->getRoute() == requestTarget)
			{
				return currLocationPtr;
			}
			
			if (length > maxLength)
			{
				matchedLocationPtr = currLocationPtr;
				maxLength = length;
			}
		}
	}
	
	return matchedLocationPtr;
}

int	RequestMsg::getPrefixMatchingLength(std::string &locationRoute, std::string &requestTarget)
{
	int	locationRouteLength;
	int	requestTargetLength;
	int	compareLength;
	
	locationRouteLength = locationRoute.size();
	requestTargetLength = requestTarget.size();
	if (locationRouteLength < requestTargetLength)
	{
		compareLength = locationRouteLength;
	}
	else
	{
		compareLength = requestTargetLength;
	}
	
	for (int i = 0; i < compareLength; ++i)
	{
		if (locationRoute[i] != requestTarget[i])
				return -1;
	}
	return compareLength;
}

/*
             █████     ███  ████         
            ░░███     ░░░  ░░███         
 █████ ████ ███████   ████  ░███   █████ 
░░███ ░███ ░░░███░   ░░███  ░███  ███░░  
 ░███ ░███   ░███     ░███  ░███ ░░█████ 
 ░███ ░███   ░███ ███ ░███  ░███  ░░░░███
 ░░████████  ░░█████  █████ █████ ██████ 
  ░░░░░░░░    ░░░░░  ░░░░░ ░░░░░ ░░░░░░  
*/

void			RequestMsg::setStatusInfo(int code, std::string line)
{
	this->statusCode = code;
	this->statusLine = line;
	std::cout << "[DEBUG] STATUS CODE SET : [" << code << "] " << line << "\n";
}

void			RequestMsg::setErrorInfo(int code, std::string line)
{
	this->statusCode = code;
	this->statusLine = line;
	this->isThisError = true;
	std::cout << "[DEBUG] ERROR CODE SET : [" << code << "] " << line << "\n";
}

size_t			RequestMsg::skipWhiteSpace(const std::string &str, const size_t &len, size_t i)
{
	while (i < len && true == isThisWhiteSpace(str, i))
		i++;
	return (i);
}

size_t			RequestMsg::findWhiteSpace(const std::string &str, const size_t &len, size_t i)
{
	while (i < len && false == isThisWhiteSpace(str, i))
		i++;
	return (i);
}

bool			RequestMsg::isThisWhiteSpace(const std::string &str, size_t offset)
{
	if (offset >= str.size())
		return (false);
	if (str[offset] == '\t' || str[offset] == ' ')
		return (true);
	else
		return (false);
}

bool			RequestMsg::hasWhiteSpace(const std::string &str)
{
	for (size_t i = 0; i < str.size(); ++i)
	{
		if (isThisWhiteSpace(str, i) == true)
			return true;
	}
	return false;
}

void            RequestMsg::printRawLines()
{
    std::cout << "[START-LINE]\n";
    std::cout << this->rawStartLine;
	std::cout << "\n[HEADER]\n";
	std::cout << this->rawHeader;
	std::cout << "\n[BODY]\n";
	std::cout << this->rawBody << "\n";
}

void            RequestMsg::printHeaderInfo()
{
	std::cout << "[DEBUG] is this error";
	if (this->isThisError == true)
		std::cout << ": YES\n";
	else
		std::cout << ": NO\n";
	
	std::cout << "[DEBUG] contentType: " << contentTypeVal << "\n";
	std::cout << "[DEBUG] hostVal: " << hostVal << "\n";
	std::cout << "[DEBUG] connectionVal: ";
	if (this->connectionVal == CLOSE)
		std::cout << "CLOSE\n";
	else if (this->connectionVal == KEEP_ALIVE)
		std::cout << "KEEPALIVE\n";
	
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

void            RequestMsg::setRawStartLine(std::string val)
{
    this->rawStartLine = val;
}


void            RequestMsg::setRawHeader(std::string val)
{
    this->rawHeader = val;
}


void            RequestMsg::setRawBody(std::string val)
{
    this->rawBody = val;
}

void            RequestMsg::setStausCode(int val)
{
    this->statusCode = val;
}


void            RequestMsg::setStatusLine(std::string val)
{
    this->statusLine = val;
}

void            RequestMsg::setContentLength(long long val)
{
    this->contentLength = val;
}

void            RequestMsg::setIsThisChunked(bool val)
{
    this->isThischunked = val;
}

void            RequestMsg::setIsThisError(bool val)
{
    this->isThisError = val;
}



void            RequestMsg::setMethod(std::string val)
{
    this->method = val;
}

void            RequestMsg::setRequestTarget(std::string val)
{
    this->requestTarget = val;
}

void            RequestMsg::setHttpVersion(std::string val)
{
    this->httpVersion = val;
}

void            RequestMsg::setMethodVal(int val)
{
    this->methodVal = val;
}

void            RequestMsg::setContentTypeVal(std::string val)
{
    this->contentTypeVal = val;
}

void            RequestMsg::setHostVal(std::string val)
{
    this->hostVal = val;
}

void            RequestMsg::setConnectionVal(int val)
{
    this->connectionVal = val;
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


std::string     RequestMsg::getRawStartLine()
{
    return (this->rawStartLine);
}

std::string     RequestMsg::getRawHeader()
{
    return (this->rawHeader);
}

std::string     RequestMsg::getRawBody()
{
    return (this->rawBody);
}

int             RequestMsg::getStausCode()
{
    return (this->statusCode);
}
std::string     RequestMsg::getStatusLine()
{
    return (this->statusLine);
}

long long       RequestMsg::getContentLength()
{
    return (this->contentLength);
}

bool            RequestMsg::getIsThisChunked()
{
    return (this->isThischunked);
}

bool            RequestMsg::getIsThisError()
{
    return (this->isThisError);
}

std::string     RequestMsg::getMethod()
{
    return (this->method);
}

std::string     RequestMsg::getRequestTarget()
{
    return (this->requestTarget);
}

std::string     RequestMsg::getHttpVersion()
{
    return (this->httpVersion);
}

std::string		RequestMsg::getHeader(std::string name)
{
	return (this->header[name]);
}

int             RequestMsg::getMethodVal()
{
    return (this->methodVal);
}

std::string     RequestMsg::getContentTypeVal()
{
    return (this->contentTypeVal);
}

std::string     RequestMsg::getHostVal()
{
    return (this->hostVal);
}

int             RequestMsg::getConnectionVal()
{
    return (this->connectionVal);
}

LocationManager		*RequestMsg::getMyLocationBlock()
{
	return (this->myLocationBlock);
}