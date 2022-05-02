// #include <sys/time.h>
// #include <fcntl.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <stdlib.h>
// #include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <memory>
#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <sys/event.h> // for kqueue

#define PORT 4242

void set_sockaddr(sockaddr_in *address)
{
	// memset((char *)address, 0, sizeof(*address)); // ?
	address->sin_family = AF_INET;
	address->sin_port = htons(PORT);
	address->sin_addr.s_addr = htonl(INADDR_ANY);
	memset(address->sin_zero, 0, sizeof(address->sin_zero));
}

int main()
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0); // TCP: SOCK_STREAM UDP: SOCK_DGRAM
	sockaddr_in address;
	int listen_fd;

	if (server_fd <= 0)
	{
		std::cerr << "socket error" << std::endl;
		return 0;
	}

	set_sockaddr(&address);

	int address_len = sizeof(address);

	int optvalue = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optvalue, sizeof(optvalue)); // for bind error

	if (int ret = bind(server_fd, (sockaddr *)&address, address_len) < 0) // (socklen_t)sizeof, bind suc:0 fa:-1
	{
		std::cout << ret << std::endl;
		std::cerr << "bind error " << bind(server_fd, (sockaddr *)&address, address_len) << std::endl;
		return 0;
	}

	if ((listen_fd = listen(server_fd, 10)) < 0)
	{
		std::cerr << "listen error" << std::endl;
		exit(0); // why exit?
	}

	// int kq_fd = kqueue(); // kqqqqq 주석을 처리하든 안하든 똑같당 잘 되고있는지 모르겠당
	// if (kq_fd == -1)
	// {
	// 	std::cout << "kqueue error\n";
	// 	exit(0);
	// }
	// std::vector<struct kevent> kq_list;
	// struct kevent event_list[8];
	// struct kevent temp_event;
	// EV_SET(&temp_event, server_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	// kq_list.push_back(temp_event);

	int kevent_v;
	while (1)
	{
		std::cout << "waiting for new connection...\n";

		// if ((kevent_v = kevent(kq_fd, &kq_list[0], kq_list.size(), event_list, 8, NULL)) == -1) // kqqq
		// {
		// 	std::cerr << "kevent error\n";
		// 	exit(0);
		// }

		int acc_socket;
		if ((acc_socket = accept(server_fd,												// block fn
								 (sockaddr *)&address, (socklen_t *)&address_len)) < 0) //
		{
			std::cerr << "accept error" << std::endl;
			exit(0);
		}

		std::cout << "Connected!\n";
		////////////////////// Conneted!!!

		char request[1024] = {0};
		std::string str_buf;
		// int valread = read(acc_socket, &request, 1024);
		int valread = recv(acc_socket, request, 1024, 0);
		str_buf += request;
		std::cout << str_buf << std::endl;

		std::string hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: ";
		hello += std::to_string(str_buf.length() + 16);
		// std::cout << std::to_string(str_buf.length()) << std::endl; // requrest lenth
		hello += "\n\nyour request :\n";
		hello += str_buf; // IMPORTANT!!

		write(acc_socket, hello.c_str(), hello.length());

		close(acc_socket);
	}

	return 0;
}
