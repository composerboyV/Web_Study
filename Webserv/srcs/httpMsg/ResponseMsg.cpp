#include "ResponseMsg.hpp"
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>

ResponseMsg::ResponseMsg()
{
    this->myRequest = NULL;
    this->myLocation = NULL;
    
    this->isThisError = false;
    this->statusCode = -1;
    
    isFinalTargetDir = false;

}

ResponseMsg::ResponseMsg(const ResponseMsg &other)
{
    this->myRequest = other.myRequest;
    this->myLocation = other.myLocation;
}

ResponseMsg::~ResponseMsg()
{

}

void ResponseMsg::initResponse()
{
    this->myRequest = NULL;
    this->myLocation = NULL;
    
    this->isThisError = false;
    
    this->statusCode = -1;
    this->statusLine = "";
    
    this->finalTargetPath = "";
    this->query = "";
    this->isFinalTargetDir = false;
    
    this->rawStartLine = "";
    this->rawHeader = "";
    this->rawBody = "";
    
    this->rawResponse.clear();
    
    std::cout << "[DEBUG] ResponseMsg initialized successfully\n";
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



void            ResponseMsg::makeResponse()
{
    std::cout << "[DEBUG] 🔔🔔 makeResponse() start\n";

	if (isRedirectRequest() == true)
	{
		std::cout << "[DEBUG] redirect detected\n";
		buildRedirectResponse();
		return ;
	}

    if (this->isThisError == true)
    {
        buildErrorResponse();
        return ;
    }

    if (this->myRequest->getMethodVal() != POST && buildFinalTargetPath() == false)
    {
        buildErrorResponse();
        return ;
    }
    
    std::cout << "[DEBUG] 🔔🔔 FinalTargetPath : " << this->finalTargetPath << "\n";

    if (isCGIRequest()) {
        std::cout << "[DEBUG] CGI request detected\n";
        processCGIRequest();
        return ;
    }

    std::cout << "[DEBUG] processing non-CGI request\n";

    switch(this->myRequest->getMethodVal()) {
        case GET :
            processGetRequest();
            break;

        case POST :
            processPostRequest();
            break;

        case DELETE :
            processDeleteRequest();
            break; 

        default :
            buildErrorResponse();
            return ;
    }

    if (this->rawResponse.size() == 0 && this->isThisError == true)
        buildErrorResponse();

    std::cout << "[DEBUG] ResponseMsg::makeResponse() completed\n";
}

bool	ResponseMsg::isRedirectRequest()
{
	if (this->myLocation && this->myLocation->isRedirectSet() == true)
	{
		return (true);	
	}
	else
	{
		return (false);
	}
}

void	ResponseMsg::buildRedirectResponse()
{
	int			statusCode;
	std::string	redirectPage;
	
	statusCode = this->myLocation->getRedirectStatusCode();
	redirectPage = this->myLocation->getRedirectPageWithKey(statusCode);

	std::stringstream	ss;
	ss << statusCode;
	this->rawStartLine = "HTTP/1.1 " + ss.str() + " ";
	if (statusCode == 301)
		this->rawStartLine += "Moved Permanently\r\n";
	else if (statusCode == 302)
		this->rawStartLine += "Found\r\n";

	this->rawHeader += "Location: " + redirectPage + "\r\n";
	this->rawHeader += "Content-Length: 0\r\n";
	this->rawHeader += "Connection: close\r\n\r\n";

	std::string	fullResponse = this->rawStartLine + this->rawHeader;
	this->rawResponse.clear();
	this->rawResponse.insert(this->rawResponse.end(), fullResponse.begin(), fullResponse.end());

	std::cout << "[DEBUG] redirect statusCode: " << statusCode << " / redirect target: " << redirectPage << "\n";
	std::cout << fullResponse << "\n";
}


/*
   █████████    █████████  █████
  ███░░░░░███  ███░░░░░███░░███ 
 ███     ░░░  ███     ░░░  ░███ 
░███         ░███          ░███ 
░███         ░███    █████ ░███ 
░░███     ███░░███  ░░███  ░███ 
 ░░█████████  ░░█████████  █████
  ░░░░░░░░░    ░░░░░░░░░  ░░░░░ 
*/


bool	ResponseMsg::isCGIRequest()
{
	std::string requestTarget = myRequest->getRequestTarget();

	if (requestTarget.find("/cgi-bin") == 0)
	{
		return (true);
	}
	else
	{
		std::cout << "[DEBUG] Not a CGI request: " << requestTarget << "\n";
		return (false);
	}
}

void        ResponseMsg::processCGIRequest()
{
	std::string								cgiInterpreter;
	std::map <std::string, std::string> 	cgiEnv;

	if (isExecutableScript(this->finalTargetPath) == false)
	{
		buildErrorResponse();
		return ;
	}
	if (setupForCGI(this->finalTargetPath, cgiInterpreter, cgiEnv) == false)
	{
		buildErrorResponse();
		return ;
	}
	processExecutingCGI(cgiInterpreter, this->finalTargetPath, cgiEnv);
}

bool	ResponseMsg::isExecutableScript(std::string &target)
{
	if (fileExists(target) == false)
	{
		setErrorInfo(404, "Not Found");
		return (false);
	}
	if (isDirectory(target) == true)
	{
		setErrorInfo(503, "Forbidden");
		return (false);
	}
	if (access(target.c_str(), X_OK) != 0)
	{
		setErrorInfo(503, "Forbidden");
		return (false);
	}
	return (true);
}

bool	ResponseMsg::setupForCGI(std::string &target, std::string &cgiInterpreter, std::map <std::string, std::string> &cgiEnv)
{
	cgiInterpreter = getCGIInterpreter(target);

	if (cgiInterpreter.empty())
	{
		setErrorInfo(500, "Internal Server Error");
		return (false);
	}
	cgiEnv = buildCgiEnvironment();
	return true;
}

std::map <std::string, std::string>	ResponseMsg::buildCgiEnvironment()
{
	std::map<std::string, std::string> env;

	env["REQUEST_METHOD"] = myRequest->getMethod();
	env["REQUEST_URI"] = myRequest->getRequestTarget();
	env["SCRPIT_NAME"] = myRequest->getRequestTarget();
	env["SERVER_PROTOCOL"] = myRequest->getHttpVersion();
	env["SERVER_SOFTWARE"] = "webserv/1.0";
	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["HTTP_HOST"] = myRequest->getHostVal();
	return (env);
}

std::string     ResponseMsg::getCGIInterpreter(const std::string& target)
{
	size_t		pos;
	std::string	ext;

	pos = target.find_last_of('.');
	if (pos == std::string::npos)
	{
		return "";
	}
	ext = target.substr(pos);
	if (this->myLocation->isExistingCGI(ext) == true)
	{
		return (this->myLocation->getCgiInfoWithKey(ext));
	}
	else
	{
		return "";
	}
}

void	ResponseMsg::processExecutingCGI(const std::string& interpreter, const std::string& target, \
										const std::map<std::string, std::string>& cgiEnv)
{
	std::string	cgiOutput;

	cgiOutput = runCGIScript(interpreter, target, cgiEnv);
	if (cgiOutput.empty())
	{
		setErrorInfo(500, "Internal Server Error");
		buildErrorResponse();
		return ;
	}
	parseCGIOutput(cgiOutput);
	std::cout << "[DEBUG] CGI response processed successfully" << "\n";
}

std::string ResponseMsg::runCGIScript(const std::string& interpreter, const std::string& target, \
										const std::map<std::string, std::string>& cgiEnv)
{  
	int			pipeIn[2], pipeOut[2];
	pid_t		pid;
	std::string	cgiOutput;

	if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1)
	{
		return "";
	}
	pid = fork();
	if (pid == -1)
	{
		closeAllPipe(pipeIn, pipeOut);
		return "";
	}
	if (pid == 0)
	{
		std::cout << "[DEBUG] run CGI" << "\n";
		std::cout << "[DEBUG] interpreter: " << interpreter << "\n";
		setupBeforeExecCGI(pipeIn, pipeOut);
		execCGIScript(interpreter, target, cgiEnv);
	}
	else
	{
		handleCGIOutput(cgiOutput, pipeIn, pipeOut);
		waitpid(pid, NULL, 0);
	}
	return (cgiOutput);
}

