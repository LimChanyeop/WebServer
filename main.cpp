#include "./includes/Client.hpp"
#include "./includes/Fd.hpp"
#include "./includes/Kqueue.hpp"
#include "./includes/Webserv.hpp"
#include <dirent.h>
#include <errno.h>
#include <netinet/in.h>

int find_server(Config Config, Client &client, int id) {
	std::vector<Server>::iterator it;
	for (it = Config.v_server.begin(); it != Config.v_server.end(); it++) {
		std::cout << "listen:" << it->get_socket_fd() << " vs " << id << std::endl;
		if (it->get_socket_fd() == id) {
			std::cout << "It is server!!\n";
			client.set_server_id(it - Config.v_server.begin());
			client.set_server_sock(id);
			// std::cout << client.get_server_id() << std::endl;
			// std::cout << client.get_server_sock() << std::endl;
			return 1; // 드디어 어떤 서버인지 찾음
		}
	}
	if (it == Config.v_server.end()) {
		// std::cerr << "Can not found Server\n";
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
	for (std::vector<Server>::iterator it = Config.v_server.begin(); it != Config.v_server.end(); it++) {
		// std::cout << "event-" << it->get_socket_fd() << std::endl;
		change_events(kq.change_list, it->get_socket_fd(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	}
	while (1)
	{
		std::string str_buf;

		// std::cout << "waiting for new connection...\n";
		int num_of_event = kq.set_event();
		for (int i = 0; i < num_of_event; i++)
		{
			int id = kq.event_list[i].ident;
			std::cout << "event id:" << id << \
				" , event filter:" << kq.event_list[i].filter << \
				" , status:" << clients[id].get_status() << std::endl;
			if (kq.event_list[i].filter == EVFILT_READ && clients[id].status != WAIT)
			{
				std::cout << "accept READ Event / ident :" << id << std::endl;
				if (clients[id].get_status() == need_to_GET_read || clients[id].get_status() == need_to_is_file_read ||
					clients[id].get_status() == need_to_cgi_read || clients[id].get_status() == need_error_read) // 이벤트 주체가 READ open
				{
					std::cout << "FILE READ\n";
					FILE *file_ptr = fdopen(id, "r");
					fseek(file_ptr, 0, SEEK_END);
					int file_size = ftell(file_ptr);
					rewind(file_ptr);
					int seek = 0;
					int valfread = 0;
					char READ[1024] = {0};
					std::string read_for_open;
					// int valread = recv(acc_socket, read_for_open, 1024, 0);

					while (seek < file_size) {
						seek += fread(READ, sizeof(char), 1023, file_ptr);
						read_for_open += READ;
					}
					int read_fd = clients[id].get_read_fd(); //
					clients[read_fd].response.response_str = read_for_open;
					if (clients[id].get_status() == need_error_read)
						clients[read_fd].set_status(error_read_ok);
					else if (clients[id].get_status() == need_to_is_file_read)
						clients[read_fd].set_status(is_file_read_ok);
					else if (clients[id].get_status() == need_to_cgi_read)
						clients[read_fd].set_status(cgi_read_ok);
					else
						clients[read_fd].set_status(GET_read_ok);
					std::cout << "READ_ok\n";
					close(id);
					clients.erase(id);
				} else if (find_server(Config, clients[id], id)) // 이벤트 주체가 server
				{
					std::cout << "clients[" << id << "].get_server_id():" << clients[id].get_server_id() << std::endl;
					webserv.accept_add_events(id, Config.v_server[clients[id].get_server_id()], kq, clients);
				} else if (clients.find(id) != clients.end()) // 이벤트 주체가 client
				{
					// server_it = webserv.find_server_it(Config, clients[id]);
					clients[id].request_parsing(id);
					clients[id].set_status(request_ok); // requests_ok
					std::cout << "PORT:" << clients[id].request.get_host() << std::endl;
					// clients[id].set_status(ok);
					int server_id = webserv.find_server_id(id, Config, clients[id].request, clients);
					std::cout << "server_id:" << clients[id].get_server_id() << ", req host:" << clients[id].request.get_host() << std::endl;
					// int server_id = server_it - Config.v_server.begin();

					if (clients[id].request.get_method() == "GET")
					{
						int location_id = webserv.find_location_id(server_id, Config, clients[id].request, clients[id]); // /abc가 있는가?
						// std::cout << server_id << " " << location_id << std::endl;
						if (location_id == 404)																		  // is not found
						{
							std::cout << "404\n";
							int open_fd = open("./status_pages/404.html", O_RDONLY);
							if (open_fd < 0)
								std::cerr << "open error - " << clients[id].get_route() << std::endl;
							std::cout << "404-my fd::" << id << ", open fd::" << open_fd << std::endl;
							clients[open_fd].set_read_fd(id); // event_fd:6 -> open_fd:10  발생된10->6
							clients[open_fd].set_status(need_error_read);

							clients[id].set_status(WAIT);
							change_events(kq.change_list, open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // read event 추가
							break;
						} else if (location_id == -1) // is file
						{
							std::cout << "is file\n";
							std::string referer = clients[id].request.get_referer();
							if (*referer.begin() == '/')
								referer.erase(referer.begin(), referer.begin() + 1);
							std::string root = Config.v_server[server_id].v_location[location_id].get_root();
							if (*(root.end() - 1) == '/')
								root.erase(root.end() - 1, root.end());
							clients[id].set_route("." + root + "/" + referer);
							std::cout << "route-" << clients[id].get_route() << std::endl;

							int open_fd = open(clients[id].get_route().c_str(), O_RDONLY);
							if (open_fd < 0) {
								std::cerr << "open error - " << clients[id].get_route() << std::endl;
								int open_fd = open("./status_pages/404.html", O_RDONLY);
								if (open_fd < 0)
									std::cerr << "open error - " << clients[id].get_route() << std::endl;
								std::cout << "isfile-my fd::" << id << ", open fd::" << open_fd << std::endl;
								clients[open_fd].set_read_fd(id); // event_fd:6 -> open_fd:10  발생된10->6
								clients[open_fd].set_status(need_error_read);

								clients[id].set_status(WAIT);
								change_events(kq.change_list, open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // read event 추가
								break;
							}
							clients[open_fd].set_read_fd(id); // event_fd:6 -> open_fd:10  발생된10->6
							clients[open_fd].set_status(need_to_is_file_read);

							clients[id].set_status(WAIT);
							change_events(kq.change_list, open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // read event 추가
							break;
						}
						clients[id].set_location_id(location_id);
						std::cout << "there is index?" << Config.v_server[server_id].v_location[location_id].get_index() << std::endl;
						if (Config.v_server[server_id].v_location[location_id].get_index() != "" &&
							(clients[id].request.get_referer().find("php") == std::string::npos ||
							 clients[id].request.get_referer().find("py") == std::string::npos))
						{
							// if (autoindex)
							std::string index = Config.v_server[server_id].v_location[location_id].get_index();
							std::string root = Config.v_server[server_id].v_location[location_id].get_root();
							if (*(root.end() - 1) == '/')
								root.erase(root.end() - 1, root.end());
							clients[id].set_route("." + root + "/" + index);
							//											/View + / + Default.html
							// std::cout << "route: " << clients[id].get_route() << std::endl;

							int open_fd = open(clients[id].get_route().c_str(), O_RDONLY);
							if (open_fd < 0) {
								std::cerr << "open error - " << clients[id].get_route() << std::endl;
								int open_fd = open("./status_pages/404.html", O_RDONLY);
								if (open_fd < 0)
									std::cerr << "open error - " << clients[id].get_route() << std::endl;
								std::cout << "my fd::" << id << ", open fd::" << open_fd << std::endl;
								clients[open_fd].set_read_fd(id); // event_fd:6 -> open_fd:10  발생된10->6
								clients[open_fd].set_status(need_error_read);

								clients[id].set_status(WAIT);
								change_events(kq.change_list, open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // read event 추가
								break;
							}

							std::cout << "fd::" << id << ", open fd::" << open_fd << std::endl;
							clients[open_fd].set_read_fd(id); // event_fd:6 -> open_fd:10  발생된10->6
							clients[open_fd].set_status(need_to_GET_read);
							change_events(kq.change_list, open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // read event 추가
						} else if ((clients[id].request.get_referer().find("php") == std::string::npos ||
							 clients[id].request.get_referer().find("py") == std::string::npos)) {
							clients[id].set_status(ok);
						} else /////////////////// cgi
						{
							int cgi_fd = webserv.run_cgi(Config.v_server[server_id], location_id, clients[id]); // envp have to fix

							clients[cgi_fd].set_read_fd(id);
							change_events(kq.change_list, cgi_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
							clients[cgi_fd].set_status(need_to_cgi_read);
						}
					}
					else if (clients[id].request.get_method() == "POST") {
						int is_dir = webserv.is_dir(Config.v_server[server_id], clients[id].request, clients[id]);
						if (is_dir == 1) // is dir
						{
							std::cout << "ISDIR\n";
							std::string route = "." + clients[id].request.get_referer() + "/NEW_FILE";
							int open_fd = open(route.c_str(), O_RDWR | O_CREAT);
							int i = 0;
							while (open_fd == -1)
							{
								open_fd = open((route + std::to_string(i)).c_str(), O_RDWR | O_CREAT);
								std::cerr << "open error - " << route + std::to_string(i) << std::endl;
								i++;
							}
							if (open_fd < 0)
								std::cerr << "open error - " << route << std::endl;
							std::cout << "POST-my fd::" << id << ", open fd::" << open_fd << std::endl;
							clients[open_fd].set_status(need_to_POST_write);
							clients[open_fd].write_fd = id;
							clients[open_fd].request.post_body = clients[id].request.post_body;

							clients[id].set_status(WAIT);
							change_events(kq.change_list, open_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL); // write event 추가
						}
						else
						{
							std::cout << "not dir\n";
							std::string route = "." + clients[id].request.get_referer();
							int open_fd;
							if (clients[id].RETURN == 200)
								open_fd = open(route.c_str(), O_RDWR);
							else
								open_fd = open(route.c_str(), O_RDWR | O_CREAT);
							if (open_fd < 0)
								std::cerr << "open error - " << route << std::endl;
							std::cout << "POST-my fd!!" << id << ", open fd!!" << open_fd << std::endl;
							clients[open_fd].set_status(need_to_POST_write);
							clients[open_fd].write_fd = id;
							clients[open_fd].request.post_body = clients[id].request.post_body;

							clients[id].set_status(WAIT);
							std::cout << "event add ok~\n";
							// fcntl(open_fd, F_SETFL, O_NONBLOCK);
							change_events(kq.change_list, open_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL); // write event 추가
						}
					}
				}
			} // if /READ

			// std::cout << "clients[" << id << "].get_server_id():" << clients[id].get_server_id() << std::endl;
			if (clients[id].get_server_id() < -1)
				continue;
			if (kq.event_list[i].filter == EVFILT_WRITE && clients[id].get_status() == need_to_POST_write)
			{
				std::cout << "hi! im post write!\n";
				// FILE *fp = fdopen(id, "w");
				// if (fp == NULL) {
				// 	std::cout << "fdopen error" << std::endl;
				// 	continue;
				// }
				// fwrite(clients[id].request.post_body.c_str(), 1, \
				// 	clients[id].request.post_body.length(), fp); // POST write
				write(id, clients[id].request.post_body.c_str(), clients[id].request.post_body.length());
				int write_fd = clients[id].get_write_fd();
				std::cout << "write-" << write_fd << ":" << clients[id].request.post_body << std::endl;
				clients[write_fd].set_status(POST_ok);
				clients.erase(id);
				close(id);
				// fclose(fp);
				break;
				// write(id, clients[id].response.get_send_to_response().c_str(), \
					clients[id].response.get_send_to_response().length());
			} //clients[id].get_server_sock() == Config.v_server[clients[id].get_server_id()].get_socket_fd() &&
			else if (kq.event_list[i].filter == EVFILT_WRITE && \
				clients[id].get_status() >= WRITE_LINE) {
				std::cout << "accept WRITE Event / ident :" << id << std::endl;
				if (clients[id].get_status() >= WRITE_LINE) // && server_it->request.get_host() != "")
				{
					if (clients[id].get_status() == error_read_ok) {
						clients[id].response.set_header(404, "");
					} else if (clients[id].get_status() == is_file_read_ok) {
						clients[id].response.set_header(clients[id].RETURN, "");
						std::cout << "response :: " << clients[id].response.get_send_to_response().c_str() << std::endl;
					} else if (clients[id].get_status() == cgi_read_ok) {
						clients[id].response.set_header(200, "cgi");
						std::cout << "CGI response :: " << clients[id].response.get_send_to_response().c_str() << std::endl;
					} else if (clients[id].get_status() == POST_ok) {
						std::cout << "POST RETURN:" << clients[id].RETURN << std::endl;
						clients[id].response.set_header(clients[id].RETURN, "");
						std::cout << "POST response :: " << clients[id].response.get_send_to_response().c_str() << std::endl;
					}
					else if (clients[id].request.referer.find("favicon.ico") == std::string::npos && clients[id].request.get_method() == "GET") {
						if (Config.v_server[clients[id].get_server_id()].get_autoindex() == "on") // location on?
						{
							std::string root;
							if (Config.v_server[clients[id].get_server_id()].get_autoindex() != "")
								root = Config.v_server[clients[id].get_server_id()].get_autoindex();
							if (Config.v_server[clients[id].get_server_id()].v_location[clients[id].get_location_id()].get_autoindex() != "")
								root = Config.v_server[clients[id].get_server_id()].v_location[clients[id].get_location_id()].get_autoindex();
							std::vector<Location>::iterator it = Config.v_server[clients[id].get_server_id()].v_location.begin();
							for (; it != Config.v_server[clients[id].get_server_id()].v_location.end(); it++) ////// why root no?
							{
								// std::cout << "root:" << it->location << std::endl;
								clients[id].response.set_autoindex(it->location);
								// root.push_back(it->location);
							}
						}
						clients[id].response.set_header(200, "");
					} else
					{
						clients[id].response.set_header(200, ""); // ok
					}
					// FILE *fp = fdopen(id, "wb");
					// if (fp == NULL) {
					// 	std::cout << "fdopen error" << std::endl;
					// 	continue;
					// }
					// std::cout << "response :: " << clients[id].response.get_send_to_response().c_str() << std::endl;
					// int count = fwrite(clients[id].response.get_send_to_response().c_str(), sizeof(char),
					// 				   clients[id].response.get_send_to_response().size(), fp);
					// std::cout << "count:" << count << std::endl;
					write(id, clients[id].response.get_send_to_response().c_str(), clients[id].response.get_send_to_response().length());

					clients.erase(id);
					// fclose(fp);
					close(id);
					// clients[id].request.clear_request();
					// clients[id].response.response_str = "";
					// std::cout << "response..\n\n";
				}
			} // else if /WRITE
			  // std::cout << "while..\n";
		}
	}
	exit(0);
	return 0;
}
