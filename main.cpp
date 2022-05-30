#include "./includes/Webserv.hpp"
#include "./includes/Kqueue.hpp"
#include "./includes/Fd.hpp"
#include <netinet/in.h>

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

		// std::cout << "waiting for new connection...\n";

		for (std::vector<Server>::iterator server_it = Config.v_server.begin(); server_it != Config.v_server.end(); server_it++)
		{
			// std::cout << "********IT-" << server_it - Config.v_server.begin() << std::endl;
			int num_of_event = kq.set_event();
			for (int i = 0; i < num_of_event; i++)
			{
				std::cout << "rq listen::::::" << server_it->request.get_host() << std::endl;
				// print_event(server_it->event_list[i]);
				if (kq.event_list[i].filter == EVFILT_READ)
				{
					std::cout << "accept READ Event / ident :" << kq.event_list[i].ident << std::endl;
					std::cout << "client_id:" << kq.event_list[i].ident << " vs server_id:" << server_it->get_socket_fd() << std::endl;
					if (kq.event_list[i].ident == server_it->get_socket_fd())
					{
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
						kq.clients[acc_fd] = "server";
					}
					else if (kq.clients.find(kq.event_list[i].ident) != kq.clients.end())
					{ // client
						char READ[1024] = {0};
						int valread;
						std::string request;
						while ((valread = read(kq.event_list[i].ident, READ, 1023)) == 1023)
						{
							request += READ;
						}
						if (valread >= 0)
						{
							request += READ;
						}
						// std::cout << "*request*\n" << request << std::endl;
						// if (request_checker(request, base_block) < 0)
						// {
						// 	std::cerr << "Invalid Request\n";
						// 	exit(0);
						// }
						// int valread = recv(acc_socket, request, 1024, 0);
						server_it->request.split_request(request);
						server_it->request.request_parsing(server_it->request.requests);

						// std::cout << "\n\nreq:\n\n";
						// rq.print_request();
						kq.clients[kq.event_list[i].ident] = "client";
						// std::cout << "rq requests[0]: " << server_it->request.requests[0] << std::endl;
						// std::cout << "rq listen::::::" << server_it->request.get_host() << std::endl;
					}
				}
				else if (kq.event_list[i].filter == EVFILT_WRITE && kq.clients[kq.event_list[i].ident] == "client" && server_it->request.get_host() != "")
				{
					std::cout << "accept WRITE Event / ident :" << kq.event_list[i].ident << std::endl;

					int server_id;
					int location_id;
					
					// std::cout << "rq listen::::::" << server_it->request.get_host() << std::endl;
					server_id = webserv.find_server_id(i, Config, server_it->request, kq);
					// std::cout << "TEST-"
					// 	  << "port:" << port << "server_id:" << server_id << "referer:" << rq.referer << std::endl;
					location_id = webserv.find_location_id(server_id, Config, server_it->request, kq);

					if (kq.clients.find(kq.event_list[i].ident) != kq.clients.end())
					{
						std::cout<< "server_id = " << server_id << "location_id = " << location_id << std::endl;
						std::cout << "|" <<  Config.v_server[server_id].v_location[location_id].get_index() << "|\n";
						std::string route = Config.v_server[server_id].get_root() + '/' + remove_delim(Config.v_server[server_id].v_location[location_id].get_index());
						//	/Users/minsikkim/Desktop/WeL0ve42Seoul/WebServer/View + / + Default.html
						std::cout << "route: "<<route << std::endl;
						// std::ifstream ifs("/Users/minsikkim/Desktop/WeL0ve42Seoul/WebServer/View/NAVER.html");
						std::ifstream ifs(route.c_str());
						if (ifs.is_open() == ifs.bad())
							std::cerr << "open error!\n";
						// else
							// change_events(change_list, , EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
						std::string line;
						while (getline(ifs, line).good())
						{
							server_it->response.get_response() += line;
						}
						if (server_it->request.referer.find("favicon.ico") != std::string::npos)
							server_it->response.set_response(1, server_it->response.get_response()); // 42
						else
							server_it->response.set_response(1, server_it->response.get_response());
						// write(kq.event_list[i].ident, rq.response.c_str(), rq.response.size());
						// std::cout << "***********response:" << webserv.get_response() << webserv.get_response().size() << std::endl;
						write(kq.event_list[i].ident, server_it->response.get_response().c_str(), server_it->response.get_response().length());

						kq.clients.erase(kq.event_list[i].ident);
						close(kq.event_list[i].ident);
						ifs.close();
						server_it->request.clear_request();
						server_it->response.set_response(2, "");
					}
				}
			}
		}
	}
	exit(0);
	return 0;
}
