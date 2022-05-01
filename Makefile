CC = c++

SFILE = testserver.cpp
# SOBJ = $(SFILE:.c=.o)
CFILE = testclient.cpp
# COBJ = $(CFILE:.c=.o)

SNAME = server
CNAME = client

all : $(SNAME) $(CNAME)

$(SNAME) : $(SFILE)
	$(CC) $(SFILE) -o $(SNAME)

$(CNAME) : $(CFILE)
	$(CC) $(CFILE) -o $(CNAME)

fclean :
	rm -f $(SNAME) $(CNAME)

re : fclean all

.PHONY : all fclean
