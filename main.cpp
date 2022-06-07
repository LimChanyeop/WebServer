#include "./includes/Webserv.hpp"
#include "./includes/Kqueue.hpp"
#include "./includes/Fd.hpp"
#include "./includes/Client.hpp"

#include <netinet/in.h>
#include <dirent.h>

int find_server(Config Config, Client &client, int id)
{
	std::vector<Server>::iterator it;
	for (it = Config.v_server.begin(); it != Config.v_server.end(); it++)
	{
		std::cout << "listen:" << it->get_socket_fd() << " vs " << id << std::endl;
		if (it->get_socket_fd() == id)
		{
			std::cout << "It is server!!\n";
			client.set_server_id(it - Config.v_server.begin());
			client.set_server_sock(id);
			std::cout << client.get_server_id() << std::endl;
			std::cout << client.get_server_sock() << std::endl;
			return 1; // 드디어 어떤 서버인지 찾음
		}
	}
	if (it == Config.v_server.end())
	{
		std::cerr << "Can not found Server\n";
		return 0;
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
		int num_of_event = kq.set_event();
		for (int i = 0; i < num_of_event; i++)
		{
			int id = kq.event_list[i].ident;
			// std::cout << "rq listen::::::" << server_it->request.get_host() << std::endl;
			if (kq.event_list[i].filter == EVFILT_READ)
			{
				std::cout << "accept READ Event / ident :" << id << std::endl;
				if (clients[id].get_status() == need_to_read || clients[id].get_status() == need_to_cgi_read) // 이벤트 주체가 READ open
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
					int read_fd = clients[id].get_read_fd(); //
					clients[read_fd].response.response_str = read_for_open;
					clients[read_fd].set_status(READ_ok);
					std::cout << "READ_ok\n";
					close(id);
					clients.erase(id);
				}
				else if (find_server(Config, clients[id], id)) // 이벤트 주체가 server
				{
					webserv.accept_add_events(id, Config.v_server[1], kq, clients);
				}
				else if (clients.find(id) != clients.end()) // 이벤트 주체가 client
				{
					// server_it = webserv.find_server_it(Config, clients[id]);

					clients[id].request_parsing(id);
					clients[id].set_status(request_ok); // requests_ok
					// clients[id].set_status(ok);
					int server_id = webserv.find_server_id(id, Config, clients[id].request, clients);
					clients[id].set_server_id(server_id);
					// int server_id = server_it - Config.v_server.begin();
					int location_id = webserv.find_location_id(server_id, Config, clients[id].request, kq);
					clients[id].set_location_id(location_id);

					if (clients[id].request.get_method() == "GET")
					{
						std::cout << server_id << " " << location_id << std::endl;
						std::cout << "there is index?" << Config.v_server[server_id].v_location[location_id].get_index() << std::endl;
						if (Config.v_server[server_id].v_location[location_id].get_index() != "" && \
							(clients[id].request.get_referer().find("php") == std::string::npos || clients[id].request.get_referer().find("py") == std::string::npos))
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
							change_events(kq.change_list, open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // read event 추가
						}
						else /////////////////// cgi
						{
							int cgi_fd = webserv.run_cgi(Config.v_server[server_id], location_id, envp); // envp have to fix
							
							clients[cgi_fd].set_read_fd(id);
							change_events(kq.change_list, cgi_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
							clients[cgi_fd].set_status(need_to_cgi_read);
						}
					}
					else if (clients[id].request.get_method() == "POST") {
                        // std::cout << "\nPOST\n" << clients[id].request.body << std::endl;
                        std::cout << "OPENDIR: " << opendir(("." + clients[id].request.get_referer()).c_str()) << std::endl;
                        std::ofstream file;
                        if (errno == ENOENT || errno == ENOTDIR) // 2 no file or directory exist
                        {
                            file.open(("." + clients[id].request.get_referer()).c_str(), std::ios::app);
                            file << "Post Body \n";
                            // clients[id].response.set_header(201, clients[id].response.response_str, "");
                            // write(id, clients[id].response.get_send_to_response().c_str(), clients[id].response.get_send_to_response().length());
                            std::cout << strerror(errno) << std::endl;

                            clients[id].set_status(POST_ok);
                        }
						else // is dir
						{
							std::cout << "ISDIR\n";
							std::string route = "." + clients[id].request.get_referer() + "NEW_FILE";
							file.open(route.c_str(), std::ios::app);
							clients[id].set_status(POST_ok);
						}
                    }
					//if (post)
					{
						// 경로를찾아서 경로가 파일인지 디렉토리인지 아니면 없는지 opendir, readdir
						// 파일 -> 그 파일에 덮어씌우기?
						// 디렉토리 -> 그 경로에 뉴 파일 쓰기
						// 없으면 -> 마지막 경로 이름으로 파일 쓰기 /하이/바이

						// 경로--request.referer
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
			// std::cout << kq.event_list[i].filter << std::endl;
			// std::cout << EVFILT_WRITE << std::endl;
			// std::cout << clients[id].get_server_sock() << std::endl;
			// std::cout << Config.v_server[1].get_socket_fd() << std::endl;
			// std::cout << clients[id].get_status()  << std::endl;
			
			if (kq.event_list[i].filter == EVFILT_WRITE && \
				clients[id].get_server_sock() == Config.v_server[1].get_socket_fd() && \
				clients[id].get_status() > 5)
			{
				std::cout << "accept WRITE Event / ident :" << id << std::endl;
				if (clients[id].get_status() > 5)// && server_it->request.get_host() != "")
				{
					if (clients[id].request.referer.find("favicon.ico") == std::string::npos && clients[id].request.get_method() == "GET")
					{
						if (Config.v_server[clients[id].get_server_id()].get_autoindex() == "on") // location on?
						{
							std::vector<std::string> root;
							std::vector<Location>::iterator it = Config.v_server[clients[id].get_server_id()].v_location.begin();
							for (; it != Config.v_server[clients[id].get_server_id()].v_location.end(); it++) ////// why root no?
							{
								// std::cout << "root:" << it->location << std::endl;
								clients[id].response.set_autoindex(it->location);
								// root.push_back(it->location);
							}
						}
						clients[id].response.set_header(200, clients[id].response.response_str, ""); // OK
					}
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
			// std::cout << "while..\n";
		}
	}
	exit(0);
	return 0;
}
