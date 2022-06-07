#ifndef INFO_HPP
#define INFO_HPP

#include "./Server.hpp"
#include <vector>

class Server;

enum status
{
	no,
	ok,
	just_read
};

class Client
{
private:
	int server_sock;
	std::vector<Server>::iterator server_it;
	int status;
	int read_fd;
	
public:
	Client(/* args */);
	~Client();

	const int &get_server_sock(void) const;
	const std::vector<Server>::iterator &get_server_it(void) const;
	const int &get_status(void) const;
	const int &get_read_fd(void) const;

	void set_server_sock(int fd);
	void set_server_it(std::vector<Server>::iterator server_it_);
	void set_status(int y_n);
	void set_read_fd(int fd);
};

#endif