void	ResponseMsg::setupBeforeExecCGI(int pipeIn[2], int pipeOut[2])
{
			
	dup2(pipeIn[0], STDIN_FILENO);
	dup2(pipeOut[1], STDOUT_FILENO);
	dup2(pipeOut[1], STDERR_FILENO);
	closeAllPipe(pipeIn, pipeOut);
}

void	ResponseMsg::execCGIScript(const std::string& interpreter, const std::string& target, \
										const std::map<std::string, std::string>& cgiEnv)
{
	char**	envArray = createEnvArray(cgiEnv);
	char*	argv[] = {
		const_cast<char*>(interpreter.c_str()),
		const_cast<char*>(target.c_str()),
		NULL
	};
	
	std::cout << "[DEBUG] Executing: " << interpreter << " " << target << "\n";
	execve(interpreter.c_str(), argv, envArray);
	std::cout << "[DEBUG] execve fail" << "\n";
	exit(1);
}

void	ResponseMsg::handleCGIOutput(std::string &cgiOutput, int pipeIn[2], int pipeOut[2])
{
	close(pipeIn[0]);
	close(pipeOut[1]);
	if (this->myRequest->getMethodVal() == POST)
		sendDataToCGI(pipeIn);
	cgiOutput = readCGIOutput(pipeOut);
}

