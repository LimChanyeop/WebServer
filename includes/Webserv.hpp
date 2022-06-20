#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <bitset>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Kqueue.hpp"
#include "Client.hpp"
#include "Config.hpp"
#include "Kqueue.hpp"
#include "Request.hpp"

#include <fstream>	   // for file io
#include <sys/event.h> // for kqueue
#include <time.h>

class Config;
class Kqueue;
class Client;
class Request;

class Webserv
{
private:
	std::map<std::string, std::string> mimes;
	Kqueue kq;

public:
	Webserv(/* args */);
	~Webserv();

	std::map<std::string, std::string> &get_mimes(void);
	Kqueue &get_kq(void);

	std::string &mime_read(std::string &default_mime);
	void mime_parsing(std::string &mime);
	void ready_webserv(Config &Config);
	std::vector<Server>::iterator find_server_it(Config &Config, Client &client);
	int find_server_id(const int &event_ident, const Config &config, const Request &rq, std::map<int, Client> &clients);
	int find_location_id(const int &server_id, const Config &config, const Request &rq, Client &client);
	int check_size(std::map<int, Client> &clients, Config &config, int &ident, int &lo);
	int is_dir(const Server &server, const Request &rq, Client &client);
	void accept_add_events(const int &event_ident, Server &server, Kqueue &kq, std::map<int, Client> &clients);
	void run_cgi(const Server &server, const std::string &index_root, Client &client);
};

void change_events(std::vector<struct kevent> &change_list, uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data,
				   void *udata);

#endif