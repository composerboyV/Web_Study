#ifndef REQUESTMSG_HPP
# define REQUESTMSG_HPP

# include <string>
# include <map>

# include "../enum.h"
# include "../configParsing/LocationManager.hpp"
# include "../configParsing/ServerManager.hpp"

class RequestMsg {
    private :
        std::string                             rawStartLine;
        std::string                             rawHeader;
        std::string                             rawBody;

        int                                     statusCode;
        std::string                             statusLine;

        long long                               contentLength;
        bool                                    isThischunked;

        bool                                    isThisError;

        std::string                             method;
        std::string                             requestTarget;
        std::string                             httpVersion;

        std::map <std::string, std::string>     header;

        int                                     methodVal;
        std::string                             contentTypeVal;
        std::string                             hostVal;
        int                                     connectionVal;

        LocationManager                         *myLocationBlock;
        bool                                    locationFound;

    public :
        RequestMsg();
        RequestMsg(
            std::string _rawStartLine, 
            std::string _rawHeader,
            std::string _rawBody,
            int         _statusCode,
            std::string _statusLine, 
            long long   _contentLength,
            bool        _isThisChunked,
            bool        _isThisError
        );
        RequestMsg(const RequestMsg &other);
        ~RequestMsg();

        RequestMsg      &operator = (const RequestMsg &other);

        // main function
		void    			parseHost();
		void    			checkMethod();
		void    			parseRawHeader();
		void    			validateRequest();
        void                printHeaderInfo();
		void    			parseConnection();
        void                parseRawStartLine();
        void    			assignHttpMethodEnum();
		void    			parseContentType();
		void    			setErrorInfo(int code, std::string line);
        void    			setStatusInfo(int code, std::string line);
        void                parseHttpRequest(std::vector<ServerManager *> &servBlocks);
		void    			getFieldNameAndValue(size_t &currOffset, std::string &fieldName, std::string &fieldValue);
        
		bool    			isValidName(std::string &name);
		bool    			isValidValue(std::string &value);
		bool    			isExistingHeader(std::string name);
        bool    			checkBodySize();
        
		std::string 		getOneLineByCRLF(size_t &currOffset);
        // location routing
		int				    getPrefixMatchingLength(std::string &locationRoute, std::string &requestTarget);

		void			    routeLocationBlock(std::vector<ServerManager *> &servBlocks);

		ServerManager	    *matchServBlock(std::vector<ServerManager *> &servBlocks);
		ServerManager	    *matchServBlockByHost(std::vector<ServerManager *> &servBlocks, std::string hostVal);

		LocationManager     *matchLocationBlock(ServerManager *matchedServPtr);
		LocationManager     *matchLocationByEqual(std::vector<LocationManager> &locationBlocks, std::string requestTarget);
		LocationManager     *matchLocationByPrefix(std::vector<LocationManager> &locationBlocks, std::string &requestTarget);
        LocationManager     *getDefaultLocationPtr(std::vector<LocationManager> &locationBlocks);


        // setter
        void                setStausCode(int val);
        void                setMethodVal(int val);
        void                setIsThisError(bool val);
        void                setConnectionVal(int val);
        void                setIsThisChunked(bool val);
        void                setMethod(std::string val);
        void                setRawBody(std::string val);
        void                setHostVal(std::string val);
        void                setRawHeader(std::string val);
        void                setStatusLine(std::string val);
        void                setContentLength(long long val);
        void                setHttpVersion(std::string val);
        void                setRawStartLine(std::string val);
        void                setRequestTarget(std::string val);
        void                setContentTypeVal(std::string val);

        // getter
        int                 getStausCode();
        int                 getMethodVal();
        int                 getConnectionVal();
        
        std::string         getMethod();
        std::string         getRawBody();
        std::string         getHostVal();
        std::string         getRawHeader();
        std::string         getStatusLine();
        std::string         getHttpVersion();
        std::string         getRawStartLine();
        std::string         getRequestTarget();
        std::string         getContentTypeVal();
		std::string		    getHeader(std::string name);

        bool                getIsThisError();
        bool                getIsThisChunked();
        
        long long           getContentLength();
        
        LocationManager		*getMyLocationBlock();

        // utils
        bool			isThisWhiteSpace(const std::string &str, size_t offset);
		bool			hasWhiteSpace(const std::string &str);
        
        void            printRawLines();
        
        size_t			skipWhiteSpace(const std::string &str, const size_t &len, size_t i);
        size_t          findWhiteSpace(const std::string &str, const size_t &len, size_t i);

};

#endif