void	ResponseMsg::sendDataToCGI(int pipeIn[2])
{
	std::string	body = myRequest->getRawBody();
	if (body.empty() == false)
	{
		std::cout << "[DEBUG] Sending POST data to CGI: " << body.length() << " bytes" << "\n";
		write(pipeIn[1], body.c_str(), body.length());
	}
	close(pipeIn[1]);
}


std::string	ResponseMsg::readCGIOutput(int pipeOut[2])
{
	std::string	output;
	char		buffer[8192];
	ssize_t		bytesRead;
	
	while ((bytesRead = read(pipeOut[0], buffer, sizeof(buffer) - 1)) > 0)
	{
		buffer[bytesRead] = '\0';
		output.append(buffer, bytesRead);
	}
	close(pipeOut[0]);
	return (output);
}

char**  ResponseMsg::createEnvArray(const std::map<std::string, std::string>& env)
{
	char	**envArray = new char*[env.size() + 1];
	int		i = 0;
	std::map <std::string, std::string>::const_iterator	iter;

	for (iter = env.begin(); iter != env.end(); ++iter)
	{
		std::string envVar = iter->first + "=" + iter->second;
		const size_t len = envVar.size();
		envArray[i] = new char[len + 1];
		std::memcpy(envArray[i], envVar.c_str(), len + 1);
		++i;
	}
	envArray[i] = NULL;
	return (envArray);
}

void    ResponseMsg::parseCGIOutput(const std::string& cgi_output)
{
	size_t  headerEnd = cgi_output.find("\r\n\r\n");
	if (headerEnd == std::string::npos)
	{
		headerEnd = cgi_output.find("\n\n");
		if (headerEnd == std::string::npos)
		{
			statusCode = 200;
			rawStartLine = "HTTP/1.1 200 OK";
			std::ostringstream ss;

			ss << cgi_output.size();
			rawHeader = "Content-type: text/html\r\nContent-Length: " + ss.str() + "\r\n";
			rawBody = cgi_output;
			std::string fullResponse = rawStartLine + "\r\n" + rawHeader + "\r\n" + rawBody;
			rawResponse = std::vector<char>(fullResponse.begin(), fullResponse.end());
			return ;
		}
	}
	std::string headerPart = cgi_output.substr(0, headerEnd);
	std::string bodyPart = cgi_output.substr(headerEnd + (headerEnd == cgi_output.find("\r\n\r\n") ? 4: 2 ));

	statusCode = 200;
	rawStartLine = "HTTP/1.1 200 OK";
	rawHeader ="";
	std::istringstream  headerStream(headerPart);
	std::string line;

	while (std::getline(headerStream, line)) {
		if (!line.empty() && line[line.length() - 1] == '\r')
		{
			line.erase(line.length() - 1);
		}
		if (line.find("Content-Type:") == 0)
		{
			rawHeader += line + "\r\n";
		}
		else if (line.find("Status:") == 0)
		{
			std::string statusStr = line.substr(7);
			if (statusStr.length() >= 3) {
				std::stringstream ss(statusStr.substr(0,3));
				int statusCode;
				ss >> statusCode;
				std::string rawStartLine = "HTTP/1.1" + statusStr;
			}
		}
		else if (!line.empty())
		{
			rawHeader = line + "\r\n";
		}
	}

	if (rawHeader.find("Content-Length:") == std::string::npos)
	{
		std::stringstream ss;
		ss << bodyPart.size();
		rawHeader += "Content-Length: " + ss.str() + "\r\n";
	}
	rawBody = bodyPart;

	std::string fullResponse = rawStartLine + "\r\n" + rawHeader + "\r\n" + rawBody;
	rawResponse = std::vector<char>(fullResponse.begin(), fullResponse.end());
}

