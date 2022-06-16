#include "./includes/Client.hpp"
#include "./includes/Fd.hpp"
#include "./includes/Kqueue.hpp"
#include "./includes/Webserv.hpp"
#include <dirent.h>
#include <errno.h>
#include <netinet/in.h>

void error_exit(std::string error)
{
	std::cerr << error << " error!\n";
	exit(-1);
}

int is_client(Config config, int id)
{
	std::vector<Server>::iterator it = config.v_server.begin();
	for (; it != config.v_server.end(); it++)
	{
		if (it->get_socket_fd() == id)
			return 0;
	}
	return 1;
}

void set_content_type(Client &client, const Webserv &webserv)
{
	std::__1::map<std::__1::string, std::__1::string>::const_iterator it;
	// for (it = webserv.mimes.begin(); it != webserv.mimes.end(); it++)
	// {
	// 	std::cerr << it->first << " = " << it->second << std::endl;
	// }

	if (client.open_file_name == "")
	{
		client.content_type = "text/html";
		return ;
	}
	std::string open_file_name = client.open_file_name;
	int find;
	while ((find = open_file_name.find('/')) != std::string::npos)
	{
		open_file_name.erase(0, find + 1);
	}
	std::string::iterator s_it = open_file_name.begin();
	while (s_it != open_file_name.end() && *s_it != '.')
		s_it++;
	open_file_name.erase(0, s_it - open_file_name.begin() + 1);
	if (open_file_name == "\n")
	{
		client.content_type = "text/plain";
		return ;
	}
	std::map<std::string, std::string>::const_iterator m_it = webserv.mimes.begin();
	while (m_it != webserv.mimes.end())
	{
		// std::cerr << "open_file_name: " << open_file_name << " vs mime: " << m_it->first << std::endl;
		if (m_it->first.find(open_file_name) != std::string::npos)
		{
			// std::cerr << "\n**i found mime !!**\n\n";
			client.content_type = m_it->second;
			return ;
		}
		m_it++;
	}
	if (m_it == webserv.mimes.end())
	{
		// std::cer << "\n*i cant find mime !!*\n\n";
		client.content_type = "text/plain";
	}
	return ;

	if (client.request.referer.find("html")) ////////
		client.content_type = "text/html";
	else if (client.request.referer.find("txt"))
		client.content_type = "text/plain";

	else if (client.request.referer.find("gif"))
		client.content_type = "image/gif";
	else if (client.request.referer.find("png"))
		client.content_type = "image/png";
}

int find_server(Config Config, Client &client, int id)
{
	std::vector<Server>::iterator it;
	for (it = Config.v_server.begin(); it != Config.v_server.end(); it++)
	{
		// std::out << "listen:" << it->get_socket_fd() << " vs " << id << std::endl;
		if (it->get_socket_fd() == id)
		{
			// std::out << "It is server!!\n";
			client.set_server_id(it - Config.v_server.begin());
			client.set_server_sock(id);
			// std::cout << client.get_server_id() << std::endl;
			// std::cout << client.get_server_sock() << std::endl;
			return 1; // 드디어 어떤 서버인지 찾음
		}
	}
	return 0;
}

