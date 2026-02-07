#ifndef REQUESTPARSER_HPP
# define REQUESTPARSER_HPP

# include <string>
# include <vector>
# include "RequestMsg.hpp"
# include "../enum.h"

class RequestParser
{
    private :
        int                 currState;
        int                 onlyStartLineFlag;

        size_t              currOffset;
        
        bool                hasStartLineEndOffset;
        size_t              startLineEndOffset;

        bool                hasHeaderStartOffset;
        size_t              headerStartOffset;
        bool                hasHeaderEndOffset;
        size_t              headerEndOffset;

        bool                hasBodyStartOffset;
        size_t              bodyStartOffset;
        bool                hasBodyEndOffset;
        size_t              bodyEndOffset;

        bool                hasMsgEndOffset;
        size_t              msgEndOffset;

        long long           contentLength;
        bool                isThisChunked;

        int                 errCode;
        std::string         errLine;
        
        std::string         dechunkedBody;


    public :
        RequestParser();
        RequestParser(const RequestParser &other);
        ~RequestParser();

        RequestParser   &operator = (const RequestParser &other);
        
        int             getErrCode();

        bool            hasCompleteBody(const std::vector <char> &data);
        bool            setBodyStartOffset(const std::vector <char> &data);
        bool            hasCompleteStartLine(const std::vector <char> &data);
        bool            setHeaderStartOffset(const std::vector <char> &data);
        bool            hasCompleteHeaderLine(const std::vector <char> &data);
        bool            hasCompleteHttpMessage(const std::vector <char> &data);
        bool            hasDoubleCRLF(const std::vector <char> &data, size_t offset);

        void            resetParser();
        void            setBodyEndOffset();
        void            initRequestMsg(RequestMsg &msg);
        void            setHeaderEndOffset(size_t offset);
        void            setErrInfo(int _code, std::string line);
        void            dechunkData(const std::vector <char> &data);
        void            searchContentLength(const std::vector <char> &data);
        void            setOnlyStartLineFlag(const std::vector <char> &data);
        void            parseClientReadBuffer(const std::vector <char> &data);
        void            searchEncodingChunked(const std::vector <char> &data);
        void            fillRequestMsg(const std::vector <char> &data, RequestMsg &msg);

        size_t          getMsgEndOffset();
        size_t          getOffsetBeforeCRLF(const std::vector <char> &data, size_t i);
        size_t          skipOWS(const std::vector <char> &data, size_t start, size_t end);
};

#endif