CC = c++

SFILE = ./Server/Webserv.cpp \
		./Socket/Socket.cpp \
		./Socket/ServerSocket.cpp \
		./Socket/ClientSocket.cpp \
		./utils/parseUtils.cpp \
		./utils/debug.cpp \
		./Config/Base_block.cpp
		

# SOBJ = $(SFILE:.c=.o)
CFILE = ./Client/testclient.cpp
# COBJ = $(CFILE:.c=.o)

SNAME = Webserv
CNAME = TestClient

all : $(SNAME) $(CNAME)

$(SNAME) : $(SFILE)
	$(CC) $(SFILE) -o $(SNAME)

$(CNAME) : $(CFILE)
	$(CC) $(CFILE) -o $(CNAME)

fclean :
	rm -f $(SNAME) $(CNAME)

re : fclean all

.PHONY : all fclean
