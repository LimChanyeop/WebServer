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
	// std::cout << "listen:" << base_block.servers[0].get_listen() << std::endl;
	std::cout << base_block.servers[0].locations.size() << std::endl; // 엄청 큰값이 나온다??
	std::cout << base_block.servers[1].locations.size() << std::endl; // 엄청 큰값이 나온다??
	std::cout << base_block.servers[2].locations.size() << std::endl; // 엄청 큰값이 나온다??
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
	setsockopt(serv_sock.get_socket_fd(), SOL_SOCKET, SO_REUSEPORT, &optvalue,
			   sizeof(optvalue)); // to solve bind error
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
	if (kq_fd < 0)
		std::cerr << "kq error\n";

	std::vector<struct kevent> change_list;
	struct kevent event_list[8];
	std::map<int, std::string> clients;

	change_events(change_list, serv_sock.get_socket_fd(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

	int num_of_event;
	while (1)
	{
		std::string str_buf;
		Request rq;

		// std::cout << "waiting for new connection...\n";

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
			// // print_event(event_list[i]);
			// if (event_list[i].flags & EV_ERROR)
			// {
			// 	if (event_list[i].ident == serv_sock.get_socket_fd())
			// 		std::cerr << "server socket error\n";
			// 	else
			// 	{
			// 		std::cerr << "client socket error" << std::endl;
			// 		// disconnect_client(event_list[i].ident, clients);
			// 	}
			// }
			switch (event_list[i].filter)
			{
			case EVFILT_READ:
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
					fcntl(acc_fd, F_SETFL, O_NONBLOCK);
					change_events(change_list, acc_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
					change_events(change_list, acc_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
					// getsockname(acc_fd, (sockaddr *)&serv_sock.get_address(), (socklen_t *)&serv_sock.get_address_len());
					clients[acc_fd] = "ok";
				}
				else if (clients.find(event_list[i].ident) != clients.end())
				{
					char READ[1024] = {0};
					int valread;
					std::string request;
					while ((valread = read(event_list[i].ident, READ, 1023)) == 1023)
					{
						request += READ;
					}
					if (valread >= 0)
					{
						request += READ;
					}
					// if (request_checker(request, base_block) < 0)
					// {
					// 	std::cerr << "Invalid Request\n";
					// 	exit(0);
					// }
					// int valread = recv(acc_socket, request, 1024, 0);
					rq.split_request(request);
					rq.request_parsing(rq.requests);
					std::cout << "\n\nreq:\n\n";
					rq.print_request();
					clients[event_list[i].ident] = "request";
				}
			}
			case EVFILT_WRITE:
			{
				if (rq.i < 0)
					break;
				std::string port = "4242"; // 왜 포트를 못찾았을까? -> 포트를 파싱 안했었넹~ok -> 근데도 못찾네~
				std::cout << rq.get_host() << std::endl;
				for (i = 0; i < base_block.servers.size(); i++)
				{
					std::cout << "listen:" << base_block.servers[i].get_listen() << "vs" << rq.get_host() << std::endl;
					if (base_block.servers[i].get_listen().c_str() == rq.get_host().c_str()) // 못찾는게,, 이게 다르데;;
					{
						std::cout << "same\n";
						port = rq.get_host();
						break;
					}
				}
				std::string temp;
				int it = rq.referer.find(port);
				if (it == rq.referer.size() || it < 0)
				{
					std::cerr << "Cant find port\n";
				}
				else
					rq.referer.erase(0, it);
				std::cout << "TEST-"
						  << "port:" << port << "it:" << it << "rq.referer:" << rq.referer << "referer:" << rq.referer << std::endl;

				int j;
				std::cout << base_block.servers[i].locations.size() << std::endl; // 엄청 큰값이 나온다?? i = 1로 가정(4242)
				for (j = 0; j < 2; j++)
				{
					std::cout << "TEST-location:" << base_block.servers[1].locations[j].location << std::endl;
					if (base_block.servers[1].locations[j].location == rq.referer)
					{
						break;
					}
				}
				if (clients.find(event_list[i].ident) != clients.end() && clients[event_list[i].ident] == "request")
				{
					std::cout << "accept WRITE Event / ident :" << event_list[i].ident << std::endl;

					std::string route = base_block.servers[1].get_root() + base_block.servers[1].locations[j].location + base_block.servers[1].locations[j].get_index();
					//	/Users/minsikkim/Desktop/WeL0ve42Seoul/WebServer/View + / + Default.html
					std::ifstream ifs(route.c_str());
					std::cout << route << std::endl;
					// std::ifstream ifs("/Users/minsikkim/Desktop/WeL0ve42Seoul/WebServer/View/NAVER.html");
					if (ifs.is_open() == ifs.bad())
						std::cerr
							<< "open error!\n";
					std::string line;
					while (getline(ifs, line))
					{
						rq.response += line;
					}

					webserv.set_response(i, rq.response);
					write(event_list[i].ident, webserv.get_response().c_str(), webserv.get_response().length());
					// write(event_list[i].ident, rq.response.c_str(), rq.response.size());
					clients.erase(event_list[i].ident);
					close(event_list[i].ident);
					ifs.close();
					// exit(0);
				}
			}
			}
			rq.clear_request();
		}
	}
	exit(0);
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
