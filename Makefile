CC = c++

# MAIN = main.cpp

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
		

# SOBJ = $(SFILE:.c=.o)
CFILE = ./Client/testclient.cpp
# COBJ = $(CFILE:.c=.o)

SNAME = Webserv
CNAME = TestClient

all : $(SNAME) $(CNAME)

#  -g -fsanitize=address
$(SNAME) : $(SFILE)
	$(CC) $(SFILE) -o $(SNAME) -g -fsanitize=address

$(CNAME) : $(CFILE)
	$(CC) $(CFILE) -o $(CNAME) -g -fsanitize=address

fclean :
	rm -f $(SNAME) $(CNAME)

re : fclean all

.PHONY : all fclean
