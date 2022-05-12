#ifndef SERVER_HPP
#define SERVER_HPP

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

#include "./utils/parseUtils.hpp"
#include <fstream>     // for file io
#include <sys/event.h> // for kqueue
#include <time.h>      // for time

#define PORT 4242

class Server
{
private:
        const int port;
        int socket;
        int server_socket;
        int connection_socket;
        std::string response_str;

public:
        Server() : port(PORT){};
        ~Server(){};
        void set_sockaddr(sockaddr_in *address);
        std::string get_response(void) { return response_str; }
        // void setServer(int port, );

        void set_response(int i, std::string str_buf)
        {
                response_str = "HTTP/1.1 200 OK\nContent-Type: "
                               "text/plain\nContent-Length: ";
                response_str += std::to_string(str_buf.length() + 10000);
                response_str += "\n\ndsad<!DOCTYPE html><h1>TITLE</h1></html>\n";
                response_str += str_buf; // IMPORTANT!!
        }
};

void print_event(struct kevent event_list);
#endif