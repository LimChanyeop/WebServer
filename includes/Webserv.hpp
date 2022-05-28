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

#include "ParseUtils.hpp"
#include "Config.hpp"

#include <fstream>	   // for file io
#include <sys/event.h> // for kqueue
#include <time.h>

class Config;

class Webserv
{
private:

public:
	Webserv(/* args */);
	~Webserv();

	void ready_webserv(Config &Config);

	int set_event(Config &config);
};

void change_events(std::vector<struct kevent> &change_list, uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void *udata);

#endif