int main(int argc, char *argv[])
{
	Webserv webserv;
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
	Config Config;
	Config.config_parsing(vec_attr);
	std::string &mime = webserv.mime_read(default_mime);
	webserv.mime_parsing(mime);
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
	while (1)
	{
		std::string str_buf;

		int num_of_event = kq.set_event();
		for (int i = 0; i < num_of_event; i++)
		{
			int id = kq.event_list[i].ident;
			// std::cout << "event id:" << id << \
			// 	" , event filter:" << kq.event_list[i].filter << \
			// 	" , status:" << clients[id].get_status() << std::endl;
			if (kq.event_list[i].flags & EV_ERROR)
			{
				// clients[id]

				if (is_client(Config, id))
				{
					if (clients[id].pid == -1)
					{
						if (clients[id].request.requests.size() > 0)
							close(clients[id].read_fd); // fclose by jwoo
						if (clients[id].response.response_str.length() > 0)
							close(clients[id].write_fd);
					}
				}
				close(kq.event_list[i].ident);
				continue;
			}
			else if (kq.event_list[i].filter == EVFILT_READ)
			{
				// std::out << "accept READ Event / ident :" << id << std::endl;
				if (clients[id].get_status() == need_to_GET_read || clients[id].get_status() == need_to_is_file_read ||
					clients[id].get_status() == need_error_read)
				{
					// std::out << "FILE READ, id:" << id << " ,read_id:" << clients[id].read_fd << "\n"; // open->read->write fopen->fread->fwrite
					FILE *file_ptr = fdopen(id, "r");
					int valfread = 0;
					std::string fread_str;
					char buff[1024];
					memset(buff, 0, 1024);
					while ((valfread = fread(buff, sizeof(char), 1023, file_ptr)) > 0)
					{
						buff[valfread] = 0;
						fread_str.append(buff, valfread);
					}
					if (valfread < 0)
					{
						std::cerr << "fread error\n";
						exit(0);
					}
					int read_fd = clients[id].get_read_fd(); //
					clients[read_fd].response.response_str = fread_str;
					// std::cout << "read_ok : " << fread_str << std::endl;
					if (clients[id].get_status() == need_error_read)
						clients[read_fd].set_status(error_read_ok);
					else if (clients[id].get_status() == need_to_is_file_read)
						clients[read_fd].set_status(is_file_read_ok);
					else
						clients[read_fd].set_status(GET_read_ok);
					// std::out << "READ_ok\n";
					close(id);
					clients.erase(id);
				}
				else if (clients[id].get_status() == need_to_cgi_read) // 이벤트 주체가 READ open // file read->fread
				{
					if (clients[id].get_status() == need_to_cgi_read) // read
					{
						int status;
						int ret = waitpid(clients[id].pid, &status, WNOHANG);
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
					}
					// std::out << "FILE READ, id:" << id << " ,read_id:" << clients[id].read_fd << "\n"; // open->read->write fopen->fread->fwrite
					int read_val = 0;
					std::string read_str;
					char buff[1024];
					// int valread = recv(acc_socket, read_str, 1024, 0);
					memset(buff, 0, 1024);
					while ((read_val = read(id, buff, 1023)) > 0)
					{ // read
						buff[read_val] = 0;
						read_str += buff;
						// std::out << "**read_str:" << read_str << "\n\n";
					}
					if (read_val < 0)
					{
						std::cerr << "read_error!\n";
						exit(0);
					}
					std::string header;
					std::string temp = read_str;
					
					int find;
					if ((find = temp.find("X-Powered-By:")) != std::string::npos) // if cgi -> header parsing
					{
						if ((find = temp.find("<")) != std::string::npos)
						{
							header = temp.erase(find - 1, temp.end() - temp.begin());
							read_str = read_str.erase(0, find);
						}
					}
					// std::out << "header:" << header << std::endl;
					clients[clients[id].read_fd].header = header;

					// std::cout << "read)_for)open:" << read_str << std::endl;
					int read_fd = clients[id].get_read_fd(); //
					clients[read_fd].response.response_str = read_str;
					
					clients[read_fd].set_status(cgi_read_ok);
					// std::out << "READ_ok\n";
					close(id);
					clients.erase(id);
				}
				else if (find_server(Config, clients[id], id)) // 이벤트 주체가 server
				{
					webserv.accept_add_events(id, Config.v_server[clients[id].get_server_id()], kq, clients);
				}
				else if (clients.find(id) != clients.end()) // 이벤트 주체가 client
				{
					if (clients[id].request_parsing(id) == -1)
					{
						change_events(kq.change_list, id, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
						break ;
					}
					clients[id].set_status(request_ok); // requests_ok
					int server_id = webserv.find_server_id(id, Config, clients[id].request, clients);

					if (clients[id].request.get_method() == "GET")
					{
						int location_id = webserv.find_location_id(server_id, Config, clients[id].request, clients[id]); // /abc가 있는가?
						if (location_id == 404) // is not found
						{
							clients[id].RETURN = 404;
							int open_fd = open("./status_pages/404.html", O_RDONLY);
							clients[id].open_file_name = "./status_pages/404.html";
							if (open_fd < 0)
								std::cerr << "open error - " << clients[id].get_route() << std::endl;
							clients[open_fd].set_read_fd(id); // event_fd:6 -> open_fd:10  발생된10->6
							clients[open_fd].set_status(need_error_read);

							clients[id].set_status(WAIT);
							change_events(kq.change_list, open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // read event 추가
							break;
						}
						else if (location_id == -1)
						{
							clients[id].RETURN = 200;
							clients[id].set_status(ok);
							break;
						}
						else if (location_id == -2) // is file
						{
							// std::out << "is file\n";
							clients[id].RETURN = 200;
							// std::out << "get_route:" << clients[id].get_route() << std::endl;
							if (clients[id].get_route().find(".php") != std::string::npos || \
								clients[id].get_route().find(".py") != std::string::npos)
							{
								// std::out << "im cgi!!\n";
								// std::out << "index_root: " << clients[id].get_route() << std::endl;
								// std::out << "cgi-file: " << Config.v_server[server_id].get_cgi_path() << std::endl;
								webserv.run_cgi(Config.v_server[server_id], clients[id].get_route(), clients[id]); // envp have to fix
								close(clients[id].write_fd);
								clients[clients[id].read_fd].set_read_fd(id);
								clients[clients[id].read_fd].set_status(need_to_cgi_read);
								clients[clients[id].read_fd].pid = clients[id].pid;
								// std::cout << "clients[clients[" << id << "].read_fd].get_read_fd() :" << clients[clients[id].read_fd].get_read_fd()
								// 		  << std::endl;
								// std::cout << "read_fd : " << clients[id].read_fd << std::endl;
								change_events(kq.change_list, clients[id].read_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
								break;
							}
							std::string referer = clients[id].request.get_referer();
							if (*referer.begin() == '/')
								referer.erase(referer.begin(), referer.begin() + 1);
							std::string root = Config.v_server[server_id].v_location[location_id].get_root();
							if (root != "" && root != "" && *(root.end() - 1) == '/')
								root.erase(root.end() - 1, root.end());
							clients[id].set_route(root + "/" + referer);

							int open_fd = open(('.' + clients[id].get_route()).c_str(), O_RDONLY);
							clients[id].open_file_name = '.' + clients[id].get_route();
							if (open_fd < 0)
							{
								std::cerr << "open error - ." << clients[id].get_route() << std::endl;
								int open_fd = open("./status_pages/404.html", O_RDONLY);
								clients[id].open_file_name = "./status_pages/404.html";
								if (open_fd < 0)
								{
									std::cerr << "open error - ." << clients[id].get_route() << std::endl;
									exit(-1);
								}
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
						std::string index = Config.v_server[server_id].v_location[location_id].get_index();
						if (Config.v_server[server_id].v_location[location_id].get_index() != "" &&
							(index.find("php") == std::string::npos && index.find("py") == std::string::npos))
						{
							clients[id].RETURN = 200;
							std::string root = Config.v_server[server_id].v_location[location_id].get_root();
							if (root != "" && *(root.end() - 1) != '/')
								root += '/';
							clients[id].set_route(root + index);

							int open_fd = open(('.' + clients[id].get_route()).c_str(), O_RDONLY);
							clients[id].open_file_name = '.' + clients[id].get_route();
							if (open_fd < 0)
							{
								std::cerr << "open error - ." << clients[id].get_route() << std::endl;
								int open_fd = open("./status_pages/404.html", O_RDONLY);
								clients[id].open_file_name = "./status_pages/404.html";
								if (open_fd < 0)
									std::cerr << "open error - ." << clients[id].get_route() << std::endl;
								clients[open_fd].set_read_fd(id); // event_fd:6 -> open_fd:10  발생된10->6
								clients[open_fd].set_status(need_error_read);

								clients[id].set_status(WAIT);
								change_events(kq.change_list, open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // read event 추가
								break;
							}

							clients[open_fd].set_read_fd(id); // event_fd:6 -> open_fd:10  발생된10->6
							clients[open_fd].set_status(need_to_GET_read);
							change_events(kq.change_list, open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // read event 추가
						}
						else if ((index.find("php") == std::string::npos && index.find("py") == std::string::npos))
						{
							clients[id].RETURN = 200;
							clients[id].set_status(ok);
						}
						else /////////////////// cgi
						{
							// std::out << "im cgi!!\n";
							clients[id].RETURN = 201;
							std::string root = '.' + Config.v_server[server_id].v_location[location_id].get_root();
							if (root != "" && *(root.end() - 1) != '/')
								root += '/';
							std::string index_root = root + Config.v_server[server_id].v_location[location_id].get_index();
							// std::out << "index_root: " << index_root << std::endl;
							// std::out << "cgi-file: " << Config.v_server[server_id].get_cgi_path() << std::endl;
							webserv.run_cgi(Config.v_server[server_id], index_root, clients[id]); // envp have to fix
							close(clients[id].write_fd);
							clients[clients[id].read_fd].set_read_fd(id);
							clients[clients[id].read_fd].set_status(need_to_cgi_read);
							clients[clients[id].read_fd].pid = clients[id].pid;
							// std::cout << "clients[clients[" << id << "].read_fd].get_read_fd() :" << clients[clients[id].read_fd].get_read_fd()
							// 		  << std::endl;
							// std::cout << "read_fd : " << clients[id].read_fd << std::endl;
							change_events(kq.change_list, clients[id].read_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
						}
					}
					else if (clients[id].request.get_method() == "POST") /////////////// *************** post *****************
					{
						int is_dir = webserv.is_dir(Config.v_server[server_id], clients[id].request, clients[id]);
						if (is_dir == 1) // is dir
						{
							std::cout << "ISDIR\n";
							clients[id].RETURN = 201;
							std::string referer = clients[id].request.get_referer();
							if (*(referer.end() - 1) != '/')
								referer += '/';
							std::string route = "." + clients[id].request.get_referer() + "NEW_FILE";
							FILE *file_ptr;
							int i = 0;
							while ((file_ptr = fopen((route + std::to_string(i)).c_str(), "r")) != NULL)
							{
								if (fclose(file_ptr) == EOF)
									error_exit("fclose");
								i++;
							}
							// fclose(file_ptr);
							int open_fd = open((route + std::to_string(i)).c_str(), O_RDWR | O_CREAT | O_APPEND | O_SYNC, S_IWUSR | S_IRUSR);
							clients[id].open_file_name = route + std::to_string(i);
							if (open_fd < 0)
							{
								clients[id].open_file_name = "404";
								std::cerr << "open error - " << route << std::endl;
							}
							std::cout << "POST-my fd::" << id << ", open fd::" << open_fd << std::endl;
							clients[open_fd].set_status(need_to_POST_write);
							clients[open_fd].write_fd = id;
							if (clients[id].request.get_referer().find("php") != std::string::npos ||
								clients[id].request.get_referer().find("py") != std::string::npos)
							{
								// cgi
								clients[id].RETURN = 200;
								std::string index_root = route + std::to_string(i);
								webserv.run_cgi(Config.v_server[server_id], index_root, clients[id]);
								clients[clients[id].read_fd].pid = clients[id].pid;
								clients[clients[id].read_fd].set_read_fd(id);
								// clients[clients[id].write_fd].set_write_fd(id); 생각해보니 필요없음
								clients[clients[id].read_fd].set_status(need_to_cgi_write);
								std::cout << "clients[clients[" << id << "].read_fd].get_read_fd() :" << clients[clients[id].read_fd].get_read_fd()
										  << std::endl;
								std::cout << "read_fd : " << clients[id].read_fd << std::endl;
								change_events(kq.change_list, clients[id].read_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // cgi result 읽기
								change_events(kq.change_list, clients[id].write_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0,
											  NULL); // cgi에 post_body 쓰기
							}
							else
								clients[open_fd].request.post_body = clients[id].request.post_body;

							clients[id].set_status(WAIT);
							change_events(kq.change_list, open_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL); // write event 추가
						}
						else /////////////////////////////////////
						{
							std::cout << "not dir\n";
							std::string route = "." + clients[id].request.get_referer();
							int open_fd;
							if (clients[id].RETURN == 200)
								open_fd = open(route.c_str(), O_RDWR | O_APPEND | O_SYNC, S_IWUSR | S_IRUSR);
							else
								open_fd = open(route.c_str(), O_RDWR | O_CREAT | O_APPEND | O_SYNC, S_IWUSR | S_IRUSR);
							clients[id].open_file_name = route;
							if (open_fd < 0)
							{
								clients[id].open_file_name = "404";
								std::cerr << "open error - " << route << std::endl;
							}
							std::cout << "POST-my fd!!" << id << ", open fd!!" << open_fd << std::endl;
							clients[open_fd].set_status(need_to_POST_write);
							clients[open_fd].write_fd = id;
							clients[open_fd].request.post_body = clients[id].request.post_body;

							clients[id].set_status(WAIT);
							// fcntl(open_fd, F_SETFL, O_NONBLOCK);
							change_events(kq.change_list, open_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL); // write event 추가
						}
					} // end POST
				}
			} // end FILT READ

			if (clients[id].get_server_id() < -1)
				continue;
			if (kq.event_list[i].filter == EVFILT_WRITE &&
				(clients[id].get_status() == need_to_POST_write || clients[clients[id].read_fd].get_status() == need_to_cgi_write)) ////////////////////////////////
			{
				FILE *fp = fdopen(id, "w");
				if (fp == NULL)
				{
					if (fclose(fp) == EOF)
						error_exit("fclose");
					continue;
				}

				write(id, clients[id].request.post_body.c_str(), clients[id].request.post_body.length());
				// std::out << "write-" << id << ":" << clients[id].request.post_body << std::endl;
				clients[clients[id].get_write_fd()].set_status(POST_ok);
				close(id);
				clients.erase(id);
				break;
			}
			else if (kq.event_list[i].filter == EVFILT_WRITE && clients[id].get_status() >= WRITE_LINE)
			{
				// std::out << "accept WRITE Event / ident :" << id << std::endl;
				set_content_type(clients[id], webserv);
				if (clients[id].get_status() >= WRITE_LINE) // && server_it->request.get_host() != "")
				{
					if (clients[id].get_status() == error_read_ok)
					{
						clients[id].response.set_header(404, "", clients[id].content_type);
					}
					else if (clients[id].get_status() == is_file_read_ok)
					{
						clients[id].response.set_header(clients[id].RETURN, "", clients[id].content_type);
						// std::cerr << "response :: " << clients[id].response.get_send_to_response().c_str() << std::endl;
					}
					else if (clients[id].get_status() == cgi_read_ok)
					{
						clients[id].response.set_header(200, clients[id].header, clients[id].content_type);
						// std::cerr << "CGI response :: " << clients[id].response.get_send_to_response().c_str() << std::endl;
					}
					else if (clients[id].get_status() == POST_ok)
					{
						// std::cer << "POST RETURN:" << clients[id].RETURN << std::endl;
						clients[id].response.set_header(clients[id].RETURN, "", clients[id].content_type);
						// std::cer << "POST response :: " << clients[id].response.get_send_to_response().c_str() << std::endl;
					}
					else if (clients[id].request.referer.find("favicon.ico") == std::string::npos && clients[id].request.get_method() == "GET")
					{																			  // clients[id].location_id != -1 &&
						if (Config.v_server[clients[id].get_server_id()].get_autoindex() == "on") // location on?
						{
							// std::cer << "auto indexing~!\n";
							DIR *dir;
							int is_root = 0;
							struct dirent *ent;
							if (clients[id].location_id < 0 && clients[id].is_file != 1) // /abc
								dir = opendir(('.' + clients[id].request.referer).c_str());
							else if (clients[id].request.get_referer() == "/")
							{
								is_root = 1;
								dir = opendir(('.' + clients[id].request.referer).c_str());
							}
							else
							{
								// int open_fd = open("./status_pages/404.html", O_RDONLY); // not autoindexing
								// if (open_fd < 0)
								// 	std::cerr << "open error - " << clients[id].get_route() << std::endl;
								// std::cout << "404-my fd::" << id << ", open fd::" << open_fd << std::endl;
								// clients[open_fd].set_read_fd(id); // event_fd:6 -> open_fd:10  발생된10->6
								// clients[open_fd].set_status(need_error_read);

								// clients[id].set_status(WAIT);
								// change_events(kq.change_list, open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // read event 추가
								break;
							}
							if (dir != NULL)
							{
								/* print all the files and directories within directory */
								clients[id].response.response_str += "<!DOCTYPE html>\n";
								clients[id].response.response_str += "<html>\n";
								clients[id].response.response_str += "<head>\n</head>\n";
								clients[id].response.response_str += "<body>\n";
								clients[id].response.response_str += "<h1>Index of ." + clients[id].request.referer + "</h1>\n";
								clients[id].response.response_str += "</a><br>\n";
								while ((ent = readdir(dir)) != NULL)
									clients[id].response.set_autoindex(clients[id].request.referer, ent->d_name, is_root);
								closedir(dir);
							}
							else
							{
								/* could not open directory */
								perror("");
								return EXIT_FAILURE;
							}
						}
						clients[id].response.set_header(200, "", clients[id].content_type);
					}
					else
					{
						clients[id].response.set_header(200, "", clients[id].content_type); // ok
					}

					if (clients[id].get_status() == cgi_read_ok) {
						write(id, clients[id].response.get_send_to_response().c_str(), clients[id].response.get_send_to_response().length());
						close(id);
						clients.erase(id);
						break;
					}
					FILE *fp = fdopen(id, "wb");
					if (fp == NULL)
					{
						// std::out << "fdopen error" << std::endl;
						continue;
					}
					// std::cerr << "response :: " << clients[id].response.get_send_to_response().c_str() << std::endl;
					int count = 0;
					fwrite(clients[id].response.get_send_to_response().c_str(), sizeof(char),
						clients[id].response.get_send_to_response().size(), fp);

					if (fclose(fp) == EOF)
						error_exit("fclose");
					close(id);
					clients.erase(id);
				}
			}
		}
	}
	exit(0);
	return 0;
}
