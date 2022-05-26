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

#include "parseUtils.hpp"
#include <fstream>	   // for file io
#include <sys/event.h> // for kqueue
#include <time.h>	   // for time

class Base_block;
#include "Base_block.hpp"

#define PORT 4242

class Webserv
{
private:
	const int port;
	int socket;
	int server_socket;
	int connection_socket;
	std::string response_str;

public:
	Webserv() : port(PORT){};
	~Webserv(){};
	void set_sockaddr(sockaddr_in *address);
	std::string get_response(void) { return response_str; }
	// void setServer(int port, );

	void set_response(int i, std::string str_buf)
	{
		if (i == 1)
		{
			response_str = "HTTP/1.1 200 OK\r\nContent-Type: "
						   "text/html\r\nContent-Length: ";
			response_str += std::to_string(str_buf.length() + 1);
			response_str += "\r\n\r\n";
			response_str += str_buf + "\r\n";
		}
		else if (i == 2)
		{
			response_str = str_buf;
		}
		else if (i == 42)
		{
			response_str = "HTTP/1.1 200 OK\r\nContent-Type: "
						   "image/x-icon\r\nContent-Length: ";
			response_str += std::to_string(str_buf.length() + 1);
			response_str += "\r\n\r\n";
			response_str += str_buf + "\r\n";
		}
	}

	void clear_response(void)
	{
		response_str.clear();
	}
};

void print_event(struct kevent event_list);
int request_checker(std::string &request, const Base_block &bb);
std::string remove_delim(const std::string &str);

#endif