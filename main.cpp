#include "./includes/Client.hpp"
#include "./includes/Fd.hpp"
#include "./includes/Kqueue.hpp"
#include "./includes/Webserv.hpp"
#include "./includes/ParseUtils.hpp"
#include <dirent.h>
#include <errno.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
	Webserv webserv;
	Config Config;
	std::vector<std::string> vec_attr;
	std::string default_conf = "./conf/default.conf";
	std::string default_mime = "./setting/mime.types";
	if (argc == 2)
		split_config(remove_annotaion(argv[1]), vec_attr);
	else if (argc == 1)
		split_config(remove_annotaion(const_cast<char *>(default_conf.c_str())), vec_attr);
	else
	{
		std::cerr << "input error argc-(" << argc << ")\n";
		exit(0);
	}
	Config.config_parsing(vec_attr);
	Config.server_check();
	std::string &mime = webserv.mime_read(default_mime);
	webserv.mime_parsing(mime);
	// std::cout << "________________________________\n";
	// Config.v_server[1].print_all();
	// std::cout << "________________________________\n";
	// Config.print_all();

	webserv.ready_webserv(Config);
	std::map<int, Client> clients;
	// webserv.get_kq().setting();
	webserv.get_kq().set_kq_fd(kqueue());
	for (std::vector<Server>::iterator it = const_cast<std::vector<Server> &>(Config.get_v_server()).begin(); it != Config.get_v_server().end(); it++)
	{
		// std::cout << "event-" << it->get_socket_fd() << std::endl;
		fcntl(it->get_socket_fd(), F_SETFL, O_NONBLOCK);
		change_events(webserv.get_kq().get_change_list(), it->get_socket_fd(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	}
	while (1)
	{
		std::string str_buf;

		int num_of_event = webserv.get_kq().set_event();
		for (int i = 0; i < num_of_event; i++)
		{
			int id = webserv.get_kq().get_event_list()[i].ident;
			// std::cout << "event id:" << id << \
			// 	" , event filter:" << webserv.get_kq().get_event_list()[i].filter << \
			// 	" , status:" << clients[id].get_status() << std::endl;
			if (webserv.get_kq().get_event_list()[i].flags & EV_ERROR)
			{
				// clients[id]
				std::cerr << "EV_ERROR!\n";

				if (webserv.is_client(Config, id))
				{
					if (clients[id].get_pid() == -1)
					{
						if (clients[id].get_request().get_requests().size() > 0)
							fclose(const_cast<FILE *>(clients[id].get_fp())); // fclose by jwoo
						if (clients[id].get_response().get_response_str().length() > 0)
							close(clients[id].get_write_fd());
					}
				}
				webserv.set_error_page(clients, id, 500);
				// close(webserv.get_kq().get_event_list()[i].ident);
				continue;
			}
			else if (webserv.get_kq().get_event_list()[i].filter == EVFILT_READ)
			{
				std::cout << "accept READ Event / ident :" << id << std::endl;
				if (clients[id].get_status() == need_to_GET_read || clients[id].get_status() == need_to_is_file_read ||
					clients[id].get_status() == need_error_read)
				{
					std::cout << "FILE READ, id:" << id << " ,read_id:" << clients[id].get_read_fd() << "\n"; // open->read->write fopen->fread->fwrite
					
					// FILE *file_ptr = fdopen(id, "r");
					FILE *file_ptr;
					file_ptr = fdopen(id, "r");
					int valfread = 0;
					std::string fread_str;
					char buff[1024];
					memset(buff, 0, 1024);
					std::cout << "fp: " << file_ptr << std::endl;
					while ((valfread = fread(buff, sizeof(char), 1023, file_ptr)) > 0)
					{
						if (valfread < 0)
						{
							std::cerr << "fread error\n";
							break;
						}
						buff[valfread] = 0;
						fread_str.append(buff, valfread);
					}
					
					fclose(file_ptr);
					int read_fd = clients[id].get_read_fd(); //
					clients[read_fd].get_response().set_response_str(fread_str);
					if (clients[id].get_status() == need_error_read)
						clients[read_fd].set_status(error_read_ok);
					else if (clients[id].get_status() == need_to_is_file_read)
						clients[read_fd].set_status(is_file_read_ok);
					else
						clients[read_fd].set_status(GET_read_ok);
					std::cout << ", id: " << read_fd << " status: " << clients[read_fd].get_status() << std::endl;
					close(id);
					clients.erase(id);

					// if (file_ptr == NULL)
					// int valfread = 0;
					// std::string fread_str;
					// char buff[1024];
					// memset(buff, 0, 1024);
					// while ((valfread = fread(buff, sizeof(char), 1023, file_ptr)) > 0)
					// {
					// 	if (valfread < 0)
					// 	{
					// 		std::cerr << "fread error\n";
					// 		break;
					// 	}
					// 	buff[valfread] = 0;
					// 	fread_str.append(buff, valfread);
					// }
					
					// // fclose(file_ptr);
					// close(id);
					// int read_fd = clients[id].get_read_fd(); //
					// clients[read_fd].get_response().set_response_str(fread_str);
					// if (clients[id].get_status() == need_error_read)
					// 	clients[read_fd].set_status(error_read_ok);
					// else if (clients[id].get_status() == need_to_is_file_read)
					// 	clients[read_fd].set_status(is_file_read_ok);
					// else
					// 	clients[read_fd].set_status(GET_read_ok);
					// std::cout << ", id: " << read_fd << " status: " << clients[read_fd].get_status() << std::endl;
					// clients.erase(id);
				}
				else if (clients[id].get_status() == need_to_cgi_read) // 이벤트 주체가 READ open // file read->fread
				{
					int status;
					int ret = waitpid(clients[id].get_pid(), &status, WNOHANG);
					if (ret == 0)
						continue;
					else if (WIFSIGNALED(status) == true)
					{
						// cgi error
						clients.erase(id);
						close(id);
						continue;
					}
					else if (WIFEXITED(status) != true)
					{
						continue;
					}
					std::cerr << "FILE READ, id:" << id << " ,read_id:" << clients[id].get_read_fd() << "\n"; // open->read->write fopen->fread->fwrite
					int valread = 0;
					std::string read_str;
					char buff[1024];
					// int valread = recv(acc_socket, read_str, 1024, 0);
					memset(buff, 0, 1024);
					while ((valread = read(id, buff, 1023)) > 0)
					{ // read
						buff[valread] = 0;
						read_str.append(buff, valread);
					}
					// std::cerr << "**read_str:" << read_str << "\n\n";
					if (valread < 0)
					{
						std::cerr << "read_error!\n";
						exit(0);
					}
					int read_fd = clients[id].get_read_fd(); //

					clients[read_fd].header_parsing(read_str);
					// std::cout << "read)_for)open:" << read_str << std::endl;
					clients[read_fd].get_response().set_response_str(read_str);

					clients[read_fd].set_status(cgi_read_ok);
					// std::out << "READ_ok\n";
					close(id);
					clients.erase(id);
				}
				else if (webserv.find_server(Config, clients[id], id)) // 이벤트 주체가 server /////////////////////////////////////////////////
				{
					webserv.accept_add_events(id, const_cast<std::vector<Server> &>(Config.get_v_server())[clients[id].get_server_id()], webserv.get_kq(), clients);
				}
				else if (clients.find(id) != clients.end() && clients[id].get_status() != WAIT) // 이벤트 주체가 client
				{
					if (clients[id].request_parsing(id) == -1)
					{
						webserv.set_error_page(clients, id, 500);
						// clients[id].set_RETURN(404); // 500
						// int open_fd = open("./status_pages/404.html", O_RDONLY);
						// clients[id].set_open_file_name("./status_pages/404.html");
						// if (open_fd < 0)
						// 	std::cerr << "open error - " << clients[id].get_route() << std::endl;
						// clients[open_fd].set_read_fd(id); // event_fd:6 -> open_fd:10  발생된10->6
						// clients[open_fd].set_status(need_error_read);

						// clients[id].set_status(WAIT);
						// fcntl(open_fd, F_SETFL, O_NONBLOCK);
						// change_events(webserv.get_kq().get_change_list(), open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // read event 추가
						break;
					} // requests_ok

					std::cerr << "GO!::::" << clients[id].get_request().get_start_line() << std::endl;
					int server_id = webserv.find_server_id(id, Config, clients[id].get_request(), clients);
					clients[id].set_server_id(server_id);
					if (server_id < 0)
					{
						close(id);
						clients.erase(i);
					}

					if (webserv.check_except(clients, Config, id, server_id) == -1)
						break ;
					if (webserv.check_size(clients, Config, id, server_id) == -1)
						break ;

					if (clients[id].get_request().get_method() == "GET" ||
						clients[id].get_request().get_method() == "DELETE")
					{
						int location_id = webserv.find_location_id(server_id, Config, clients[id].get_request(), clients[id]); // /abc가 있는가?
						std::cout << "lo id : " << location_id << std::endl;
						if (location_id == 404)																				   // is not found
						{
							std::cerr << "set_404" << std::endl;
							webserv.set_error_page(clients, id, 404);
							// clients[id].set_RETURN(404);
							// int open_fd = open("./status_pages/404.html", O_RDONLY);
							// clients[id].set_open_file_name("./status_pages/404.html");
							// if (open_fd < 0)
							// 	std::cerr << "open error - " << clients[id].get_route() << std::endl;
							// clients[open_fd].set_read_fd(id); // event_fd:6 -> open_fd:10  발생된10->6
							// clients[open_fd].set_status(need_error_read);

							// clients[id].set_status(WAIT);
							// fcntl(open_fd, F_SETFL, O_NONBLOCK);
							// change_events(webserv.get_kq().get_change_list(), open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // read event 추가
							break;
						}
						else if (location_id == -1) // dir
						{
							clients[id].set_RETURN(200);
							if (clients[id].get_request().get_method() == "GET")
								clients[id].set_status(ok);
							else
								clients[id].set_status(DELETE_ok);
							break;
						}
						else if (location_id == -2) // is file
						{
							std::cout << "is file\n";
							clients[id].set_RETURN(200);
							// std::out << "get_route:" << clients[id].get_route() << std::endl;
							if (clients[id].get_route().find(".php") != std::string::npos ||
								clients[id].get_route().find(".py") != std::string::npos)
							{
								std::cout << "im cgi!!\n";
								// std::out << "index_root: " << clients[id].get_route() << std::endl;
								// std::out << "cgi-file: " << Config.v_server[server_id].get_cgi_path() << std::endl;
								std::cerr << "cli boundary:" << clients[id].get_request().get_boundary() << std::endl;
								webserv.run_cgi(Config.get_v_server()[server_id], clients[id].get_route(), clients[id]); // envp have to fix
								close(clients[id].get_write_fd());
								clients[clients[id].get_read_fd()].set_read_fd(id);
								clients[clients[id].get_read_fd()].set_status(need_to_cgi_read);
								clients[clients[id].get_read_fd()].set_pid(clients[id].get_pid());
								// std::cout << "clients[clients[" << id << "].read_fd].get_read_fd() :" << clients[clients[id].read_fd].get_read_fd()
								// 		  << std::endl;
								std::cout << "read_fd : " << clients[id].get_read_fd() << std::endl;
								std::cerr << "write_fd : " << clients[id].get_write_fd() << std::endl;
								clients[id].set_status(WAIT);
								fcntl(clients[id].get_read_fd(), F_SETFL, O_NONBLOCK);
								change_events(webserv.get_kq().get_change_list(), clients[id].get_read_fd(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
								break;
							}
							std::string referer = clients[id].get_request().get_referer();
							std::cout << "server_id: " << server_id << ", location_id: " << location_id <<std::endl;
							if (*referer.begin() == '/')
								referer.erase(referer.begin(), referer.begin() + 1);
							std::string root = Config.get_v_server()[server_id].get_root();
							std::cerr << "root: " << root << std::endl;
							if (root != "" && *(root.end() - 1) != '/')
								root += '/';
							clients[id].set_route(root + referer);
							std::cout << "route: " << clients[id].get_route() << std::endl;

							int open_fd = open(('.' + clients[id].get_route()).c_str(), O_RDONLY);
							std::cout << "route: " << '.' + clients[id].get_route() << std::endl;
							clients[id].set_open_file_name('.' + clients[id].get_route());
							if (open_fd < 0)
							{
								std::cerr << "set_404" << std::endl;
								webserv.set_error_page(clients, id, 404);
								// std::cerr << "open error - ." << clients[id].get_route() << std::endl;
								// int open_fd = open("./status_pages/404.html", O_RDONLY);
								// clients[id].set_open_file_name("./status_pages/404.html");
								// if (open_fd < 0)
								// {
								// 	std::cerr << "open error - ." << clients[id].get_route() << std::endl;
								// 	exit(-1);
								// }
								// clients[open_fd].set_read_fd(id); // event_fd:6 -> open_fd:10  발생된10->6
								// clients[open_fd].set_status(need_error_read);

								// clients[id].set_status(WAIT);
								// fcntl(open_fd, F_SETFL, O_NONBLOCK);
								// change_events(webserv.get_kq().get_change_list(), open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // read event 추가
								break;
							}
							if (clients[id].get_request().get_method() == "DELETE")
							{
								clients[id].set_status(DELETE_ok);
								close(open_fd);
								break;
							}
							clients[open_fd].set_read_fd(id); // event_fd:6 -> open_fd:10  발생된10->6
							clients[open_fd].set_status(need_to_is_file_read);

							clients[id].set_status(WAIT);
							fcntl(open_fd, F_SETFL, O_NONBLOCK);
							change_events(webserv.get_kq().get_change_list(), open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // read event 추가
							break;
						}
						clients[id].set_location_id(location_id);
						int stat = Config.get_v_server()[server_id].get_v_location()[location_id].get_redi_status();
						if (stat > 0) // rediraction!!
						{
							std::cerr << "redirection~\n";
							clients[id].set_RETURN(stat);
							clients[id].set_redi_root(Config.get_v_server()[server_id].get_v_location()[location_id].get_redi_root());
							clients[id].set_status(redi_write);
							break;
						}
						std::string index = Config.get_v_server()[server_id].get_v_location()[location_id].get_index();
						std::cout << "index: " << index << std::endl;
						if (Config.get_v_server()[server_id].get_v_location()[location_id].get_index() != "" &&
							(index.find("php") == std::string::npos && index.find("py") == std::string::npos))
						{
							std::cerr << "is index!\n";
							clients[id].set_RETURN(200);
							std::string root = Config.get_v_server()[server_id].get_v_location()[location_id].get_root();
							std::cout << "root[" << root << "]" << std::endl;
							if (root == "/" || root == "" || *(root.end() - 1) == '/')
								root.pop_back();
							std::cout << "root[" << root << "]" << std::endl;
							if (*index.begin() == '/')
								clients[id].set_route(root + index);
							else
								clients[id].set_route(root + '/' + index);
							std::cout << "route: ." << clients[id].get_route() << std::endl;

							int open_fd = open(('.' + clients[id].get_route()).c_str(), O_RDONLY);
							clients[id].set_open_file_name('.' + clients[id].get_route());
							if (open_fd == -1)
							{
								std::cerr << "set_404" << std::endl;
								webserv.set_error_page(clients, id, 404);
								// std::cerr << "open error - ." << clients[id].get_route() << std::endl;
								// int open_fd = open("./status_pages/404.html", O_RDONLY);
								// clients[id].set_open_file_name("./status_pages/404.html");
								// if (open_fd < 0)
								// 	std::cerr << "open error - ." << clients[id].get_route() << std::endl;
								// clients[open_fd].set_read_fd(id); // event_fd:6 -> open_fd:10  발생된10->6
								// clients[open_fd].set_status(need_error_read);

								// clients[id].set_status(WAIT);
								// fcntl(open_fd, F_SETFL, O_NONBLOCK);
								// change_events(webserv.get_kq().get_change_list(), open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // read event 추가
								break;
							}

							clients[open_fd].set_read_fd(id); // event_fd:6 -> open_fd:10  발생된10->6
							clients[open_fd].set_status(need_to_GET_read);
							// clients[id].set_status(WAIT);
							fcntl(open_fd, F_SETFL, O_NONBLOCK);
							change_events(webserv.get_kq().get_change_list(), open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // read event 추가
							std::cout << "open index ok!, open_fd: " << open_fd << ", clients[open_fd].set_read_fd: " << id << std::endl;
							break;
						}
						else if ((index.find("php") == std::string::npos && index.find("py") == std::string::npos))
						{
							clients[id].set_RETURN(200);
							clients[id].set_status(ok);
						}
						else /////////////////// cgi
						{
							// std::out << "im cgi!!\n";
							clients[id].set_RETURN(201);
							std::string root = '.' + Config.get_v_server()[server_id].get_v_location()[location_id].get_root();
							if (root != "" && *(root.end() - 1) != '/')
								root += '/';
							std::string index_root = root + Config.get_v_server()[server_id].get_v_location()[location_id].get_index();
							// std::out << "index_root: " << index_root << std::endl;
							// std::out << "cgi-file: " << Config.v_server[server_id].get_cgi_path() << std::endl;
							webserv.run_cgi(Config.get_v_server()[server_id], index_root, clients[id]); // envp have to fix
							close(clients[id].get_write_fd());
							clients[clients[id].get_read_fd()].set_read_fd(id);
							clients[clients[id].get_read_fd()].set_status(need_to_cgi_read);
							clients[clients[id].get_read_fd()].set_pid(clients[id].get_pid());
							// std::cout << "clients[clients[" << id << "].read_fd].get_read_fd() :" << clients[clients[id].read_fd].get_read_fd()
							// 		  << std::endl;
							// std::cout << "read_fd : " << clients[id].read_fd << std::endl;
							fcntl(clients[id].get_read_fd(), F_SETFL, O_NONBLOCK);
							clients[id].set_status(WAIT);
							change_events(webserv.get_kq().get_change_list(), clients[id].get_read_fd(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
						}
					}


					////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
					else if (clients[id].get_request().get_method() == "POST") /////************ post ***************** ////////////////////////////////////////////////////////////
					{
						int is_dir = webserv.is_dir(Config.get_v_server()[server_id], clients[id].get_request(), clients[id]);
						if (is_dir == 1) // is dir
						{
							std::cout << "ISDIR\n";
							// clients[id].RETURN = 201;
							std::string referer = clients[id].get_request().get_referer();
							if (*(referer.end() - 1) != '/')
								referer += '/';
							std::string route = "." + clients[id].get_request().get_referer() + "NEW_FILE";
							FILE *file_ptr;
							int i = 0;
							while ((file_ptr = fopen((route + std::to_string(i)).c_str(), "r")) != NULL)
							{
								fclose(file_ptr);
								// if (fclose(file_ptr) == EOF)
								// 	error_exit("fclose");
								i++;
							}
							// fclose(file_ptr); //아직 fdleak
							clients[id].set_open_file_name(route + std::to_string(i));
							int open_fd = open((route + std::to_string(i)).c_str(), O_RDWR | O_CREAT | O_APPEND | O_SYNC, 0777);
							if (open_fd < 0)
							{
								std::cerr << "set_404" << std::endl;
								webserv.set_error_page(clients, id, 404);
								break;
								// clients[id].set_open_file_name("404");
								// std::cerr << "open error - " << route << std::endl;
							}
							std::cout << "POST-my fd::" << id << ", open fd::" << open_fd << std::endl;
							clients[open_fd].set_status(need_to_POST_write);
							clients[open_fd].set_write_fd(id);
							std::cerr << "referer: " << clients[id].get_request().get_referer() << std::endl;
							std::cerr << "post_filename: " << clients[id].get_request().get_post_filename() << std::endl;
							std::cerr << "post_content_type: " << clients[id].get_request().get_post_content_type() << std::endl;
							// if (clients[id].get_request().get_referer().find("php") != std::string::npos ||
							// 	clients[id].get_request().get_referer().find("py") != std::string::npos)
							// {
							// 	// cgi
							// 	clients[id].RETURN = 200;
							// 	std::string index_root = route + std::to_string(i);
							// 	webserv.run_cgi(Config.v_server[server_id], index_root, clients[id]);
							// 	clients[clients[id].read_fd].pid = clients[id].pid;
							// 	clients[clients[id].read_fd].set_read_fd(id);
							// 	// clients[clients[id].write_fd].set_write_fd(id); 생각해보니 필요없음
							// 	clients[clients[id].read_fd].set_status(need_to_cgi_write);
							// 	std::cout << "clients[clients[" << id << "].read_fd].get_read_fd() :" << clients[clients[id].read_fd].get_read_fd()
							// 			  << std::endl;
							// 	std::cout << "read_fd : " << clients[id].read_fd << std::endl;
							// 	change_events(webserv.get_kq().change_list, clients[id].read_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // cgi result 읽기
							// 	change_events(webserv.get_kq().change_list, clients[id].write_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0,
							// 				  NULL); // cgi에 post_body 쓰기
							// }
							// else
								clients[open_fd].get_request().set_post_body(clients[id].get_request().get_post_body());

							clients[id].set_status(WAIT);
							fcntl(open_fd, F_SETFL, O_NONBLOCK);
							change_events(webserv.get_kq().get_change_list(), open_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL); // write event 추가
						}
						else ///////////////////////////////////// POST
						{
							std::cout << "not dir\n";
							std::string route = "." + clients[id].get_request().get_referer();
							if (clients[id].get_request().get_referer().find("php") != std::string::npos ||
								clients[id].get_request().get_referer().find("py") != std::string::npos)
							{
								// POST CGI
								clients[id].set_RETURN(200);
								std::string index_root = route + std::to_string(i);
								std::cerr << "cli boundary:" << clients[id].get_request().get_boundary() << std::endl;
								webserv.run_cgi(Config.get_v_server()[server_id], index_root, clients[id]);
								clients[clients[id].get_read_fd()].set_pid(clients[id].get_pid());
								clients[clients[id].get_write_fd()].set_pid(clients[id].get_pid());
								clients[clients[id].get_read_fd()].set_read_fd(id);
								clients[clients[id].get_write_fd()].set_write_fd(id);
								clients[clients[id].get_read_fd()].get_request().set_post_body(clients[id].get_request().get_post_body());
								clients[clients[id].get_write_fd()].get_request().set_post_body(clients[id].get_request().get_post_body()); // body
								clients[clients[id].get_write_fd()].get_request().set_header(clients[id].get_request().get_header()); // header
								clients[clients[id].get_write_fd()].set_status(need_to_cgi_write);

								std::cerr << "clients[read_fd:" << clients[id].get_read_fd() << "].set_read_fd(" << id << ")\n";
								std::cerr << "clients[write_fd:" << clients[id].get_write_fd() << "].set_write_fd(" << id << ")\n";
								std::cerr << "read_fd : " << clients[id].get_read_fd() << ", write_fd : " << clients[id].get_write_fd() <<std::endl;

								clients[id].set_status(WAIT);
								fcntl(clients[id].get_write_fd(), F_SETFL, O_NONBLOCK);
								fcntl(clients[id].get_read_fd(), F_SETFL, O_NONBLOCK);
								change_events(webserv.get_kq().get_change_list(), clients[id].get_write_fd(), EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL); // cgi에 post_body 쓰기
								change_events(webserv.get_kq().get_change_list(), clients[id].get_read_fd(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
								break;
							}
							int open_fd;
							if (clients[id].get_RETURN() == 200)
								open_fd = open(route.c_str(), O_RDWR | O_APPEND | O_SYNC, 0777);
							else
								open_fd = open(route.c_str(), O_RDWR | O_CREAT | O_APPEND | O_SYNC, 0777);
							clients[id].set_open_file_name(route);
							if (open_fd < 0)
							{
								// clients[id].set_open_file_name("404");
								std::cerr << "open error - " << route << std::endl;
								std::cerr << "set_404" << std::endl;
								webserv.set_error_page(clients, id, 404);
								break;
							}
							std::cout << "POST-my fd!!" << id << ", open fd!!" << open_fd << std::endl;
							clients[open_fd].set_status(need_to_POST_write);
							clients[open_fd].set_write_fd(id);
							clients[open_fd].get_request().set_post_body(clients[id].get_request().get_post_body());

							clients[id].set_status(WAIT);
							fcntl(open_fd, F_SETFL, O_NONBLOCK);
							change_events(webserv.get_kq().get_change_list(), open_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL); // write event 추가
						}
					} // end POST
				}
			} // end FILT READ
			else if (webserv.get_kq().get_event_list()[i].filter == EVFILT_WRITE)
			{
				// std::cerr << "id: " << id << ", status: " << 
				if (clients[id].get_status() == need_to_POST_write) //////////////////////////////// file에다가 write
				{
					std::cerr << "im POST(file) write!!\n";
					FILE *fp = fdopen(id, "wb");

					// write(id, clients[id].get_request().post_body.c_str(), clients[id].get_request().post_body.length());
					fwrite(clients[id].get_request().get_post_body().c_str(), sizeof(char), clients[id].get_request().get_post_body().length(), fp);
					std::cerr << "write-" << id << ":\n" << clients[id].get_request().get_post_body() << std::endl;
					clients[clients[id].get_write_fd()].set_status(POST_ok);

					fclose(fp);
					close(id);
					clients.erase(id);
					break;
				}
				else if (clients[id].get_status() == need_to_cgi_write) // CGI에다가 write
				{
					std::cerr << "im POST-CGI write!! id: " << id << ", origin is: " << clients[id].get_write_fd() << ".\n"; 
					FILE *fp = fdopen(id, "wb");

					// fwrite(clients[id].get_request().get_header().c_str(), sizeof(char), clients[id].get_request().get_header().length(), fp);
					fwrite(clients[id].get_request().get_post_body().c_str(), sizeof(char), clients[id].get_request().get_post_body().length(), fp);
					// write(id, clients[id].get_request().get_post_body().c_str(), clients[id].get_request().get_post_body().length());
					// write(id, clients[id].get_request().get_header().c_str(), clients[id].get_request().get_header().length());
					// std::cerr << "write-" << id << " +++++++++++++++++++++++++++++++++++\n" << clients[id].get_request().get_post_body() << "\n+++++++++++++++++++++++++++++++++++++++\n";
					int read_fd = clients[clients[id].get_write_fd()].get_read_fd();
					clients[read_fd].set_status(need_to_cgi_read);
					clients[id].set_status(WAIT);
					// change_events(webserv.get_kq().get_change_list(), read_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // cgi result 읽기

					fclose(fp);
					close(id);
					clients.erase(id);
					std::cerr << "post cgi end, go to fd: " << read_fd << "\n";
					break;
				}
				else if (clients[id].get_status() >= WRITE_LINE)
				{
					std::cout << "accept WRITE Event / ident :" << id << ",status: " << clients[id].get_status() << std::endl;
					webserv.set_content_type(clients[id], webserv);
					if (clients[id].get_status() >= WRITE_LINE) // && server_it->request.get_host() != "")
					{
						if (clients[id].get_status() == redi_write)
						{
							clients[id].get_response().set_header(clients[id].get_RETURN(), clients[id].get_redi_root(), clients[id].get_content_type());
						}
						else if (clients[id].get_status() == error_read_ok)
						{
							clients[id].get_response().set_header(clients[id].get_RETURN(), "", clients[id].get_content_type());
						}
						else if (clients[id].get_status() == is_file_read_ok)
						{
							std::cout << "is file\n";
							webserv.set_indexing(clients[id], id);
							clients[id].get_response().set_header(clients[id].get_RETURN(), "", clients[id].get_content_type());
							// std::cerr << "response :: " << clients[id].get_response().get_send_to_response().c_str() << std::endl;
						}
						else if (clients[id].get_status() == cgi_read_ok)
						{
							clients[id].get_response().set_header(200, clients[id].get_request().get_header(), clients[id].get_content_type());
							std::cerr << "CGI response :: " << clients[id].get_response().get_send_to_response().c_str() << std::endl;
						}
						else if (clients[id].get_status() == POST_ok)
						{
							// std::cer << "POST RETURN:" << clients[id].get_RETURN() << std::endl;
							clients[id].get_response().set_header(clients[id].get_RETURN(), "", clients[id].get_content_type());
							// std::cer << "POST response :: " << clients[id].get_response().get_send_to_response().c_str() << std::endl;
						}
						else if (clients[id].get_status() == DELETE_ok)
						{
							clients[id].get_response().set_header(200, "", "DELETE");
						}
						else if (clients[id].get_location_id() > 0)
						{
							clients[id].get_response().set_header(clients[id].get_RETURN(), "", clients[id].get_content_type());
						}
						else if (clients[id].get_request().get_referer().find("favicon.ico") == std::string::npos && clients[id].get_request().get_method() == "GET")
						{																			  // clients[id].location_id != -1 &&
							if (Config.get_v_server()[clients[id].get_server_id()].get_autoindex() == "on") // location on?
							{
								std::cerr << "auto indexing~!\n";
								DIR *dir;
								int is_root = 0;
								struct dirent *ent;
								std::cout << "route: " << '.' + clients[id].get_request().get_referer() << std::endl;
								if (clients[id].get_location_id() < 0 && clients[id].get_is_file() != 1) // /abc
									dir = opendir(('.' + clients[id].get_request().get_referer()).c_str());
								else if (clients[id].get_request().get_referer() == "/")
								{
									is_root = 1;
									dir = opendir(('.' + clients[id].get_request().get_referer()).c_str());
								}
								else
								{
									webserv.set_error_page(clients, id, 404);
									break;
								}
								if (dir != NULL)
								{
									/* print all the files and directories within directory */
									clients[id].get_response().get_response_str() += "<!DOCTYPE html>\n";
									clients[id].get_response().get_response_str() += "<html>\n";
									clients[id].get_response().get_response_str() += "<head>\n</head>\n";
									clients[id].get_response().get_response_str() += "<body>\n";
									clients[id].get_response().get_response_str() += "<h1>Index of ." + clients[id].get_request().get_referer() + "</h1>\n";
									clients[id].get_response().get_response_str() += "</a><br>\n";
									while ((ent = readdir(dir)) != NULL)
										clients[id].get_response().set_autoindex(clients[id].get_request().get_referer(), ent->d_name, is_root);
									closedir(dir);
								}
								else
								{
									/* could not open directory */
									perror("");
									return EXIT_FAILURE;
								}
							}
							clients[id].get_response().set_header(200, "", clients[id].get_content_type());
						}
						else
						{
							std::cout << "are you get_read_ok?\n";
							clients[id].get_response().set_header(200, "", clients[id].get_content_type()); // ok
						}

						FILE *fp = fdopen(id, "wb");
						if (clients[id].get_status() == cgi_read_ok)
						{
							write(id, clients[id].get_response().get_send_to_response().c_str(), clients[id].get_response().get_send_to_response().length());
							// fwrite(clients[id].get_response().get_send_to_response().c_str(), sizeof(char), clients[id].get_response().get_send_to_response().length(), fp);
							std::cerr << "END::::" << clients[id].get_request().get_start_line() << std::endl;
							close(id);
							// fclose(fp);
							clients.erase(id);
							break;
						}
						// std::cerr << "response :: " << clients[id].get_response().get_send_to_response().c_str() << std::endl;
						int count = 0;

						// std::cout << "send_to_res\n" << clients[id].get_response().get_send_to_response() << std::endl;
						if (fp == NULL)
						{
							std::cerr << "ㅇㅛ기요요요요요기\n\n" << std::endl;
							// webserv.set_error_page(clients, id, 500);
							change_events(webserv.get_kq().get_change_list(), id, EVFILT_WRITE, EV_DELETE | EV_ENABLE, 0, 0, NULL); // write event 추가
							change_events(webserv.get_kq().get_change_list(), id, EVFILT_WRITE, EV_DELETE | EV_ENABLE, 0, 0, NULL); // write event 추가
							// fclose(fp);
							// close(id);
							clients.erase(id);
							break;
						}
						fwrite(clients[id].get_response().get_send_to_response().c_str(), sizeof(char),
							clients[id].get_response().get_send_to_response().size(), fp);
						std::cerr << "END::::" << clients[id].get_request().get_start_line() << std::endl;

						fclose(fp);
						close(id);
						clients.erase(id);
					}
				}
			}
		}
	}
	exit(0);
	return 0;
}
