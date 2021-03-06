#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "ParseUtils.hpp"
#include <iostream>
#include "Server.hpp"

class Server;

class Location
{
private:
	std::string location;
	int client_limit_body_size;
	int request_limit_header_size;
	std::string user;
	std::string worker_processes;
	std::string listen;		 // server
	std::string server_name; // server
	std::string root;
	std::string index;
	std::string autoindex;
	std::string error_page;
	std::string cgi_path;
	std::string allow_methods;
	std::string auth_key;
	int redi_status;
	std::string redi_root;

public:
	Location(Server sb);

	const std::string &get_user(void) const;
	const std::string &get_worker_processes(void) const;
	const std::string &get_listen(void) const;
	const std::string &get_server_name(void) const;
	const std::string &get_root(void) const;
	const std::string &get_index(void) const;
	const std::string &get_autoindex(void) const;
	const std::string &get_error_page(void) const;
	const std::string &get_cgi_path(void) const;
	const std::string &get_allow_methods(void) const;
	const std::string &get_auth_key(void) const;
	const std::string &get_location(void) const;
	const int &get_client_limit_body_size(void) const;
	const int &get_request_limit_header_size(void) const;
	const int &get_redi_status(void) const;
	const std::string &get_redi_root(void) const;

	void set_user(std::string str);
	void set_worker_processes(std::string str);
	void set_listen(std::string str);
	void set_server_name(std::string str);
	void set_root(std::string str);
	void set_index(std::string str);
	void set_autoindex(std::string str);
	void set_error_page(std::string str);
	void set_cgi_path(std::string str);
	void set_allow_methods(std::string str);
	void set_auth_key(std::string str);
	void set_client_limit_body_size(int i);
	void set_request_limit_header_size(int i);
	void set_redi_return(int i, std::string root);

	void config_parsing(std::vector<std::string>::iterator &it, std::vector<std::string> &lists);
};

#endif
