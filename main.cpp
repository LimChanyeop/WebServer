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
		exit(EXIT_FAILURE);
	}
	Config.config_parsing(vec_attr);
	Config.config_check();
	std::string &mime = webserv.mime_read(default_mime);
	webserv.mime_parsing(mime);

	webserv.ready_webserv(Config);
	std::map<int, Client> clients;
	webserv.get_kq().set_kq_fd(kqueue());
	for (std::vector<Server>::iterator it = const_cast<std::vector<Server> &>(Config.get_v_server()).begin(); it != Config.get_v_server().end(); it++)
	{
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
			if (webserv.get_kq().get_event_list()[i].flags & EV_ERROR)
			{
				if (webserv.is_client(Config, id))
				{
					if (clients[id].get_pid() == -1)
					{
						if (clients[id].get_request().get_requests().size() > 0)
							fclose(const_cast<FILE *>(clients[id].get_read_fp())); // fclose by jwoo
						if (clients[id].get_response().get_response_str().length() > 0)
							fclose(const_cast<FILE *>(clients[id].get_write_fp()));
					}
				}
				if (clients[id].get_read_fd() > 0)
					clients.erase(clients[id].get_read_fd());
				else if (clients[id].get_write_fd() > 0)
					clients.erase(clients[id].get_write_fd());
				clients.erase(id);
				continue;
			}
			else if (webserv.get_kq().get_event_list()[i].filter == EVFILT_READ)
			{
				if (clients[id].get_status() == need_to_GET_read || clients[id].get_status() == need_to_is_file_read ||
					clients[id].get_status() == need_error_read)
				{
					FILE *file_ptr;
					file_ptr = fdopen(id, "r");
					int read_fd = clients[id].get_read_fd(); //
					int valfread = 0;
					std::string fread_str;
					char buff[BUFSIZE];
					
					memset(buff, 0, BUFSIZE);
					while ((valfread = fread(buff, sizeof(char), BUFSIZE - 1, file_ptr)) >= 0)
					{
						if (valfread == 0)
							break;
						buff[valfread] = 0;
						fread_str.append(buff, valfread);
					}
					if (valfread < 0)
					{
						webserv.set_error_page(clients, read_fd, 500, Config);

						close(id);
						clients.erase(id);
						break;
					}
					
					fclose(file_ptr);
					clients[read_fd].get_response().set_response_str(fread_str);
					if (clients[id].get_status() == need_error_read)
						clients[read_fd].set_status(error_read_ok);
					else if (clients[id].get_status() == need_to_is_file_read)
						clients[read_fd].set_status(is_file_read_ok);
					else
						clients[read_fd].set_status(GET_read_ok);
					close(id);
					clients.erase(id);
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
					int valread = 0;
					std::string read_str;
					char buff[BUFSIZE];
					int read_fd = clients[id].get_read_fd(); //
					memset(buff, 0, BUFSIZE);
					while ((valread = read(id, buff, BUFSIZE - 1)) >= 0)
					{ // read
						if (valread == 0)
							break;
						buff[valread] = 0;
						read_str.append(buff, valread);
					}
					if (valread < 0)
					{
						webserv.set_error_page(clients, read_fd, 500, Config);

						close(id);
						clients.erase(id);
						break;
					}

					clients[read_fd].header_parsing(read_str);
					clients[read_fd].get_response().set_response_str(read_str);

					clients[read_fd].set_status(cgi_read_ok);
					close(id);
					clients.erase(id);
				}
				else if (webserv.find_server(Config, clients[id], id)) // 이벤트 주체가 server
				{
					webserv.accept_add_events(id, const_cast<std::vector<Server> &>(Config.get_v_server())[clients[id].get_server_id()], webserv.get_kq(), clients);
				}
				else if (clients.find(id) != clients.end() && clients[id].get_status() != WAIT) // 이벤트 주체가 client
				{
					int k = 1;
					clients[id].set_status(chunked_WAIT);
					FILE *file_ptr = fdopen(id, "r");
					if (file_ptr == NULL)
					{
						return -1;
					}
					clients[id].set_read_fp(file_ptr);

					while (k == 1)
					{
						k = clients[id].request_parsing(file_ptr);
					} // requests_ok
					if (k == -1)
					{
						webserv.set_error_page(clients, id, 500, Config);
						break;
					}

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
						if (location_id == 404)																				   // is not found
						{
							webserv.set_error_page(clients, id, 404, Config);
							break;
						}
						else if (location_id == -1) // dir
						{
							if (clients[id].get_request().get_method() == "GET")
							{
								clients[id].set_RETURN(200);
								clients[id].set_status(ok);
							}
							else // DELETE
							{
								clients[id].set_RETURN(202); // Accepted
								clients[id].set_status(DELETE_ok);
								change_events(webserv.get_kq().get_change_list(), id, EVFILT_READ, EV_DELETE | EV_ENABLE, 0, 0, NULL);
								break;
							}
							if (clients[id].get_request().get_referer() == "/" &&
								Config.get_v_server()[server_id].get_index() != "")
							{
								webserv.read_index(clients, id, Config);
							}
							else if (Config.get_v_server()[server_id].get_autoindex() != "on")
							{
								int open_fd = open("./static_files/Default.html", O_RDONLY);
								if (open_fd < 0)
								{
									webserv.set_error_page(clients, id, 404, Config);
									break ;
								}
								clients[open_fd].set_read_fd(id); // event_fd:6 -> open_fd:10  발생된10->6
								clients[open_fd].set_status(need_to_is_file_read);

								clients[id].set_status(WAIT);
								change_events(webserv.get_kq().get_change_list(), id, EVFILT_READ, EV_DELETE | EV_ENABLE, 0, 0, NULL);
								fcntl(open_fd, F_SETFL, O_NONBLOCK);
								change_events(webserv.get_kq().get_change_list(), open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // read event 추가
							}

							break;
						}
						else if (location_id == -2) // is file
						{
							if (clients[id].get_request().get_method() == "DELETE")
							{
								remove(('.' + clients[id].get_request().get_referer()).c_str());
								clients[id].set_status(ok);
							}
							else
								webserv.read_index(clients, id, Config);
							break;
						}
						clients[id].set_location_id(location_id);

						int stat = Config.get_v_server()[server_id].get_v_location()[location_id].get_redi_status();
						if (stat > 0) // rediraction!!
						{
							clients[id].set_RETURN(stat);
							clients[id].set_redi_root(Config.get_v_server()[server_id].get_v_location()[location_id].get_redi_root());
							clients[id].set_status(redi_write);
							break;
						}
						std::string index = Config.get_v_server()[server_id].get_v_location()[location_id].get_index();
						if (Config.get_v_server()[server_id].get_v_location()[location_id].get_index() != "" &&
							(index.find("php") == std::string::npos && index.find("py") == std::string::npos))
						{
							clients[id].set_RETURN(200);
							std::string root = Config.get_v_server()[server_id].get_v_location()[location_id].get_root();
							if (root == "/" || root == "" || *(root.end() - 1) == '/')
								root.pop_back();
							if (*index.begin() == '/')
								clients[id].set_route(root + index);
							else
								clients[id].set_route(root + '/' + index);

							int open_fd = open(('.' + clients[id].get_route()).c_str(), O_RDONLY);
							clients[id].set_open_file_name('.' + clients[id].get_route());
							if (open_fd == -1)
							{
								webserv.set_error_page(clients, id, 404, Config);
								break;
							}

							clients[open_fd].set_read_fd(id); // event_fd:6 -> open_fd:10  발생된10->6
							clients[open_fd].set_status(need_to_GET_read);
							clients[id].set_status(WAIT);
							change_events(webserv.get_kq().get_change_list(), id, EVFILT_READ, EV_DELETE | EV_ENABLE, 0, 0, NULL);
							fcntl(open_fd, F_SETFL, O_NONBLOCK);
							change_events(webserv.get_kq().get_change_list(), open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // read event 추가
							break;
						}
						else if ((index.find("php") == std::string::npos && index.find("py") == std::string::npos))
						{
							clients[id].set_RETURN(200);
							clients[id].set_status(ok);
						}
						else // cgi
						{
							clients[id].set_RETURN(201);
							std::string root = '.' + Config.get_v_server()[server_id].get_v_location()[location_id].get_root();
							if (root != "" && *(root.end() - 1) != '/')
								root += '/';
							std::string index_root = root + Config.get_v_server()[server_id].get_v_location()[location_id].get_index();
							webserv.run_cgi(Config.get_v_server()[server_id], index_root, clients[id], location_id); // envp have to fix
							close(clients[id].get_write_fd());
							clients[clients[id].get_read_fd()].set_read_fd(id);
							clients[clients[id].get_read_fd()].set_status(need_to_cgi_read);
							clients[clients[id].get_read_fd()].set_pid(clients[id].get_pid());
							clients[id].set_status(WAIT);
							change_events(webserv.get_kq().get_change_list(), id, EVFILT_READ, EV_DELETE | EV_ENABLE, 0, 0, NULL);
							fcntl(clients[id].get_read_fd(), F_SETFL, O_NONBLOCK);
							change_events(webserv.get_kq().get_change_list(), clients[id].get_read_fd(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
						}
					}
					else if (clients[id].get_request().get_method() == "POST") //************ POST *****************
					{
						int is_dir = webserv.is_dir(Config.get_v_server()[server_id], clients[id].get_request(), clients[id]);
						if (is_dir == 1) // is dir
						{
							std::string referer = clients[id].get_request().get_referer();
							if (*(referer.end() - 1) != '/')
								referer += '/';
							std::string route = "." + clients[id].get_request().get_referer() + "NEW_FILE";
							FILE *file_ptr;
							int i = 0;
							while ((file_ptr = fopen((route + std::to_string(i)).c_str(), "r")) != NULL)
							{
								fclose(file_ptr);
								i++;
							}
							clients[id].set_open_file_name(route + std::to_string(i));
							int open_fd = open((route + std::to_string(i)).c_str(), O_RDWR | O_CREAT | O_APPEND | O_SYNC, 0777);
							if (open_fd < 0)
							{
								webserv.set_error_page(clients, id, 404, Config);
								break;
							}
							clients[open_fd].set_status(need_to_POST_write);
							clients[open_fd].set_write_fd(id);
							clients[open_fd].get_request().set_post_body(clients[id].get_request().get_post_body());

							clients[id].set_status(WAIT);
							change_events(webserv.get_kq().get_change_list(), id, EVFILT_READ, EV_DELETE | EV_ENABLE, 0, 0, NULL);
							fcntl(open_fd, F_SETFL, O_NONBLOCK);
							change_events(webserv.get_kq().get_change_list(), open_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL); // write event 추가
						}
						else
						{
							std::string index_root = "." + clients[id].get_request().get_referer();
							if (clients[id].get_request().get_referer().find("php") != std::string::npos ||
								clients[id].get_request().get_referer().find("py") != std::string::npos)
							{
								// POST CGI
								clients[id].set_RETURN(200);
								webserv.run_cgi(Config.get_v_server()[server_id], index_root, clients[id], -1);
								clients[clients[id].get_read_fd()].set_pid(clients[id].get_pid());
								clients[clients[id].get_write_fd()].set_pid(clients[id].get_pid());
								clients[clients[id].get_read_fd()].set_read_fd(id);
								clients[clients[id].get_write_fd()].set_write_fd(id);

								clients[clients[id].get_read_fd()].get_request().set_post_body(clients[id].get_request().get_post_body());
								clients[clients[id].get_write_fd()].get_request().set_post_body(clients[id].get_request().get_post_body()); // body
								clients[clients[id].get_write_fd()].get_request().set_header(clients[id].get_request().get_header()); // header

								clients[clients[id].get_write_fd()].set_status(need_to_cgi_write);

								clients[id].set_status(WAIT);
								change_events(webserv.get_kq().get_change_list(), id, EVFILT_READ, EV_DELETE | EV_ENABLE, 0, 0, NULL);
								fcntl(clients[id].get_write_fd(), F_SETFL, O_NONBLOCK);
								fcntl(clients[id].get_read_fd(), F_SETFL, O_NONBLOCK);
								change_events(webserv.get_kq().get_change_list(), clients[id].get_write_fd(), EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL); // cgi에 post_body 쓰기
								change_events(webserv.get_kq().get_change_list(), clients[id].get_read_fd(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
								break;
							}
							int open_fd;
							if (clients[id].get_RETURN() == 200)
								open_fd = open(index_root.c_str(), O_RDWR | O_APPEND | O_SYNC, 0777);
							else
								open_fd = open(index_root.c_str(), O_RDWR | O_CREAT | O_APPEND | O_SYNC, 0777);
							clients[id].set_open_file_name(index_root);
							if (open_fd < 0)
							{
								webserv.set_error_page(clients, id, 404, Config);
								break;
							}
							clients[open_fd].set_status(need_to_POST_write);
							clients[open_fd].set_write_fd(id);
							clients[open_fd].get_request().set_post_body(clients[id].get_request().get_post_body());

							clients[id].set_status(WAIT);
							change_events(webserv.get_kq().get_change_list(), id, EVFILT_READ, EV_DELETE | EV_ENABLE, 0, 0, NULL);
							fcntl(open_fd, F_SETFL, O_NONBLOCK);
							change_events(webserv.get_kq().get_change_list(), open_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL); // write event 추가
						}
					} // end POST
				}
			} // end FILT READ
			else if (webserv.get_kq().get_event_list()[i].filter == EVFILT_WRITE)
			{
				if (clients[id].get_status() == need_to_POST_write) // file에다가 write
				{
					FILE *fp = fdopen(id, "wb");

					size_t wr_val = fwrite(clients[id].get_request().get_post_body().c_str(), sizeof(char), clients[id].get_request().get_post_body().length(), fp);
					if (wr_val < 0)
					{
						fclose(fp);
						close(id);
						clients.erase(id);
						webserv.set_error_page(clients, clients[id].get_write_fd(), 500, Config);
						break;
					}
					else
					{
						clients[clients[id].get_write_fd()].set_status(POST_ok);
					}

					fclose(fp);
					close(id);
					clients.erase(id);
					break;
				}
				else if (clients[id].get_status() == need_to_cgi_write) // CGI에다가 write
				{
					FILE *fp = fdopen(id, "wb");

					size_t wr_val = fwrite(clients[id].get_request().get_post_body().c_str(), sizeof(char), clients[id].get_request().get_post_body().length(), fp);
					if (wr_val < 0)
					{
						int read_fd = clients[clients[id].get_write_fd()].get_read_fd();
						change_events(webserv.get_kq().get_change_list(), read_fd, EVFILT_READ, EV_DELETE | EV_ENABLE, 0, 0, NULL); // cgi result 읽기

						fclose(fp);
						close(id);
						clients.erase(id);
						webserv.set_error_page(clients, clients[id].get_write_fd(), 500, Config);
						break;
					}
					else // wr_val >= 0
					{
						int read_fd = clients[clients[id].get_write_fd()].get_read_fd();
						clients[read_fd].set_status(need_to_cgi_read);

						fclose(fp);
						close(id);
						clients.erase(id);
						break;
					}
				}
				else if (clients[id].get_status() >= WRITE_LINE)
				{
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
							if (clients[id].get_request().get_referer() == "/")
								webserv.set_indexing(clients[id]);
							clients[id].get_response().set_header(clients[id].get_RETURN(), "", clients[id].get_content_type());
						}
						else if (clients[id].get_status() == cgi_read_ok)
						{
							clients[id].get_response().set_header(200, clients[id].get_request().get_header(), clients[id].get_content_type());
						}
						else if (clients[id].get_status() == POST_ok)
						{
							clients[id].get_response().set_header(clients[id].get_RETURN(), "", clients[id].get_content_type());
						}
						else if (clients[id].get_status() == DELETE_ok)
						{
							clients[id].get_response().set_header(clients[id].get_RETURN(), "", "DELETE");
						}
						else if (clients[id].get_location_id() > 0)
						{
							clients[id].get_response().set_header(clients[id].get_RETURN(), "", clients[id].get_content_type());
						}
						else if (clients[id].get_request().get_referer().find("favicon.ico") == std::string::npos && clients[id].get_request().get_method() == "GET")
						{
							if (Config.get_v_server()[clients[id].get_server_id()].get_autoindex() == "on") // location on?
							{
								DIR *dir;
								int is_root = 0;
								struct dirent *ent;
								if (clients[id].get_location_id() < 0 && clients[id].get_is_file() != 1) // /abc
									dir = opendir(('.' + clients[id].get_request().get_referer()).c_str());
								else if (clients[id].get_request().get_referer() == "/")
								{
									is_root = 1;
									dir = opendir(('.' + clients[id].get_request().get_referer()).c_str());
								}
								else
								{
									webserv.set_error_page(clients, id, 404, Config);
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
									close(id);
									clients.erase(id);
									return EXIT_FAILURE;
								}
							}
							clients[id].get_response().set_header(clients[id].get_RETURN(), "", clients[id].get_content_type());
						}
						else
						{
							clients[id].get_response().set_header(clients[id].get_RETURN(), "", clients[id].get_content_type()); // ok
						}
						FILE *fp = fdopen(id, "wb");
						size_t wr_val = 0;
						clients[id].set_write_fp(fp);
						if (clients[id].get_status() == cgi_read_ok)
						{
							while ((wr_val = write(id, clients[id].get_response().get_send_to_response().c_str(), clients[id].get_response().get_send_to_response().size()))
								< clients[id].get_response().get_send_to_response().size())
								;
							if (wr_val < 0)
							{
								webserv.set_error_page(clients, id, 500, Config);
								clients[id].get_response().set_response_str("");
								break;
							}
							else
							{
								close(id);
								fclose(fp);
								clients.erase(id);
								break;
							}
						}
						while ((wr_val += fwrite(clients[id].get_response().get_send_to_response().c_str(), sizeof(char), clients[id].get_response().get_send_to_response().size(), fp))
							< clients[id].get_response().get_send_to_response().size())
							;
						if (wr_val < 0)
						{
							webserv.set_error_page(clients, id, 500, Config);
							clients[id].get_response().set_response_str("");
							fclose(fp);
							close(id);
							clients.erase(id);
							break;
						}
						else
						{
							fclose(fp);
							close(id);
							clients.erase(id);
						}
					}
				}
			}
		}
	}
	exit(0);
	return 0;
}
