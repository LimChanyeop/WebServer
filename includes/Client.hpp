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
	chunked_WAIT,
	chunked_FINISH,
	WAIT,
	WRITE_LINE, //////////// WRITE
	redi_write,
	GET_read_ok,
	is_file_read_ok,
	cgi_read_ok,
	error_read_ok,
	POST_ok,
	DELETE_ok,
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
	std::string redi_root;
	int pid;
	FILE *fp;
	char ip[20];
	Response response;
	Request request;
	std::string route;
	std::string content_type;
	std::string open_file_name;
	std::string chunked_str;

public:
	Client(/* args */);
	~Client();

	void header_parsing(std::string &read_str);
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
	const std::string &get_redi_root(void) const;
	const int &get_is_file(void) const;
	const int &get_pid(void) const;
	const FILE *get_fp(void) const;
	const char *get_ip(void) const;
	Request &get_request(void);
	Response &get_response(void);
	const std::string &get_chunked_str(void);

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
	void set_redi_root(std::string str);
	void set_is_file(int i);
	void set_fp(FILE *fp);
	void set_pid(int i);
	void set_ip(const char *str);
	void set_request(Request request_);
	void set_response(Response response_);
};

#endif