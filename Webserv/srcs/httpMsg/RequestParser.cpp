#include "RequestParser.hpp"
#include "../configParsing/ParsingManager.hpp"
#include <cstring>
#include <iostream>
#include <sstream>
#include <fstream>

RequestParser::RequestParser()
{
    this->currState = BEFORESTART;
    this->onlyStartLineFlag = UNKNOWN;

    this->currOffset = 0;

    this->hasStartLineEndOffset = false;
    this->startLineEndOffset = 0;

    this->hasHeaderStartOffset = false;
    this->headerStartOffset = 0;
    this->hasHeaderEndOffset = false;
    this->headerEndOffset = 0;

    this->hasBodyStartOffset = false;
    this->bodyStartOffset = 0;
    this->hasBodyEndOffset = false;
    this->bodyEndOffset = 0;

    this->hasMsgEndOffset = false;
    this->msgEndOffset = 0;

    this->contentLength = -1;
    this->isThisChunked = false;

    this->errCode = -1;
}

RequestParser::RequestParser(const RequestParser &other)
{
    *this = other;
}

RequestParser::~RequestParser()
{

}

RequestParser   &RequestParser::operator = (const RequestParser &other)
{
    if (this != &other)
    {
        this->currState = other.currState;
        this->currOffset = other.currOffset;
        this->msgEndOffset = other.msgEndOffset;
        this->startLineEndOffset = other.startLineEndOffset;
        this->headerStartOffset = other.headerStartOffset;
        this->headerEndOffset = other.headerEndOffset;
        this->bodyStartOffset = other.bodyStartOffset;
        this->bodyEndOffset = other.bodyEndOffset;
        this->msgEndOffset = other.msgEndOffset;
        this->contentLength = other.contentLength;
        this->isThisChunked = other.isThisChunked;
        this->errCode = other.errCode;
        this->errLine = other.errLine;
        this->dechunkedBody = other.dechunkedBody;
    }
    return (*this);
}

/*
                            ███            
                           ░░░             
 █████████████    ██████   ████  ████████  
░░███░░███░░███  ░░░░░███ ░░███ ░░███░░███ 
 ░███ ░███ ░███   ███████  ░███  ░███ ░███ 
 ░███ ░███ ░███  ███░░███  ░███  ░███ ░███ 
 █████░███ █████░░████████ █████ ████ █████
░░░░░ ░░░ ░░░░░  ░░░░░░░░ ░░░░░ ░░░░ ░░░░░ 
*/

bool            RequestParser::hasCompleteHttpMessage(const std::vector <char> &data)
{
    if (data.size() == 0)
        return (false);

    if (this->currState == BEFORESTART)
    {
        this->currOffset = 0;
        this->currState = STARTLINE;
    }

    while (this->currOffset < data.size())
    {
        if (this->currState == STARTLINE)
        {
            if (hasCompleteStartLine(data) == false && this->currState == STARTLINE)
                return (false);
        }
        else if (this->currState == HEADER)
        {
            if (hasCompleteHeaderLine(data) == false && this->currState == HEADER)
                return (false);
        }
        else if (this->currState == BODY)
        {
            if (hasCompleteBody(data) == false && this->currState == BODY)
                return (false);
        }
        else
            break ;
    }
    return (true);
}

void            RequestParser::fillRequestMsg(const std::vector <char> &data, RequestMsg &msg)
{
    if (this->errCode != -1)
    {
        msg.setStausCode(this->errCode);
        msg.setStatusLine(this->errLine);
        return ;
    }

    msg.setRawStartLine(std::string(data.begin(), data.begin() + startLineEndOffset + 1));
    msg.setRawHeader(std::string(data.begin() + this->headerStartOffset, data.begin() + this->headerEndOffset + 1));
    
    if (this->isThisChunked == true)
        msg.setRawBody(this->dechunkedBody);
    else if (this->contentLength != -1)
        msg.setRawBody(std::string(data.begin() + this->bodyStartOffset, data.begin() + this->bodyEndOffset + 1));

}


