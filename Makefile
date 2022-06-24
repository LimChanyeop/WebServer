CC = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -g -fsanitize=address
#  -g -fsanitize=address
SFILE = ./srcs/Webserv.cpp \
		./srcs/Fd.cpp \
		./srcs/ParseUtils.cpp \
		./srcs/Config.cpp \
		./srcs/Location.cpp \
		./srcs/Server.cpp \
		./srcs/Request.cpp \
		./srcs/Response.cpp \
		./srcs/Kqueue.cpp \
		./srcs/Client.cpp \
		main.cpp

OBJECT = $(SFILE:.cpp=.o)

SNAME = webserv

all : $(SNAME)

$(SNAME) : $(OBJECT)
	$(CC) $(CXXFLAGS) -o $(SNAME) $(OBJECT)

# $(OBJECT) : $(SFILE)
# 	$(CC) $(CXXFLAGS) -c $(SFILE)

clean :
	rm -f $(OBJECT)

fclean :
	rm -f $(SNAME) $(OBJECT)

re : fclean all

.PHONY : all clean fclean
