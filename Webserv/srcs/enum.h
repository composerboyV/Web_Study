#ifndef ENUM_H
# define ENUM_H

# include <stdint.h>
# include <cstdlib>

enum RequestParserState{
    BEFORESTART,
    STARTLINE,
    HEADER,
    BODY,
    COMPLETE,
    ERROR
};

enum Result{
    YES,
    NO,
    UNKNOWN
};

enum ConfigFileParsingState {
    GLOBAL_DIRECTIVE, 
    SERVER_DIRECTIVE,
    LOCATION_DIRECTIVE,
    UNKNOWN_DIRECTIVE
};

enum MethodVal{
    DEFAULT,
    GET,
    POST,
    DELETE,
    UNIMPLEMENTED
};

enum ConnectionState{
    NOTHING_YET,
    KEEP_ALIVE,
    CLOSE,
};

enum SocketType{
    LISTENING,
    CLIENT
};

#endif