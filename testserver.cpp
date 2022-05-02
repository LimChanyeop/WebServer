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

#define PORT 4242

void set_sockaddr(sockaddr_in *address)
{
	// memset((char *)address, 0, sizeof(*address)); // ?
	address->sin_family = AF_INET;
	address->sin_port = htons(PORT);
	address->sin_addr.s_addr = htonl(INADDR_ANY);
	memset(address->sin_zero, '\0', sizeof(address->sin_zero));
}

int main()
{
	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in address;
	int listen_fd;

	if (server_fd <= 0)
	{
		std::cerr << "socket error" << std::endl;
		return 0;
	}

	set_sockaddr(&address);

	int address_len = sizeof(address);
	if (int ret = bind(server_fd, (sockaddr *)&address, address_len) < 0) // (socklen_t)sizeof
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
	int optvalue = 1;
	setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &optvalue, sizeof(optvalue)); // for bind error(Not fixxed)

	while (1)
	{
		std::cout << "waiting for new connection...\n";
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
		int valread = read(acc_socket, request, 1024);
		std::string str_buf = request;
		std::cout << str_buf << std::endl;

		std::string hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: ";
		hello += std::to_string(str_buf.length() + 16);
		std::cout << std::to_string(str_buf.length()) << std::endl;
		hello += "\n\nyour request :\n\n";
		hello += str_buf; // IMPORTANT!!

		write(acc_socket, hello.c_str(), hello.length());

		close(acc_socket);
	}

	return 0;
}