void            RequestParser::resetParser()
{
    this->currState = BEFORESTART;
    this->onlyStartLineFlag = UNKNOWN;

    this->currOffset = 0;

    this->hasStartLineEndOffset = false;
    this->startLineEndOffset = 0;

    this->hasHeaderStartOffset = false;
    this->headerStartOffset = 0;
    this->hasHeaderEndOffset = false;
    this->headerEndOffset = 0;

    this->hasBodyStartOffset = false;
    this->bodyStartOffset = 0;
    this->hasBodyEndOffset = false;
    this->bodyEndOffset = 0;

    this->hasMsgEndOffset = false;
    this->msgEndOffset = 0;

    this->contentLength = -1;
    this->isThisChunked = false;

    this->errCode = -1;

    if (this->errLine != "")
        this->errLine = "";
    
    if (this->dechunkedBody != "")
        this->dechunkedBody = "";
}


/*
          █████                         █████               ████   ███                     
         ░░███                         ░░███               ░░███  ░░░                      
  █████  ███████    ██████   ████████  ███████              ░███  ████  ████████    ██████ 
 ███░░  ░░░███░    ░░░░░███ ░░███░░███░░░███░    ██████████ ░███ ░░███ ░░███░░███  ███░░███
░░█████   ░███      ███████  ░███ ░░░   ░███    ░░░░░░░░░░  ░███  ░███  ░███ ░███ ░███████ 
 ░░░░███  ░███ ███ ███░░███  ░███       ░███ ███            ░███  ░███  ░███ ░███ ░███░░░  
 ██████   ░░█████ ░░████████ █████      ░░█████             █████ █████ ████ █████░░██████ 
░░░░░░     ░░░░░   ░░░░░░░░ ░░░░░        ░░░░░             ░░░░░ ░░░░░ ░░░░ ░░░░░  ░░░░░░  
*/

bool            RequestParser::hasCompleteStartLine(const std::vector <char> &data)
{
    long long   j;

    j = getOffsetBeforeCRLF(data, this->currOffset);
    if (j == -1)
        return (false);
    this->startLineEndOffset = j;
    this->hasStartLineEndOffset = true;
    this->currState = HEADER;
    this->currOffset = j + 3;
    return (true);
}

/*
 █████                             █████                   
░░███                             ░░███                    
 ░███████    ██████   ██████    ███████   ██████  ████████ 
 ░███░░███  ███░░███ ░░░░░███  ███░░███  ███░░███░░███░░███
 ░███ ░███ ░███████   ███████ ░███ ░███ ░███████  ░███ ░░░ 
 ░███ ░███ ░███░░░   ███░░███ ░███ ░███ ░███░░░   ░███     
 ████ █████░░██████ ░░████████░░████████░░██████  █████    
░░░░ ░░░░░  ░░░░░░   ░░░░░░░░  ░░░░░░░░  ░░░░░░  ░░░░░     
                                                           
*/

bool            RequestParser::hasCompleteHeaderLine(const std::vector <char> &data)
{
    if (setHeaderStartOffset(data) == false)
        return (false);
    
    if (this->onlyStartLineFlag != NO)
    {
        setOnlyStartLineFlag(data);
        if (this->onlyStartLineFlag == UNKNOWN)
            return (false);
        else if (this->onlyStartLineFlag == YES)
        {
            this->currState = COMPLETE;
            return (true);          
        }
    }

    while (this->currOffset < data.size())
    {
        if (hasDoubleCRLF(data, currOffset) == true)
        {
            setHeaderEndOffset(this->currOffset - 1);

            searchContentLength(data);
            if (this->currState == ERROR)
                return (false);

            searchEncodingChunked(data);
            if (this->currState == ERROR)
                return (false);

            if (this->contentLength == -1 && this->isThisChunked == false)
            {
                this->currState = COMPLETE;
                this->msgEndOffset = this->headerEndOffset + 4;
                return (true);
            }

            this->currState = BODY;
            
            return (true);
        }
        (this->currOffset)++;
    }
    return (false);
}