void	ResponseMsg::closeAllPipe(int pipe1[2], int pipe2[2])
{
	close(pipe1[0]);
	close(pipe1[1]);
	close(pipe2[0]);
	close(pipe2[1]);
}


/*
       ███   █████████  ██████████ ███████████ ███      
     ███░   ███░░░░░███░░███░░░░░█░█░░░███░░░█░░░███    
   ███░    ███     ░░░  ░███  █ ░ ░   ░███  ░   ░░░███  
 ███░     ░███          ░██████       ░███        ░░░███
░░░███    ░███    █████ ░███░░█       ░███         ███░ 
  ░░░███  ░░███  ░░███  ░███ ░   █    ░███       ███░   
    ░░░███ ░░█████████  ██████████    █████    ███░     
      ░░░   ░░░░░░░░░  ░░░░░░░░░░    ░░░░░    ░░░       
*/

void        ResponseMsg::processGetRequest()
{
    std::cout << "[DEBUG] processing GET request\n";

    if (this->finalTargetPath == "")
    {
        setErrorInfo(404, "Cant find request target");
        return ;
    }
    if (this->isFinalTargetDir == false)
    {
        std::cout << "[DEBUG] [GET] serving regular file: " << this->finalTargetPath << "\n";
        serveFile();
    }
    else
    {
        std::cout << "[DEBUG] [GET] serving directory: " << this->finalTargetPath << "\n";
        serveDirectory();
    }
}

void ResponseMsg::serveFile()
{
    std::ifstream file(this->finalTargetPath.c_str(), std::ios::binary);

    if (!(file.is_open()))
    {
        setErrorInfo(403, "cannot open file");
        return;
    }

    file.seekg(0, std::ios::end);
    std::streampos size = file.tellg();
    if (size < 0)
    {
        setErrorInfo(500, "invalid file size");
        return;
    }
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    file.read(&(buffer[0]), size);
    if (file.fail() && !file.eof())
    {
        setErrorInfo(500, "file read Error from server(maybe memory issue)");
        return;
    }
    file.close();

    std::string startLine = "HTTP/1.1 200 OK\r\n";
    std::string header = "Content-Type: " + getMimeType() + "\r\n";

    std::ostringstream oss;
    oss << size;
    header += "Content-Length: " + oss.str() + "\r\n\r\n";

    this->rawResponse.clear();
    this->rawResponse.insert(this->rawResponse.end(), startLine.begin(), startLine.end());
    this->rawResponse.insert(this->rawResponse.end(), header.begin(), header.end());
    this->rawResponse.insert(this->rawResponse.end(), buffer.begin(), buffer.end());
}





void        ResponseMsg::serveDirectory()
{
    if (this->myLocation->getAutoIndex() == true)
    {
        std::cout << "[DEBUG] getAutoIndex == true\n";
        std::string     directoryList = getDirectoryList();

        std::cout << "[DEBUG] directoryList \n" << directoryList << "\n";

        std::string     startLine = "HTTP/1.1 200 OK";
        
        std::string     header = "Content-Type: text/html\r\n";

        std::ostringstream  oss;
        oss << directoryList.size();
        header += "Content-Length: " + oss.str() + "\r\n\r\n";

		this->rawResponse.clear();
        this->rawResponse.insert(this->rawResponse.end(), startLine.begin(), startLine.end());
        this->rawResponse.insert(this->rawResponse.end(), header.begin(), header.end());
        this->rawResponse.insert(this->rawResponse.end(), directoryList.begin(), directoryList.end());
    }
    else
    {
        std::cout << "[DEBUG] getAutoIndex == false\n";
        setErrorInfo(403, "Directory listing not allowed");
    }
}

std::string     ResponseMsg::getDirectoryList()
{
    std::string html = "<html><head><title>Directory Listing</title><link rel=\"icon\" href=\"data:,\"></head><body>";
    html += "<h1>Directory Listing for " + this->finalTargetPath + "</h1><ul>";

    DIR         *dir = opendir(this->finalTargetPath.c_str());
    if (dir != NULL)
    {
        while (true)
        {
            struct dirent   *entry = readdir(dir);
            if (entry != NULL)
            {
                std::string     itemName = entry->d_name;
                if (itemName != "." && itemName != "..")
                    html += "<li><a href=\"" + itemName + "\">" + itemName + "</a></li>";
            }
            else
                break ;
        }
        
        closedir(dir);
        return (html);
    }
    else
        html += "directory listing failed\n";
    return (html);
}


