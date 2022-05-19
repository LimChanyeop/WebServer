// #include <sys/types.h>
// #include <sys/stat.h>

#include "../includes/Webserv.hpp"
#include "../includes/ClientSocket.hpp"
#include "../includes/ServerSocket.hpp"
#include "../includes/Socket.hpp"
#include "../includes/Request.hpp"
#include <sys/socket.h>

void change_events(std::vector<struct kevent> &change_list, uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data,
				   void *udata) // 이벤트를 생성하고 이벤트 목록에 추가하는 함수
{
	struct kevent temp_event;

	EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
	change_list.push_back(temp_event);
}

int main(int argc, char *argv[])
{
	std::vector<std::string> vec_attr;
	split_config(remove_annotaion(argv[1]), vec_attr);
	Base_block base_block;
	base_block.config_parsing(vec_attr);
	// for (std::vector<std::string>::iterator it = vec_attr.begin(); it != vec_attr.end(); it++)
	// {
	// 	std::cout << *it << std::endl;
	// }
	// std::cout << "========================\n";
	// base_block.print_all();
	// std::cout << "========================\n";
	// for (std::vector<Server_block>::iterator it = base_block.servers.begin(); it != base_block.servers.end(); it++)
	// {
	// 	it->print_all();
	// 	std::cout << it - base_block.servers.begin();
	// 	std::cout << "===========ser==============\n";
	// }
	Webserv webserv;
	ServerSocket serv_sock;
	serv_sock.set_socket_fd(socket(AF_INET, SOCK_STREAM, 0)); // TCP: SOCK_STREAM UDP: SOCK_DGRAM

	if (serv_sock.get_socket_fd() <= 0)
	{
		std::cerr << "socket error" << std::endl;
		return 0;
	}
	serv_sock.set_sockaddr();
	int optvalue = 1;
	setsockopt(serv_sock.get_socket_fd(), SOL_SOCKET, SO_REUSEADDR, &optvalue,
			   sizeof(optvalue)); // to solve bind error

	int ret;
	if ((ret = bind(serv_sock.get_socket_fd(), (sockaddr *)&serv_sock.get_address(), (socklen_t)sizeof(serv_sock.get_address()))) == -1)
	{
		std::cerr << "bind error : return value = " << ret << std::endl;
		return 0;
	}
	if ((listen(serv_sock.get_socket_fd(), 10)) < 0)
	{
		std::cerr << "listen error" << std::endl;
		exit(0); // why exit?
	}

	int kq_fd = kqueue(); // kqueue() returns a file descriptor

	std::vector<struct kevent> change_list;
	struct kevent event_list[8];
	std::map<int, std::string> clients;

	change_events(change_list, serv_sock.get_socket_fd(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

	int num_of_event;
	while (1)
	{
		std::string str_buf;

		std::cout << "waiting for new connection...\n";

		int n_changes = change_list.size(); // number of changes = 등록하고자 하는 이벤트 수
		int n_event_list = 8;
		if ((num_of_event = kevent(kq_fd, &change_list[0], n_changes, event_list, n_event_list, NULL)) == -1)
		{
			std::cerr << "kevent error\n";
			exit(0);
		}
		change_list.clear(); // 등록 이벤트 목록 초기화

		for (int i = 0; i < num_of_event; i++)
		{
			// print_event(event_list[i]);
			if (event_list[i].filter == EVFILT_READ)
			{
				std::cout << "accept READ Event / ident :" << event_list[i].ident << std::endl;
				if (event_list[i].ident == serv_sock.get_socket_fd())
				{
					int acc_fd;
					if ((acc_fd = accept(serv_sock.get_socket_fd(), (sockaddr *)&serv_sock.get_address(),
										 (socklen_t *)&serv_sock.get_address_len())) == -1) //
					{
						std::cerr << "accept error" << std::endl;
						exit(0);
					}
					// fcntl(acc_fd, F_SETFL, O_NONBLOCK);
					change_events(change_list, acc_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
					change_events(change_list, acc_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
					// getsockname(acc_fd, (sockaddr *)&serv_sock.get_address(), (socklen_t *)&serv_sock.get_address_len());
					clients[acc_fd] = "ok";
				}
				else if (clients.find(event_list[i].ident) != clients.end())
				{
					char READ[1024] = {0};
					std::cout << "SOCK NAME: " << clients[event_list[i].ident] << "\n";
					int valread = read(event_list[i].ident, READ, 1024);
					std::string request = READ;
					if (request_checker(request, base_block) < 0)
					{
						std::cerr << "Invalid Request\n";
						// exit(0);
					}
					// int valread = recv(acc_socket, request, 1024, 0);
					Request rq;
					rq.split_request(request);
					rq.request_parsing(rq.requests);
					rq.print_request();
					try
					{
						// for (std::vector<std::string>::iterator it = rq.requests.begin(); it != rq.requests.end(); it++)
						// 	std::cout
						// 		<< *it << std::endl;
					}
					catch (std::exception &e)
					{
						std::cout << e.what() << std::endl;
					}
				}
			}
			else if (event_list[i].filter == EVFILT_WRITE)
			{
				std::cout << "accept WRITE Event / ident :" << event_list[i].ident << std::endl;
				webserv.set_response(i, str_buf);
				write(event_list[i].ident, webserv.get_response().c_str(), webserv.get_response().length());
				// write(event_list[i].ident, "<head><link rel=\"shortcut icon\" href=\"test.ico\"></head>", 54);

				close(event_list[i].ident);
			}
		}
	}
	return 0;
}

int request_checker(std::string &request, const Base_block &bb)
{
	// std::string::iterator it
	int find_n = request.find("\n\n");
	if (find_n == std::string::npos)
	{
		std::cerr << "No body No body wants you\n";
		return -1;
	}
	if (find_n > bb.get_request_limit_header_size())
	{
		std::cerr << "431 Error\n";
		return -1;
	}
	return 0;
}
