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

#include "Config.hpp"
#include "Kqueue.hpp"
#include "Client.hpp"
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

public:
	Webserv(/* args */);
	~Webserv();

	void ready_webserv(Config &Config);
	std::vector<Server>::iterator find_server_it(Config &Config, Client &client);
	int find_server_id(int event_ident, Config config, Request rq, std::map<int, Client> &clients);
	int find_location_id(int server_id, Config config, Request rq, Kqueue kq);
	void accept_add_events(int event_ident, Server server, Kqueue &kq, std::map<int, Client> &clients);
	int run_cgi(Server server, int location_id, char **envp);
	// int set_event(Config &config, Kq kq);
};

void change_events(std::vector<struct kevent> &change_list, uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata);

#endif