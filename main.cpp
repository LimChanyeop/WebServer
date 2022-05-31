#include "./includes/Webserv.hpp"
#include "./includes/Kqueue.hpp"
#include "./includes/Fd.hpp"
#include <netinet/in.h>

int find_server(Config Config, int fd, std::vector<Server>::iterator &server_it)
{
	server_it = Config.v_server.begin();
	for (; server_it != Config.v_server.end(); server_it++)
	{
		// std::cout << "fd:" << server_it->get_socket_fd() << std::endl;
		if (fd == server_it->get_socket_fd())
		{
			return 1;
		}
	}
	return 0;
}

int main(int argc, char *argv[]) {
	Webserv webserv;
	std::vector<std::string> vec_attr;
	split_config(remove_annotaion(argv[1]), vec_attr);
	Config Config;
	Config.config_parsing(vec_attr);
	// std::cout << "________________________________\n";
	// Config.v_server[1].print_all();
	// std::cout << "________________________________\n";
	Config.print_all();

	webserv.ready_webserv(Config);
	Kqueue kq;
	// kq.setting();
	kq.kq = kqueue();
	for (std::vector<Server>::iterator it = Config.v_server.begin(); it != Config.v_server.end(); it++)
	{
		std::cout << "event-" << it->get_socket_fd() << std::endl;
		change_events(kq.change_list, it->get_socket_fd(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);	
	}
	while (1) {
		std::string str_buf;

		std::cout << "waiting for new connection...\n";
		std::vector<Server>::iterator server_it; // kq.clients[kq.event_list[i].ident].get_server_it()
		int num_of_event = kq.set_event();
		// std::cout << "n_o_e:" << num_of_event << " event_ident:" << kq.event_list[0].ident << " filter:" << kq.event_list[0].filter << std::endl;
		// std::cout << "********IT-" << server_it - Config.v_server.begin() << std::endl;
		for (int i = 0; i < num_of_event; i++)
		{
			// std::cout << "rq listen::::::" << server_it->request.get_host() << std::endl;
			if (kq.event_list[i].filter == EVFILT_READ)
			{
				std::cout << "accept READ Event / ident :" << kq.event_list[i].ident << std::endl;
				if (find_server(Config, kq.event_list[i].ident, server_it)) // 이벤트 주체가 server
				{
					std::cout << "(R1)\n";
					std::cout << "client_id:" << kq.event_list[i].ident << " vs server_id:" << server_it->get_socket_fd() << std::endl;					
					int acc_fd;
					if ((acc_fd = accept(server_it->get_socket_fd(), (sockaddr *)&(server_it->get_address()),
										(socklen_t *)&(server_it->get_address_len()))) == -1) //
					{
						std::cerr << "accept error " << acc_fd << std::endl;
						exit(0);
					}
					fcntl(acc_fd, F_SETFL, O_NONBLOCK);
					change_events(kq.change_list, acc_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
					change_events(kq.change_list, acc_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
					kq.clients[acc_fd].set_server_sock( kq.event_list[i].ident);
				}
				else if (kq.clients.find(kq.event_list[i].ident) != kq.clients.end()) // 이벤트 주체가 client
				{
					std::cout << "(R3)\n";

					// kq.clients[kq.event_list[i].ident] -> server sock_fd로 v_server[?]를 알아내면 됨
					// iterator로 v_server.get_fd() == kq.clients[kq.event_list[i].ident] 맞는것 찾아내면 됨
					for (std::vector<Server>::iterator it = Config.v_server.begin(); it != Config.v_server.end(); it++)
					{
						if (it->get_socket_fd() == kq.clients[kq.event_list[i].ident].get_server_sock())
							server_it = it; // 드디어 어떤 서버인지 찾음
					}
					if (server_it == Config.v_server.end())
					{
						std::cerr << "Cant not found Server\n";
						exit(-1);
					}
					char READ[1024] = {0};
					int valread;
					std::string request;
					// int valread = recv(acc_socket, request, 1024, 0);
					while ((valread = read(kq.event_list[i].ident, READ, 1023)) == 1023)
					{
						request += READ;
					}
					if (valread >= 0)
					{
						request += READ;
					}
					server_it->request.split_request(request);
					server_it->request.request_parsing(server_it->request.requests);

					// kq.clients[kq.event_list[i].ident].set_status(request_ok); // request_ok

					int server_id;
					int location_id;						
					server_id = webserv.find_server_id(i, Config, server_it->request, kq);
					location_id = webserv.find_location_id(server_id, Config, server_it->request, kq);

					std::cout<< "server_id = " << server_id << "location_id = " << location_id << std::endl;
					std::cout << "|" <<  Config.v_server[server_id].v_location[location_id].get_index() << "|\n";
					std::string route = Config.v_server[server_id].get_root() + '/' + remove_delim(Config.v_server[server_id].v_location[location_id].get_index());
					//	/Users/minsikkim/Desktop/WeL0ve42Seoul/WebServer/View + / + Default.html
					std::cout << "route: "<<route << std::endl;

					std::ifstream ifs(route.c_str());
					if (ifs.is_open() == ifs.bad())
						std::cerr << "open error!\n";
					std::string line;
					while (getline(ifs, line).good())
					{
						server_it->response.get_response() += line;
					}
					kq.clients[kq.event_list[i].ident].set_status(ok);
				}
			} // if /READ
			else if (kq.event_list[i].filter == EVFILT_WRITE && \
				kq.clients[kq.event_list[i].ident].get_server_sock() == server_it->get_socket_fd())// && server_it->request.get_host() != "")
			{
				std::cout << "accept WRITE Event / ident :" << kq.event_list[i].ident << std::endl;

				if (kq.clients[kq.event_list[i].ident].get_status() == ok)// && server_it->request.get_host() != "")
				{
					std::cout << "(W5)\n";
					if (server_it->request.referer.find("favicon.ico") != std::string::npos)
						server_it->response.set_response(42, server_it->response.get_response()); // 42
					else
						server_it->response.set_response(1, server_it->response.get_response());
					// std::cout << "***********response:" << webserv.get_response() << webserv.get_response().size() << std::endl;
					write(kq.event_list[i].ident, server_it->response.get_response().c_str(), server_it->response.get_response().length());

					kq.clients.erase(kq.event_list[i].ident);
					close(kq.event_list[i].ident);
					close(kq.clients[kq.event_list[i].ident].get_read_fd()); //ifs.close();
					server_it->request.clear_request();
					server_it->response.set_response(2, "");
				}
			} // else if /WRITE
		}
	}
	exit(0);
	return 0;
}
