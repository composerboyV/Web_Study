NAME = program

COMPILER = g++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -g

SRCS = srcs/main.cpp \
		srcs/configParsing/MainManager.cpp \
		srcs/configParsing/ParsingManager.cpp \
		srcs/configParsing/ServerManager.cpp \
		srcs/configParsing/LocationManager.cpp \
		srcs/socket/ClientSocket.cpp \
		srcs/socket/ConnectionManager.cpp \
		srcs/socket/ISocket.cpp \
		srcs/socket/ListeningSocket.cpp \
		srcs/httpMsg/RequestParser.cpp \
		srcs/httpMsg/RequestMsg.cpp \
		srcs/httpMsg/ResponseMsg.cpp \

OBJS = $(SRCS:.cpp=.o)

all : $(NAME)

$(NAME) : $(OBJS)
	$(COMPILER) $(CFLAGS) $(OBJS) -o $(NAME)

%.o : %.cpp
	$(COMPILER) $(CFLAGS) -c $< -o $@

clean :
	rm -rf $(OBJS)

fclean : clean
	rm -rf $(NAME)

re : fclean all

.PHONY = all re fclean clean