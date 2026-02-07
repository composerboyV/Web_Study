#ifndef RESPONSEMSG_HPP
# define RESPONSEMSG_HPP

# include "RequestMsg.hpp"
# include "../configParsing/LocationManager.hpp"

# include <map>
# include <sys/stat.h>
# include <fstream>
# include <sstream> 
# include <unistd.h>
# include <sys/wait.h>
# include <cstring>

#include <sstream>

class ResponseMsg 
{
    private :
        RequestMsg              *myRequest;
        LocationManager         *myLocation;

        bool                    isThisError;
        int                     statusCode;
        std::string             statusLine;

        std::string             finalTargetPath;
        std::string             query;
        bool                    isFinalTargetDir;

        std::vector <char>      rawResponse;
        
        std::string             rawStartLine;
        std::string             rawHeader;
        std::string             rawBody;


        std::string             intToString(int value);


    public :
		ResponseMsg();
		ResponseMsg(const ResponseMsg &other);
		~ResponseMsg();



        // main
        void                                makeResponse();

        void                                processGetRequest();
        void                                processPostRequest();
        void                                processDeleteRequest();

        // GET METHOD
        void                                serveFile();
        void                                serveDirectory();
        std::string                          getDirectoryList();

        // cgi
		bool								isCGIRequest();
		bool								isExecutableScript(std::string &target);
		bool								setupForCGI(std::string &target, std::string &cgiInterpreter, std::map <std::string, std::string> &cgiEnv);
		
		std::string							getCGIInterpreter(const std::string& target);
        std::string							runCGIScript(const std::string& interpreter, const std::string& target, const std::map<std::string, std::string>& cgiEnv);
		
        std::map<std::string, std::string>	buildCgiEnvironment();
		
        char** 								createEnvArray(const std::map<std::string, std::string>& env);
		
        std::string							readCGIOutput(int pipeOut[2]);

        void                				processCGIRequest();
		void								sendDataToCGI(int pipeIn[2]);
		void								closeAllPipe(int pipe1[2], int pipe2[2]);
		void								parseCGIOutput(const std::string& cgi_output);
		void								setupBeforeExecCGI(int pipeIn[2], int pipeOut[2]);
		void								handleCGIOutput(std::string &cgiOutput, int pipeIn[2], int pipeOut[2]);
		void								execCGIScript(const std::string& interpreter, const std::string& target, const std::map<std::string, std::string>& cgiEnv);
		void								processExecutingCGI(const std::string& interpreter, const std::string& target, const std::map<std::string, std::string>& cgiEnv);
        
        // error
        bool                                serveErrorFile();
        bool                                buildFinalTargetPath();
        bool                                checkStatAndPermissions(const std::string &target);
        
        void                                buildErrorResponse();
		void		                        processUploadingFile();
        void                                stripQuery(std::string &str);
        void                                generateDefaultErrorPageHtml();
		void				                removeQuote(std::string &fileName);
        void                                tryToCombineIndex(std::string &target);
		void		                        uploadFile(std::string &fileName, std::string &uploadStroe, std::string fileContent);
        
		std::string	                        getContentPart();
		std::string	                        getContentBoundary();
		std::string	                        getFileContent(std::string contentPart);
		std::string	                        extractFileNameFromContent(std::string &contentPart);
        std::string                         combinePath(const std::string &s1, const std::string &s2);


		// delete
		void		                        deleteFile(std::string file);

		// redirect
		bool				                isRedirectRequest();

		void				                buildRedirectResponse();
        
		// util
		bool            		            isDirectory(std::string &uploadStore);
		bool		                        fileExists(std::string file); 		
		bool                		        hasDeletePermission(std::string file);
		
        void                				initResponse();
        
        std::string                         getMimeType();
		std::string	                        getParentDirectory(std::string file);


        // getter
        std::vector <char>                  getRawResponse();


        // setter
        void                                setLocation();
        void                                buildHttpResponse();
        void                                setRequestMsg(RequestMsg *msgPtr);
        void                                setErrorInfo(int code, std::string line);
		void				                setStatusInfo(int code, std::string line);
};

#endif