/*
       ███ ███████████     ███████     █████████  ███████████ ███      
     ███░ ░░███░░░░░███  ███░░░░░███  ███░░░░░███░█░░░███░░░█░░░███    
   ███░    ░███    ░███ ███     ░░███░███    ░░░ ░   ░███  ░   ░░░███  
 ███░      ░██████████ ░███      ░███░░█████████     ░███        ░░░███
░░░███     ░███░░░░░░  ░███      ░███ ░░░░░░░░███    ░███         ███░ 
  ░░░███   ░███        ░░███     ███  ███    ░███    ░███       ███░   
    ░░░███ █████        ░░░███████░  ░░█████████     █████    ███░     
      ░░░ ░░░░░           ░░░░░░░     ░░░░░░░░░     ░░░░░    ░░░       
*/

void	ResponseMsg::processPostRequest()
{
    std::cout << "[DEBUG] processing POST request\n";

	if (this->myLocation->getHasUploadEnable() == false)
	{
		setErrorInfo(405, "Method Not Allowd");
	}
	else
	{
		processUploadingFile();
	}

	if (this->statusCode == 201)
	{
		std::string startLine = "HTTP/1.1 201 Created\r\n";
		std::string body = "<html><body><h2>created</h2></body></html>";
		std::string header = "Content-Type: text/html\r\n";
		std::ostringstream ss;
		ss << body.size();
		header += "Content-Length: " + ss.str() + "\r\n\r\n";
	
		std::string fullResponse = startLine + header + body;
		this->rawResponse = std::vector<char>(fullResponse.begin(), fullResponse.end());
	}
	else
	{
		buildErrorResponse();
	}
}

void	ResponseMsg::processUploadingFile()
{
	std::string	contentPart, fileContent;
	std::string	fileName, uploadStore;

	uploadStore = combinePath(this->myLocation->getRoot(), this->myLocation->getUploadStore());
	if (isDirectory(uploadStore) == false)
	{
		setErrorInfo(500, "Internal Server Error");
		return ;
	}
	contentPart = getContentPart();
	fileName = extractFileNameFromContent(contentPart);
	fileContent = getFileContent(contentPart);
	uploadFile(fileName, uploadStore, fileContent);
}

std::string	ResponseMsg::getContentPart()
{
	std::string	contentBoundary, contentPart;
	size_t		contentStartOffset, contentEndOffset;
	std::string	requestBody;

	contentBoundary = getContentBoundary();
	requestBody = this->myRequest->getRawBody();
	contentStartOffset = requestBody.find(contentBoundary) + contentBoundary.size();
	contentEndOffset = requestBody.find(contentBoundary + "--");
	contentPart = requestBody.substr(contentStartOffset, contentEndOffset - contentStartOffset);
	return (contentPart);
}

std::string	ResponseMsg::getContentBoundary()
{
	std::string	contentBoundary;
	size_t		boundaryStartOffset, boundaryEndOffset;
	std::string	contentType;

	if (this->myRequest->isExistingHeader("Content-Type") == false)
		return "";
	contentType = this->myRequest->getContentTypeVal();
	boundaryStartOffset = contentType.find("boundary") + 9;
	boundaryEndOffset = boundaryStartOffset;
	while (contentType[boundaryEndOffset])
	{
		if (contentType[boundaryEndOffset] == ' ' || contentType[boundaryEndOffset] == '\t')
			break ;
		++boundaryEndOffset;
	}
	contentBoundary = contentType.substr(boundaryStartOffset, boundaryEndOffset - boundaryStartOffset);
	return (contentBoundary);
}

std::string	ResponseMsg::extractFileNameFromContent(std::string &contentPart)
{
	std::string	fileName;
	size_t		startOffset, endOffset;

	startOffset = contentPart.find("filename") + 9;
	endOffset = startOffset;
	while (contentPart[endOffset])
	{
		if (contentPart[endOffset] == '\r' && contentPart[endOffset + 1] == '\n')
			break ;
		++endOffset;
	}
	fileName = contentPart.substr(startOffset, endOffset - startOffset);
	removeQuote(fileName);
	return (fileName);	
}

