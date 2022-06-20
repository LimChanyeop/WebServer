#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "Server.hpp"

#include <vector>
#include <iostream>

class Server;

class Config
{
private:
	std::vector<Server> v_server;

	int client_limit_body_size;
	int request_limit_header_size;
	std::string user;
	std::string worker_processes;
	std::string root;
	std::string index;
	std::string autoindex;
	std::string return_n;
	std::string error_page;
	std::string cgi_path;
	std::string allow_methods;
	std::string auth_key;
	std::string limit_except;

public:
	Config();
	const std::string &get_user(void) const;
	const std::string &get_worker_processes(void) const;
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
	const std::vector<Server> &get_v_server(void) const;
	const std::string &get_limit_except(void) const;

	void set_user(std::string str);
	void set_worker_processes(std::string str);
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
	void set_v_server(std::vector<Server> v_server_);
	void set_limit_except(std::string str);

	void config_parsing(std::vector<std::string> lists);
	void server_check(void);
};

#endif