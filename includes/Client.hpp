#ifndef INFO_HPP
#define INFO_HPP

#include "./Server.hpp"
#include "./Request.hpp"
#include "./Response.hpp"
#include <vector>

class Server;

enum status
{
	no,
	server_READ_ok,
	request_ok,
	open_READ_ok,
	READ_ok,
	ok,
	need_to_read
};

class Client
{
public:
	int server_sock;
	std::vector<Server>::iterator server_it; // server
	int server_id;
	int location_id;
	int read_fd;
	int status;
	// std::string request;
	Response response;
	Request request;
	std::string route;
	
public:
	Client(/* args */);
	~Client();

	void request_parsing(int event_ident, std::vector<Server>::iterator &server_it);

	const int &get_server_sock(void) const;
	std::vector<Server>::iterator &get_server_it(void); // not const
	const int &get_status(void) const;
	const int &get_read_fd(void) const;
	const std::string &get_route(void) const;
	const int &get_server_id(void) const;
	const int &get_location_id(void) const;

	void set_server_sock(int fd);
	void set_server_it(std::vector<Server>::iterator server_it_);
	void set_status(int y_n);
	void set_read_fd(int fd);
	void set_route(std::string str);
	void set_server_id(int i);
	void set_location_id(int i);
};

#endif