void	ResponseMsg::removeQuote(std::string &fileName)
{
	if (fileName[0] == '"' && fileName[fileName.size() - 1] == '"')
	{
		fileName = fileName.substr(1, fileName.size() - 2);
	}
}
std::string	ResponseMsg::getFileContent(std::string contentPart)
{
	std::string	fileContent;
	size_t		startOffset;

	startOffset = contentPart.find("\r\n\r\n") + 4;
	fileContent = contentPart.substr(startOffset);
    return (fileContent);
}

void	ResponseMsg::uploadFile(std::string &fileName, std::string &uploadStore, std::string fileContent)
{
	std::string		fileFullPath;
	std::ofstream	uploadFile;

	fileFullPath = combinePath(uploadStore, fileName);
	uploadFile.open(fileFullPath.c_str());
	if (uploadFile.is_open() == false)
	{
		setErrorInfo(500, "Internal Server Error");
		return ;
	}
	uploadFile << fileContent;
	setStatusInfo(201, "Created");
}

/*
       ███ ██████████   ██████████ █████       ██████████ ███████████ ██████████ ███      
     ███░ ░░███░░░░███ ░░███░░░░░█░░███       ░░███░░░░░█░█░░░███░░░█░░███░░░░░█░░░███    
   ███░    ░███   ░░███ ░███  █ ░  ░███        ░███  █ ░ ░   ░███  ░  ░███  █ ░   ░░░███  
 ███░      ░███    ░███ ░██████    ░███        ░██████       ░███     ░██████       ░░░███
░░░███     ░███    ░███ ░███░░█    ░███        ░███░░█       ░███     ░███░░█        ███░ 
  ░░░███   ░███    ███  ░███ ░   █ ░███      █ ░███ ░   █    ░███     ░███ ░   █   ███░   
    ░░░███ ██████████   ██████████ ███████████ ██████████    █████    ██████████ ███░     
      ░░░ ░░░░░░░░░░   ░░░░░░░░░░ ░░░░░░░░░░░ ░░░░░░░░░░    ░░░░░    ░░░░░░░░░░ ░░░       
*/




void        ResponseMsg::processDeleteRequest()
{
    std::cout << "[DEBUG] processing DELETE request\n";

	if (this->isFinalTargetDir == false)
	{
		deleteFile(this->finalTargetPath);
	}
	if (this->statusCode == 200)
	{
		this->rawStartLine = "HTTP/1.1 200 OK\r\n";
		this->rawBody = "<html><body><h2>Deleted</h2></body></html>\r\n";
		this->rawHeader += "Content-Type: text/html\r\n";
		std::stringstream ss;
		ss << this->rawBody.size();
		this->rawHeader += ("Content-Length: " + ss.str() + "\r\n");
		this->rawHeader += "\r\n";

	
		std::string fullResponse = this->rawStartLine + this->rawHeader + this->rawBody;
		this->rawResponse = std::vector<char>(fullResponse.begin(), fullResponse.end());
	}
	else
	{
		buildErrorResponse();
	}

}