bool            RequestParser::setHeaderStartOffset(const std::vector <char> &data)
{
    if (this->hasHeaderStartOffset == true)
        return (true);
    if (this->hasStartLineEndOffset == true && \
        this->startLineEndOffset + 3 < data.size())
    {
        this->headerStartOffset = this->startLineEndOffset + 3;
        this->hasHeaderStartOffset = true;
        return (true);
    }
    else
        return (false);
}

void            RequestParser::setHeaderEndOffset(size_t offset)
{
    this->headerEndOffset = offset;
    this->hasHeaderEndOffset = true;
}

void            RequestParser::setOnlyStartLineFlag(const std::vector <char> &data)
{
    if (this->startLineEndOffset + 4 >= data.size())
        return ;
    if (this->startLineEndOffset == getOffsetBeforeCRLF(data, this->startLineEndOffset) && \
        this->startLineEndOffset + 2 == getOffsetBeforeCRLF(data, this->startLineEndOffset + 2))
    {
        this->onlyStartLineFlag = YES;
        this->currOffset = this->startLineEndOffset + 4;
        this->msgEndOffset = this->currOffset;
        this->hasMsgEndOffset = true;
    }    
    else
    {
        this->onlyStartLineFlag = NO;
        this->currOffset = this->startLineEndOffset + 2;
    }
}


void            RequestParser::searchContentLength(const std::vector <char> &data)
{
    const char  *str = "Content-Length:";
    size_t      i = this->headerStartOffset;
    size_t      valStart, valEnd;

    while (i <= headerEndOffset)
    {
        if (i + 15 <= headerEndOffset && std::memcmp(&(data[i]), str, 15) == 0)
        {
            if (this->contentLength != -1)
            {
                setErrInfo(400, "Duplicate Content-Length header");
                return ;
            }
            valStart = skipOWS(data, i + 15, this->headerEndOffset);
            valEnd = getOffsetBeforeCRLF(data, valStart);

            if (valEnd == static_cast<size_t>(-1))
                return ;
            else
            {
                std::string val(data.begin() + valStart, data.begin() + valEnd + 1);
                
                if (ParsingManager::isItAllDigit(val) == false)
                {
                    setErrInfo(400, "Invalid Content-Length value");
                    return ;
                }
                this->contentLength = std::strtol(val.c_str(), NULL, 10);
            }
            i = valEnd;
        }
        i++;
    }
}


void            RequestParser::searchEncodingChunked(const std::vector <char> &data)
{
    const char      *str = "Transfer-Encoding:";
    size_t          i = this->headerStartOffset;
    size_t          valStart, valEnd;

    while (i <= headerEndOffset)
    {
        if (i + 18 <= headerEndOffset && std::memcmp(&(data[i]), str, 18) == 0)
        {
            if (this->isThisChunked == true)
            {
                setErrInfo(400, "Duplicate Transfer-Encoding header");
                return ;
            }
            valStart = skipOWS(data, i + 18, this->headerEndOffset);
            valEnd = getOffsetBeforeCRLF(data, valStart);
            if (valEnd == static_cast<size_t>(-1))
            {
                return ;
            }
            if (valStart == valEnd)
            {
                setErrInfo(400, "Wrong Transfer-Encoding value");
                return ;
            }
            else
            {
                std::string val(data.begin() + valStart, data.begin() + valEnd + 1);

                if (val != "Chunked")
                {
                    setErrInfo(400, "Invalid Transfer-Encoding value");
                    return ;
                }
                this->isThisChunked = true;
            }
            i = valEnd;
        }
        i++;
    }
}


/*
 █████                  █████           
░░███                  ░░███            
 ░███████   ██████   ███████  █████ ████
 ░███░░███ ███░░███ ███░░███ ░░███ ░███ 
 ░███ ░███░███ ░███░███ ░███  ░███ ░███ 
 ░███ ░███░███ ░███░███ ░███  ░███ ░███ 
 ████████ ░░██████ ░░████████ ░░███████ 
░░░░░░░░   ░░░░░░   ░░░░░░░░   ░░░░░███ 
                               ███ ░███ 
                              ░░██████  
                               ░░░░░░   
*/

