#ifndef SERVER_HPP
#define SERVER_HPP

#include "Config.hpp"
#include "Fd.hpp"
#include "Location.hpp"
#include "ParseUtils.hpp"
#include <vector>
#include <sys/event.h>

class Location;
class Config;

class Server : virtual public Fd
{
// protected: // Fd
// 	int fd;
// 	int socket_type;
// 	unsigned int address_len;

public:
	std::vector<Location> v_location;

	// Request request;
	// Response response;

	int client_limit_body_size;
	int request_limit_header_size;
	std::string user;
	std::string worker_processes;
	std::string listen;		 // server
	std::string server_name; // server
	std::string root;
	std::string index;
	std::string autoindex;
	std::string return_n;
	std::string error_page;
	std::string cgi_path;
	std::string allow_methods;
	std::string auth_key;

public:
	Server(Config conf);
	const std::string &get_user(void) const;
	const std::string &get_worker_processes(void) const;
	const std::string &get_listen(void) const;
	const std::string &get_server_name(void) const;
	const std::string &get_root(void) const;
	const std::string &get_index(void) const;
	const std::string &get_autoindex(void) const;
	const std::string &get_return_n(void) const;
	const std::string &get_error_page(void) const;
	const std::string &get_cgi_path(void) const;
	const std::string &get_allow_methods(void) const;
	const std::string &get_auth_key(void) const;
	const int &get_client_limit_body_size(void) const;
	const int &get_request_limit_header_size(void) const;

	void set_user(std::string str);
	void set_worker_processes(std::string str);
	void set_listen(std::string str);
	void set_server_name(std::string str);
	void set_root(std::string str);
	void set_index(std::string str);
	void set_autoindex(std::string str);
	void set_return_n(std::string str);
	void set_error_page(std::string str);
	void set_cgi_path(std::string str);
	void set_allow_methods(std::string str);
	void set_auth_key(std::string str);
	void set_client_limit_body_size(int i);
	void set_request_limit_header_size(int i);

	void config_parsing(std::vector<std::string>::iterator &it, std::vector<std::string> &lists); //, Config_base config_base)
};

#endif