#ifndef INFO_HPP
#define INFO_HPP

#include "./Request.hpp"
#include "./Response.hpp"
#include "./Server.hpp"
#include <vector>

class Server;

enum status
{
	no,
	server_READ_ok,
	request_ok,
	open_READ_ok,
	need_to_GET_read,
	need_to_is_file_read,
	need_to_cgi_read,
	need_error_read,
	need_to_POST_write,
	need_to_cgi_write,
	WAIT,
	WRITE_LINE, //////////// WRITE
	GET_read_ok,
	is_file_read_ok,
	cgi_read_ok,
	error_read_ok,
	POST_ok,
	ok
};

class Client
{
private:
	int server_sock;
	int server_id;
	int location_id;
	int read_fd;
	int write_fd;
	int status;
	int is_file;
	int RETURN;
	int pid;
	char ip[20];
	Response response;
	Request request;
	std::string route;
	std::string header;
	std::string content_type;
	std::string open_file_name;

public:
	Client(/* args */);
	~Client();

	int request_parsing(int event_ident);

	const int &get_server_sock(void) const;
	// std::vector<Server>::iterator &get_server_it(void); // not const
	const int &get_status(void) const;
	const int &get_read_fd(void) const;
	const int &get_write_fd(void) const;
	const std::string &get_route(void) const;
	const int &get_server_id(void) const;
	const int &get_location_id(void) const;
	const std::string &get_header(void) const;
	const std::string &get_content_type(void) const;
	const std::string &get_open_file_name(void) const;
	const int &get_RETURN(void) const;
	const int &get_is_file(void) const;
	const int &get_pid(void) const;
	const char *get_ip(void) const;
	Request &get_request(void);
	Response &get_response(void);

	void set_server_sock(int fd);
	// void set_server_it(std::vector<Server>::iterator server_it_);
	void set_status(int y_n);
	void set_read_fd(int fd);
	void set_write_fd(int fd);
	void set_route(std::string str);
	void set_server_id(int i);
	void set_location_id(int i);
	void set_header(std::string str);
	void set_content_type(std::string str);
	void set_open_file_name(std::string str);
	void set_RETURN(int i);
	void set_is_file(int i);
	void set_pid(int i);
	void set_ip(const char *str);
	void set_request(Request request_);
	void set_response(Response response_);
};

#endif