bool            RequestParser::hasCompleteBody(const std::vector <char> &data)
{
    if (setBodyStartOffset(data) == false)
        return (false);
    
    if (this->isThisChunked == true)
    {
        dechunkData(data);
        if (this->currState == COMPLETE || this->currState == ERROR)
            return (true);
        else
            return (false);
    }
    else
    {
        if (this->bodyStartOffset + this->contentLength <= data.size())
        {
            setBodyEndOffset();
            this->currState = COMPLETE;
            return (true);
        }
        else
            return (false);
    } 
}

bool            RequestParser::setBodyStartOffset(const std::vector <char> &data)
{
    if (this->hasBodyStartOffset == true)
        return (true);
    if (this->headerEndOffset + 5 <= data.size())
    {
        this->bodyStartOffset = this->headerEndOffset + 5;
        this->hasBodyStartOffset = true;
        this->currOffset = this->bodyStartOffset;
        return (true);
    }
    else
        return (false);
}

void            RequestParser::setBodyEndOffset()
{
    this->bodyEndOffset = this->bodyStartOffset + this->contentLength - 1;
    this->hasBodyEndOffset = true;
    this->msgEndOffset = bodyEndOffset;
    this->hasMsgEndOffset = true;
}

void            RequestParser::dechunkData(const std::vector <char> &data)
{
    size_t      i = this->currOffset;
    long long   size;
    long long   j;

    while (i < data.size())
    {
        j = getOffsetBeforeCRLF(data, i);
        if (j == -1)
            return ;

        std::string     sizeStr(data.begin() + i, data.begin() + j + 1);

        size = std::strtol(sizeStr.c_str(), NULL, 16);
        i = (size_t)j + 3;

        if (size < 0)
        {
            return ;
        }
        else if (i + size - 1 > data.size())
        {
            return ;
        } 
        else if (size == 0)
        {
            if (i + 1 < data.size())
            {
                if (data[i] == '\r' && data[i + 1] == '\n')
                {
                    i += 2;
                    this->currOffset = i;
                    this->msgEndOffset = i;
                    this->hasMsgEndOffset = true;
                    this->currState = COMPLETE;
                    return ;
                }
                else
                {
                    setErrInfo(400, "Invalid Chunk End");
                    return ;
                }
            }
            else
                return ;
        }
        else
        {
            j = getOffsetBeforeCRLF(data, i + size - 1);

            if ((long long)i + size - 1 != j)
            {
                setErrInfo(400, "Invalid Chunk Value");
                return ;
            }
            std::string tmp(data.begin() + i, data.begin() + j + 1);
            this->dechunkedBody.append(tmp);
            i = (size_t)j + 3;
        }
    }
    this->currOffset = i;
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

size_t           RequestParser::getOffsetBeforeCRLF(const std::vector <char> &data, size_t i)
{
    while (i + 2 < data.size())
    {
        if (data[i + 1] == '\r' && data[i + 2] == '\n')
            return (i);
        i++;
    }
    return (static_cast<size_t>(-1));
}


bool            RequestParser::hasDoubleCRLF(const std::vector <char> &data, size_t offset)
{
    if (offset + 3 < data.size())
    {
        if (data[offset] != '\r')
            return (false);
        if (data[offset + 1] != '\n')
            return (false);
        if (data[offset + 2] != '\r')
            return (false);
        if (data[offset + 3] != '\n')
            return (false);
        return (true);
    }
    else
        return (false);
}

size_t          RequestParser::skipOWS(const std::vector <char> & data, size_t offset, size_t end)
{
    while (offset <= end && (data[offset] == ' ' || data[offset] == '\t'))
        offset++;
    return (offset);
}

void            RequestParser::setErrInfo(int _code, std::string _line)
{
    this->errCode = _code;
    this->errLine = _line;
    this->currState = ERROR;
}

size_t          RequestParser::getMsgEndOffset()
{
    return (this->msgEndOffset);
}

int             RequestParser::getErrCode()
{
    return (this->errCode);
}