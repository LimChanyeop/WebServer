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

$(SNAME) : $(SFILE)
	$(CC) $(SFILE) -o $(SNAME) -g

$(CNAME) : $(CFILE)
	$(CC) $(CFILE) -o $(CNAME)

fclean :
	rm -f $(SNAME) $(CNAME)

re : fclean all

.PHONY : all fclean
