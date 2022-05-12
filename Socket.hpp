#ifndef SOCKET_HPP
#define SOCKET_HPP

#define CLIENT 1
#define SERVER 2
#define CONNECTION 3

#include "Server.hpp"

class Socket
{
private:
protected:
	int socket_fd;
	int type;

public:
	void set_socket_fd(int socket_fd);
	int get_socket_fd();
};

#endif