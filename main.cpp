#include "./includes/Webserv.hpp"
#include "./includes/Kqueue.hpp"
#include "./includes/Fd.hpp"
#include "./includes/Client.hpp"

#include <netinet/in.h>

int find_server(Config Config, int fd, std::vector<Server>::iterator &server_it)
{
	server_it = Config.v_server.begin();
	for (; server_it != Config.v_server.end(); server_it++)
	{
		// std::cout << "server fd:" << server_it->get_socket_fd() << std::endl;
		if (fd == server_it->get_socket_fd())
		{
			return 1;
		}
	}
	return 0;
}

int main(int argc, char *argv[], char *envp[]) {
	Webserv webserv;
	std::vector<std::string> vec_attr;
	split_config(remove_annotaion(argv[1]), vec_attr);
	Config Config;
	Config.config_parsing(vec_attr);
	// std::cout << "________________________________\n";
	// Config.v_server[1].print_all();
	// std::cout << "________________________________\n";
	// Config.print_all();

	webserv.ready_webserv(Config);
	Kqueue kq;
	std::map<int, Client> clients;
	// kq.setting();
	kq.kq = kqueue();
	for (std::vector<Server>::iterator it = Config.v_server.begin(); it != Config.v_server.end(); it++)
	{
		// std::cout << "event-" << it->get_socket_fd() << std::endl;
		change_events(kq.change_list, it->get_socket_fd(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);	
	}
	while (1) {
		std::string str_buf;

		// std::cout << "waiting for new connection...\n";
		std::vector<Server>::iterator server_it; // clients[kq.event_list[i].ident].get_server_it()
		int num_of_event = kq.set_event();
		for (int i = 0; i < num_of_event; i++)
		{
			int id = kq.event_list[i].ident;
			// std::cout << "rq listen::::::" << server_it->request.get_host() << std::endl;
			if (kq.event_list[i].filter == EVFILT_READ)
			{
				std::cout << "accept READ Event / ident :" << id << std::endl;
				if (clients[id].get_status() == need_to_read) // 이벤트 주체가 READ open
				{
					std::cout << "FILE READ\n";
					char READ[1024] = {0};
					int valread;
					std::string read_for_open;
					// int valread = recv(acc_socket, read_for_open, 1024, 0);

					while ((valread = read(id, READ, 1023)) == 1023)
					{
						read_for_open += READ;
					}
					if (valread >= 0)
					{
						read_for_open += READ;
					}
					int read_fd = clients[id].get_read_fd(); // 6
					clients[read_fd].response.response_str = read_for_open;
					clients[read_fd].set_status(READ_ok);
					close(id);
				}
				else if (find_server(Config, id, server_it)) // 이벤트 주체가 server
				{
					webserv.accept_add_events(id, server_it, kq, clients);
				}
				else if (clients.find(id) != clients.end()) // 이벤트 주체가 client
				{
					server_it = webserv.find_server_it(Config, clients[id]);

					clients[id].request_parsing(id, server_it);
					clients[id].set_status(request_ok); // request_ok

					int server_id = webserv.find_server_id(id, Config, clients[id].request, clients);
					clients[id].set_server_id(server_id);
					// int server_id = server_it - Config.v_server.begin();
					int location_id = webserv.find_location_id(server_id, Config, clients[id].request, kq);
					clients[id].set_location_id(location_id);

					if (clients[id].request.get_method() == "GET")
					{
						std::cout << server_id << " " << location_id << std::endl;
						std::cout << "there is index?" << Config.v_server[server_id].v_location[location_id].get_index() << std::endl;
						if (Config.v_server[server_id].v_location[location_id].get_index() != "")
						{
							// if (autoindex)
							clients[id].set_route(Config.v_server[server_id].get_root() + '/' + Config.v_server[server_id].v_location[location_id].get_index());
							//																		   /View + / + Default.html
							std::cout << "route: " << clients[id].get_route() << std::endl;

							int open_fd = open(clients[id].get_route().c_str(), O_RDONLY);
							if (open_fd < 0)
								std::cerr << "open error - " << clients[id].get_route() << std::endl;
							// std::cout << "my fd::" << id << ", open fd::" << fd << std::endl;
							clients[open_fd].set_read_fd(id); // event_fd:6 -> open_fd:10  발생된10->6
							clients[open_fd].set_status(need_to_read);
							change_events(kq.change_list, open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
						}
						else if (clients[id].request.get_referer().find("php") != std::string::npos)
						{
							clients[id].set_status(READ_ok);
						}
					}

					// if (server_it->request.get_referer().find("php") != std::string::npos) /////////////////// cgi
					// {
						
					// }
					// else
					// {
					// 	clients[id].set_route(Config.v_server[server_id].get_root() + '/' + Config.v_server[server_id].v_location[location_id].get_index());
					// }
				}
			} // if /READ
			else if (kq.event_list[i].filter == EVFILT_WRITE && \
				clients[id].get_server_sock() == server_it->get_socket_fd() && \
				clients[id].get_status() == READ_ok)
			{
				std::cout << "accept WRITE Event / ident :" << id << std::endl;
				if (server_it->get_autoindex() == "on") // location on?
				{
					std::vector<std::string> root;
					std::vector<Location>::iterator it = server_it->v_location.begin();
					for (; it != server_it->v_location.end(); it++) ////// why root no?
					{
						// std::cout << "root:" << it->location << std::endl;
						clients[id].response.set_autoindex(it->location);
						// root.push_back(it->location);
					}
				}
				if (clients[id].get_status() == READ_ok)// && server_it->request.get_host() != "")
				{
					if (clients[id].request.referer.find("favicon.ico") == std::string::npos && clients[id].request.get_method() == "GET")
						clients[id].response.set_header(200, clients[id].response.response_str, ""); // OK
					else
                        clients[id].response.set_header(42, clients[id].response.response_str, ""); //
					std::cout << clients[id].response.get_send_to_response() << std::endl;
					write(id, clients[id].response.get_send_to_response().c_str(), clients[id].response.get_send_to_response().length());

					clients.erase(id);
					close(id);
					clients[id].request.clear_request();
                    clients[id].response.response_str = "";
                    std::cout << "response..\n\n";
				}
			} // else if /WRITE
		}
	}
	exit(0);
	return 0;
}