void	ResponseMsg::deleteFile(std::string file)
{
	if (std::remove(file.c_str()) == 0)
	{
		setStatusInfo(200, "OK");
	}
	else
	{
		setErrorInfo(503, "Forbidden");
	}
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

void        ResponseMsg::buildErrorResponse()
{
	if (this->myLocation == NULL || this->isThisError != true)
		return ;
	
	std::cout << "[DEBUG] in build Eror response\n";
	std::string		errorFile = this->myLocation->getErrorPageWithKey(this->statusCode);
	std::cout << "[DEBUG] errorFile : " << errorFile << "\n";
	if (errorFile != "")
	{
		this->finalTargetPath = errorFile;
		if (serveErrorFile() == false)
			generateDefaultErrorPageHtml();
	}
	else
	{
		generateDefaultErrorPageHtml();
	} 
}

bool        ResponseMsg::serveErrorFile()
{
    std::ifstream       file(this->finalTargetPath.c_str(), std::ios::binary);

    if (!(file.is_open()))
    {
        setErrorInfo(403, "cannot open file");
        return (false);
    }

    // 파일 크기 계산
    file.seekg(0, std::ios::end);
    std::streampos      size = file.tellg();
    if (size < 0)
    {
        setErrorInfo(500, "invalid file size");
        return (false);
    }
    file.seekg(0, std::ios::beg);

    std::vector <char>  buffer(size);
    file.read(&(buffer[0]), size);
    if (file.fail())
    {
        setErrorInfo(500, "file read Error from server(maybe memory issue)");
        return (false);
    }
    file.close();

	std::ostringstream		statusCode;
	statusCode << this->statusCode;

    std::string         startLine = "HTTP/1.1 " + statusCode.str() + " " + this->statusLine + "\r\n";

    std::string         header = "Content-Type: " + getMimeType() + "\r\n";

    std::ostringstream  oss;
    oss << size;
	header += "Connection: Close\r\n";
    header += "Content-Length: " + oss.str() + "\r\n\r\n";

	this->rawResponse.clear();
    this->rawResponse.insert(this->rawResponse.end(), startLine.begin(), startLine.end());
    this->rawResponse.insert(this->rawResponse.end(), header.begin(), header.end());
    this->rawResponse.insert(this->rawResponse.end(), buffer.begin(), buffer.end());
	return (true);
}


void		ResponseMsg::generateDefaultErrorPageHtml()
{
	std::ostringstream		oss;
	oss << this->statusCode;

	std::string		defaultErrorHtml = 
		"<!DOCTYPE html>\n"
        "<html>\n"
        "    <head>\n"
        "        <meta charset=\"UTF-8\">\n"
        "        <title>ERROR - " + oss.str() + "</title>\n"
        "    </head>\n"
        "    <body>\n"
        "        <h1>ERROR - " + oss.str() + "</h1>\n"
        "        <p>" + this->statusLine + "</p>\n"
        "    </body>\n"
        "</html>\n";
	
	std::ostringstream		oss2;
	oss2 << defaultErrorHtml.size();
	
	std::string		startLine = "HTTP/1.1 " + oss.str() + " " + this->statusLine + "\r\n";
	
	std::string		header = 
		"Content-Type: text/html\r\n"
		"Connection: Close\r\n"
		"Content-Length: " + oss2.str() + "\r\n\r\n";

	this->rawResponse.clear();
	this->rawResponse.insert(this->rawResponse.end(), startLine.begin(), startLine.end());
	this->rawResponse.insert(this->rawResponse.end(), header.begin(), header.end());
	this->rawResponse.insert(this->rawResponse.end(), defaultErrorHtml.begin(), defaultErrorHtml.end());
}



bool        ResponseMsg::buildFinalTargetPath()
{
    std::string     target;
    DIR             *tmp;

    if (this->myLocation->getRoot().empty() == false)
        target = this->myLocation->getRoot();

    target = combinePath(target, this->myRequest->getRequestTarget());

    stripQuery(target);

    tmp = opendir(target.c_str());
    if (tmp != NULL)
    {
        closedir(tmp);
        if (this->myRequest->getMethodVal() == GET)
            tryToCombineIndex(target);
        else
        {
            std::cout << "[DEBUG] here set\n";
            this->isFinalTargetDir = true;
        }
    }
    else
    {
        if (checkStatAndPermissions(target) == false)
            return (false);
    }
    
    this->finalTargetPath = target;

    std::cout << "[DEBUG] finalTargetPath is " << this->finalTargetPath << "\n";
    return (true);
}

bool                ResponseMsg::checkStatAndPermissions(const std::string &target)
{
    struct stat     st;

    if (stat(target.c_str(), &st) == 0)
    {
        if (S_ISREG(st.st_mode))
        {
            int fd = open(target.c_str(), O_RDONLY);
            if (fd < 0)
            {
                setErrorInfo(403, "file exists but not readable");
                return (false);
            }
            close (fd);
            return (true);
        }
        else if (S_ISDIR(st.st_mode))
        {
            this->isFinalTargetDir = true;
            setErrorInfo(403, "directory not accessible");
            return (false); 
        }
        else
        {
            setErrorInfo(404, "unsupported file type");
            return (false);
        }
    }
    else
    {
        std::cerr << "[ERROR] !!!!!!!!!!!!!!!!!!! target: " << target << "\n";
        perror("stat() failed");
        this->isThisError = true;
        setErrorInfo(404, "not found");
        return (false);
    }
}

std::string         ResponseMsg::combinePath(const std::string &s1, const std::string &s2)
{
    if (s1.empty())
        return (s2);
    else if (s2.empty())
        return (s1);

    if (s1[s1.length() - 1] == '/' && s2[0] == '/')
    {
        return (s1 + s2.substr(1));
    }
    else if (s1[s1.length() - 1] != '/' && s2[0] != '/')
    {
        return (s1 + "/" + s2);
    }
    else
        return (s1 + s2);
}


void                ResponseMsg::stripQuery(std::string &str)
{
    size_t  queryPos = str.find('?');
    if (queryPos != std::string::npos)
    {
        this->query = str.substr(queryPos + 1);
        str = str.substr(0, queryPos);
    }
}

void                ResponseMsg::tryToCombineIndex(std::string &target)
{
    std::vector <std::string>   index = this->myLocation->getIndex();

    for (size_t i = 0; i < index.size(); i++)
    {
        std::string addIndexTmp = combinePath(target, index[i]);
        DIR         *tmp;

        tmp = opendir(addIndexTmp.c_str());
        if (tmp == NULL)
        {
            if (checkStatAndPermissions(addIndexTmp) == true)
            {
                target = addIndexTmp;
                this->isFinalTargetDir = false;
                return ;
            }
        }
        else
            closedir(tmp);
    }

    this->isFinalTargetDir = true;
}


bool	ResponseMsg::isDirectory(std::string &uploadStore)
{
	struct stat	statBuf;

	if (stat(uploadStore.c_str(), &statBuf) == -1)
		return false;
	if (S_ISDIR(statBuf.st_mode) == true)
		return true;
	return false;
}

bool	ResponseMsg::fileExists(std::string file)
{
    if (access(file.c_str(), F_OK) != 0)
		return (false);
	return (true);
}

bool	ResponseMsg::hasDeletePermission(std::string file)
{
	std::string	parentDirectory;

	parentDirectory = getParentDirectory(file);
	if (access(parentDirectory.c_str(), W_OK | X_OK) != 0)
		return (false);
	return (true);
}

std::string	ResponseMsg::getParentDirectory(std::string file)
{
	std::string	directory;
	size_t		directoryEndOffset;

	directoryEndOffset = file.size() - 1;
	while (directoryEndOffset > 0)
	{
		if (file[directoryEndOffset] == '/')
			break ;
		directoryEndOffset--;
	}
	directory = file.substr(0, directoryEndOffset + 1);
	return (directory);
}

std::string     ResponseMsg::getMimeType()
{
    size_t          dotPos = this->finalTargetPath.find_last_of('.');
    if (dotPos == std::string::npos)
        return ("application/octet-stream");

    std::string     extension = this->finalTargetPath.substr(dotPos + 1);

    if (extension == "html" || extension == "htm")
        return ("text/html");
    if (extension == "css")
        return ("text/css");
    if (extension == "js")
        return ("application/javascript");
    if (extension == "json")
        return ("application/json");
    if (extension == "png")
        return ("image/png");
    if (extension == "jpg" || extension == "jpeg")
        return ("image/jpeg");
    if (extension == "gif")
        return ("image/gif");
    if (extension == "svg")
        return ("image/svg+xml");
    if (extension == "txt")
        return ("text/plain");
    if (extension == "pdf")
        return ("application/pdf");
    if (extension == "mp3")
        return ("audio/mpeg");
    return ("application/octet-stream");
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

std::vector <char>  ResponseMsg::getRawResponse()
{
    return this->rawResponse;
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

void        ResponseMsg::setRequestMsg(RequestMsg *msgPtr)
{
    this->myRequest = msgPtr;
    if (msgPtr == NULL)
        return ;
    
    this->statusCode = msgPtr->getStausCode();
    this->statusLine = msgPtr->getStatusLine();
    this->isThisError = msgPtr->getIsThisError();
}   

void        ResponseMsg::setLocation()
{
    if (this->myRequest == NULL)
        return ;

    this->myLocation = this->myRequest->getMyLocationBlock();
    std::cout << "[DEBUG] this response msg location is set to =>" << this->myLocation->getRoute() << "\n";
}

void        ResponseMsg::setErrorInfo(int code, std::string line)
{
    this->statusCode = code;
    this->statusLine = line;
    this->isThisError = true;
}


std::string ResponseMsg::intToString(int value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

void		ResponseMsg::setStatusInfo(int code, std::string line)
{
	this->statusCode = code;
	this->statusLine = line;
}