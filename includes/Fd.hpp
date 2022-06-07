#ifndef FD_HPP
#define FD_HPP

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>

enum type{
	server,
	client
};
class Fd
{
protected:
	int fd;
	int socket_type;
	unsigned int address_len;
	sockaddr_in address;

public:
	Fd(/* args */);
	~Fd();

	void set_socket_fd(int fd_);
	const int &get_socket_fd(void) const;
	void set_address_len(unsigned int len);
	sockaddr_in &get_address(void);
	unsigned int &get_address_len(void);
};

#endif