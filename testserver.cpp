// #include <sys/time.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <stdlib.h>
// #include <stdio.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <bitset>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <sys/event.h> // for kqueue
#include <time.h>	   // for time

#define PORT 4242

void change_events(std::vector<struct kevent> &change_list, uintptr_t ident, int16_t filter,
				   uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
	struct kevent temp_event;

	EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
	change_list.push_back(temp_event);
}

void set_sockaddr(sockaddr_in *address)
{
	memset((char *)address, 0, sizeof(*address));
	address->sin_family = AF_INET;
	address->sin_port = htons(PORT);
	address->sin_addr.s_addr = htonl(INADDR_ANY);
	memset(address->sin_zero, 0, sizeof(address->sin_zero));
}

int main()
{
	timespec wait = {10, 0};
	int server_fd =
		socket(AF_INET, SOCK_STREAM, 0); // TCP: SOCK_STREAM UDP: SOCK_DGRAM
	if (server_fd <= 0)
	{
		std::cerr << "socket error" << std::endl;
		return 0;
	}
	sockaddr_in address;
	set_sockaddr(&address);

	int optvalue = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optvalue,
			   sizeof(optvalue)); // to solve bind error

	int address_len = sizeof(address);
	if (int ret = bind(server_fd, (sockaddr *)&address, address_len) <
				  0) // (socklen_t)sizeof, bind suc:0 fa:-1
	{
		std::cerr << "bind error : return value = " << ret << std::endl;
		return 0;
	}

	if ((listen(server_fd, 10)) < 0)
	{
		std::cerr << "listen error" << std::endl;
		exit(0); // why exit?
	}

	int kq_fd = kqueue(); // kqueue() returns a file descriptor
	std::cout << "kq_fd : " << kq_fd << std::endl;

	std::vector<struct kevent> change_list;
	struct kevent event_list[8];
	std::map<int, std::string> clients;

	// change_event();
	std::cout << "change_list N : " << change_list.size() << std::endl;
	struct kevent temp_event;
	EV_SET(&temp_event, server_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	// change_events(change_list, server_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	change_list.push_back(temp_event);

	std::cout << "change_list N : " << change_list.size() << std::endl;

	int num_of_event;
	while (1)
	{
		std::string str_buf;

		std::cout << "waiting for new connection...\n";

		int n_changes =
			change_list.size(); // number of changes = 등록하고자 하는 이벤트 수
		if ((num_of_event = kevent(kq_fd, &change_list[0], n_changes,
								   event_list, 8, NULL)) == -1)
		{
			std::cerr << "kevent error\n";
			exit(0);
		}

		change_list.clear();

		std::cout << "num_of_event : " << num_of_event << std::endl;
		for (int i = 0; i < num_of_event; i++)
		{
			std::cout << "ident : " << event_list[i].ident << std::endl
					  << "filter : " << std::bitset<16>(event_list[i].filter)
					  << std::endl // 모든 bit = 1 출력
					  << "flags : " << std::bitset<16>(event_list[i].flags)
					  << std::endl
					  << "fflags : " << std::bitset<32>(event_list[i].fflags)
					  << std::endl
					  << "data : " << std::bitset<16>(event_list[i].data)
					  << std::endl
					  << "udata : " << event_list[i].udata << std::endl;
			if (event_list[i].filter == EVFILT_READ)
			{
				std::cout << "accept READ Event " << event_list[i].ident << std::endl;
				if (event_list[i].ident == server_fd)
				{
					int acc_fd;
					if ((acc_fd = accept(server_fd, (sockaddr *)&address, (socklen_t *)&address_len)) == -1) //
					{
						std::cerr << "accept error" << std::endl;
						exit(0);
					}
					std::cout << acc_fd << std::endl;
					// fcntl(acc_fd, F_SETFL, O_NONBLOCK);
					change_events(change_list, acc_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
					change_events(change_list, acc_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
					clients[acc_fd] = "";
				}
				else if (clients.find(event_list[i].ident) != clients.end())
				{
					char request[1024] = {0};
					int valread = read(event_list[i].ident, request, 1024);
					// int valread = recv(acc_socket, request, 1024, 0);
					str_buf = request;
					std::cout << str_buf << std::endl;
				}
			}
			else if (event_list[i].filter == EVFILT_WRITE)
			{
				std::cout << "Write event " << event_list[i].ident << std::endl;
				std::string response_str =
					"HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: ";
				response_str += std::to_string(str_buf.length() + 15);
				response_str += "\n\nyour request :\n";
				response_str += str_buf; // IMPORTANT!!
				write(event_list[i].ident, response_str.c_str(), response_str.length());
				close(event_list[i].ident);
			}
		}
		// int acc_socket;
		// if ((acc_socket = accept(server_fd, // block fn
		// 						 (sockaddr *)&address,
		// 						 (socklen_t *)&address_len)) < 0) //
		// {
		// 	std::cerr << "accept error" << std::endl;
		// 	exit(0);
		// }

		// std::cout << "Connected!\n";

		// char request[1024] = {0};
		// int valread = read(acc_socket, request, 1024);
		// // int valread = recv(acc_socket, request, 1024, 0);
		// std::string str_buf = request;
		// std::cout << str_buf << std::endl;

		// std::string response_str =
		// 	"HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: ";
		// response_str += std::to_string(str_buf.length() + 15);
		// response_str += "\n\nyour request :\n";
		// response_str += str_buf; // IMPORTANT!!
		// write(acc_socket, response_str.c_str(), response_str.length());
		// close(acc_socket);
	}
	// close(kq_fd);
	return 0;
}
