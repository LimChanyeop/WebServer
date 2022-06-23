// #include <sys/types.h>
// #include <sys/stat.h>

#include "../includes/Webserv.hpp"
#include "../includes/Fd.hpp"
#include "../includes/ParseUtils.hpp"
#include "../includes/Request.hpp"

#include <dirent.h>

Webserv::Webserv(/* args */) {}

Webserv::~Webserv() {}

void change_events(std::vector<struct kevent> &change_list, uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data,
				   void *udata) // 이벤트를 생성하고 이벤트 목록에 추가하는 함수
{
	struct kevent temp_event;

	EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
	change_list.push_back(temp_event);
}

int Webserv::is_client(Config config, int id)
{
	std::vector<Server>::iterator it = const_cast<std::vector<Server> &>(config.get_v_server()).begin();
	for (; it != config.get_v_server().end(); it++)
	{
		if (it->get_socket_fd() == id)
			return 0;
	}
	return 1;
}

void Webserv::set_content_type(Client &client, const Webserv &webserv)
{
	std::__1::map<std::__1::string, std::__1::string>::const_iterator it;

	if (client.get_open_file_name() == "")
	{
		client.set_content_type("text/html");
		return;
	}
	std::string open_file_name = client.get_open_file_name();
	unsigned long find;
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
		client.set_content_type("text/plain");
		return;
	}
	std::map<std::string, std::string>::const_iterator m_it = const_cast<Webserv &>(webserv).get_mimes().begin();
	while (m_it != const_cast<Webserv &>(webserv).get_mimes().end())
	{
		if (m_it->first.find(open_file_name) != std::string::npos)
		{
			client.set_content_type(m_it->second);
			return;
		}
		m_it++;
	}
	if (m_it == const_cast<Webserv &>(webserv).get_mimes().end())
	{
		client.set_content_type("text/plain");
	}
	return;

	if (client.get_request().get_referer().find("html")) ////////
		client.set_content_type("text/html");
	else if (client.get_request().get_referer().find("txt"))
		client.set_content_type("text/plain");

	else if (client.get_request().get_referer().find("gif"))
		client.set_content_type("image/gif");
	else if (client.get_request().get_referer().find("png"))
		client.set_content_type("image/png");
}

int Webserv::find_server(Config Config, Client &client, int id)
{
	std::vector<Server>::iterator it = const_cast<std::vector<Server> &>(Config.get_v_server()).begin();
	for (; it != Config.get_v_server().end(); it++)
	{
		if (it->get_socket_fd() == id)
		{
			client.set_server_id(it - Config.get_v_server().begin());
			client.set_server_sock(id);
			return 1; // 드디어 어떤 서버인지 찾음
		}
	}
	return 0;
}

std::string &Webserv::mime_read(std::string &default_mime)
{
	int open_fd;
	open_fd = open(default_mime.c_str(), O_RDONLY);
	if (open_fd < 0)
	{
		std::cerr << "mime open error!!\n";
		exit(-1);
	}
	char buff[BUFSIZE];
	std::string read_str;
	int valread = 0;
	memset(buff, 0, BUFSIZE);
	while ((valread = read(open_fd, buff, BUFSIZE - 1)) >= 0)
	{
		if (valread == 0)
			break;
		buff[valread] = 0;
		read_str.append(buff, valread);
	}
	if (valread < 0)
	{
		std::cerr << "mime read error!!\n";
		exit(-1);
	}
	default_mime = read_str;
	return default_mime;
}

void Webserv::mime_parsing(std::string &default_mime)
{
	std::vector<std::string> split_mime;
	std::string mime_str;
	std::string::iterator it = default_mime.begin();
	while (it != default_mime.end())
	{
		if (*it == '\n')
		{
			split_mime.push_back(mime_str);
			mime_str = "";
			it++;
			continue;
		}
		mime_str += *it;
		it++;
	}
	std::vector<std::string>::iterator v_it;
	std::string mime_str2;
	for (v_it = split_mime.begin(); v_it != split_mime.end(); v_it++)
	{
		it = (*v_it).begin();
		while (it != (*v_it).end() && *it != ' ') // str1
		{
			mime_str += *it;
			it++;
		}
		while (it != (*v_it).end() && *it == ' ') // ' '
			it++;
		while (it != (*v_it).end()) // str2
		{
			mime_str2 += *it;
			it++;
		}
		this->mimes[mime_str2] = mime_str;
		mime_str.clear();
		mime_str2.clear();
	}
}

void Webserv::ready_webserv(Config &Config)
{
	std::vector<Server>::iterator it = const_cast<std::vector<Server> &>(Config.get_v_server()).begin(); // const cast
	for (; it != Config.get_v_server().end(); it++)
	{
		(*it).set_socket_fd(socket(AF_INET, SOCK_STREAM, 0)); // TCP: SOCK_STREAM UDP: SOCK_DGRAM
		if (it->get_socket_fd() <= 0)
		{
			std::cerr << "socket error" << std::endl;
			exit(0);
		}

		sockaddr_in &address = it->get_address();
		memset((char *)&address, 0, sizeof(address));
		address.sin_family = AF_INET; // tcp
		address.sin_port = htons(atoi(it->get_listen().c_str()));
		address.sin_addr.s_addr = htonl(INADDR_ANY); // open inbound restriction
		memset(address.sin_zero, 0, sizeof(address.sin_zero));

		int optvalue = 1;
		setsockopt(it->get_socket_fd(), SOL_SOCKET, SO_REUSEADDR, &optvalue,
				   sizeof(optvalue)); // to solve bind error
		setsockopt(it->get_socket_fd(), SOL_SOCKET, SO_REUSEPORT, &optvalue,
				   sizeof(optvalue)); // to solve bind error

		fcntl(it->get_socket_fd(), F_SETFL, O_NONBLOCK);
		int ret;
		if ((ret = bind(it->get_socket_fd(), (sockaddr *)&address, (socklen_t)sizeof(address))) == -1)
		{
			std::cerr << "bind error : return value = " << ret << std::endl;
			exit(0);
		}
		if ((listen(it->get_socket_fd(), NOE)) < 0)
		{
			std::cerr << "listen error" << std::endl;
			exit(0);
		}
	}
}

std::vector<Server>::iterator Webserv::find_server_it(Config &Config, Client &client)
{
	std::vector<Server>::iterator it = const_cast<std::vector<Server> &>(Config.get_v_server()).begin(); // const cast
	for (; it != Config.get_v_server().end(); it++)
	{
		if (it->get_socket_fd() == client.get_server_sock())
		{
			return it; // 드디어 어떤 서버인지 찾음
		}
	}
	if (it == Config.get_v_server().end())
	{
		std::cerr << "Can not found Server\n";
		exit(-1);
	}
	return it;
}

int Webserv::find_server_id(const int &event_ident, const Config &config, const Request &rq, std::map<int, Client> &clients)
{
	std::string port = ""; // 왜 포트를 못찾았을까? -> 포트를 파싱 안했었넹~ok -> 근데도 못찾네~
	unsigned long server_id;
	if (clients.find(event_ident) != clients.end())
	{
		server_id = 0;
		while (server_id < config.get_v_server().size())
		{
			if (atoi(config.get_v_server()[server_id].get_listen().c_str()) == atoi(rq.get_host().c_str())) // 못찾는게,, 이게 다르데;;
			{
				port = rq.get_host(); // ?
				return server_id;
			}
			server_id++;
		}
	}
	return -1;
}

int Webserv::check_except(std::map<int, Client> &clients, Config &config, int &ident, int &server_id)
{
	std::string limit_except = config.get_v_server()[server_id].get_limit_except();
	std::string method = clients[ident].get_request().get_method();
	if (method != "GET" && method != "POST" && method != "DELETE")
	{
		this->set_error_page(clients, ident, 405);
		return -1;
	}
	if (limit_except == "")
		return 1;
	if (limit_except.find(method) == std::string::npos) // no match
	{
		std::cerr << "Not except\n";
		this->set_error_page(clients, ident, 405);
		return -1;
	}
	return 1;
}

int Webserv::check_size(std::map<int, Client> &clients, Config &config, int &ident, int &server_id)
{
	if (clients[ident].get_request().get_method() == "POST")
	{
		if (clients[ident].get_request().get_post_body_size() == 0)
		{
			this->set_error_page(clients, ident, 400);
			return -1;
		}
	}
	if (clients[ident].get_request().get_header_size() > config.get_v_server()[server_id].get_request_limit_header_size())
	{
		this->set_error_page(clients, ident, 413);
		return -1;
	}
	if (clients[ident].get_request().get_post_body_size() > config.get_v_server()[server_id].get_client_limit_body_size())
	{
		this->set_error_page(clients, ident, 413);
		return -1;
	}
	return 1;
}

int Webserv::is_dir(const Server &server, const Request &rq, Client &client) // POST
{
	std::string referer = rq.get_referer();
	if (*referer.begin() == '/')
		referer.erase(referer.begin(), referer.begin() + 1);
	client.set_route(server.get_root() + referer);
	std::string route = "." + client.get_route();
	DIR *dir_ptr = NULL;
	if ((dir_ptr = opendir(route.c_str())) != NULL)
	{
		client.set_RETURN(201);
		closedir(dir_ptr);
		return 1;
	}
	FILE *file_ptr;
	if ((file_ptr = fopen(route.c_str(), "r")))
	{								  // wb
		client.set_is_file(1);
		client.set_RETURN(200);
		fclose(file_ptr);
		return 0;
	}
	else
	{
		client.set_RETURN(201); // created
		return 0;
	}
	return -1;
}

int Webserv::find_location_id(const int &server_id, const Config &config, const Request &rq, Client &client)
{
	unsigned long location_id;
	if (rq.get_method() == "GET")
	{
		for (location_id = 0; location_id < config.get_v_server()[server_id].get_v_location().size(); location_id++)
		{
			if (config.get_v_server()[server_id].get_v_location()[location_id].get_location() == rq.get_referer())
			{
				return location_id;
			}
		}
	}
	std::string referer = rq.get_referer();
	if (*referer.begin() == '/')
		referer.erase(referer.begin(), referer.begin() + 1);
	client.set_route(config.get_v_server()[server_id].get_root() + referer);
	std::string route = "." + config.get_v_server()[server_id].get_root() + referer;

	DIR *dir_ptr = NULL;	// is dir?
	if ((dir_ptr = opendir(route.c_str())) != NULL)
	{
		client.set_RETURN(200);
		closedir(dir_ptr);
		return -1;
	}
	FILE *file; // file exist?
	if ((file = fopen(route.c_str(), "r")) != NULL)
	{ // exist
		fclose(file);
		client.set_is_file(1);
		client.set_RETURN(200);
		return -2;
	}
	return 404; // 404에러
}

void Webserv::accept_add_events(const int &event_ident, Server &server, Kqueue &kq, std::map<int, Client> &clients)
{
	int acc_fd;
	if ((acc_fd = accept(server.get_socket_fd(), (sockaddr *)&(server.get_address()),
						 (socklen_t *)&(server.get_address_len()))) == -1) //
	{
		std::cerr << "accept error " << acc_fd << std::endl;
		exit(0);
	}
	fcntl(acc_fd, F_SETFL, O_NONBLOCK);
	struct timespec timeout;
	timeout.tv_sec = 10; // 초
	timeout.tv_nsec = 0;
	setsockopt(acc_fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval));
	setsockopt(acc_fd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(struct timeval));

	fcntl(acc_fd, F_SETFL, O_NONBLOCK);
	change_events(kq.get_change_list(), acc_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
	change_events(kq.get_change_list(), acc_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
	clients[acc_fd].set_server_sock(event_ident);
	clients[acc_fd].set_status(server_READ_ok);
}
char **make_env(Client &client, const Server &server)
{
	std::map<std::string, std::string> cgi_map;
	cgi_map["SERVER_PROTOCOL"] = "HTTP/1.1";
	cgi_map["GATEWAY_INTERFACE"] = "CGI/1.1";
	cgi_map["SERVER_SOFTWARE"] = "nginx server";
	cgi_map["REQUEST_METHOD"] = client.get_request().get_method();
	cgi_map["REQUEST_SCHEME"] = client.get_request().get_method();
	cgi_map["SERVER_PORT"] = client.get_request().get_host();
	cgi_map["SERVER_NAME"] = "localhost";
	cgi_map["DOCUMENT_ROOT"] = server.get_cgi_path();			//"./cgiBinary/php-cgi"; //
	cgi_map["DOCUMENT_URI"] = client.get_route();				//"/View/file.php"; // 리퀘스트에 명시된 전체 주소가 들어가야 함 //
	cgi_map["REQUEST_URI"] = client.get_route();				// "/View/file.php";	// 리퀘스트에 명시된 전체 주소가 들어가야 함 //
	cgi_map["SCRIPT_NAME"] = client.get_route();				// "/View/file.php";	// 실행파일 전체 주소가 들어가야함 //
	cgi_map["SCRIPT_FILENAME"] = '.' + client.get_route();		//"./View/file.php";
	cgi_map["QUERY_STRING"] = client.get_request().get_query();	//
	cgi_map["REMOTE_ADDR"] = "127.0.0.1"; 						//client.get_ip();
	cgi_map["REDIRECT_STATUS"] = "200";
	std::cerr << "file name:" << client.get_request().get_post_filename() << std::endl;
	if (client.get_request().get_method() == "POST")
		cgi_map["CONTENT_LENGTH"] = client.get_request().get_contentLength(); // GET은 노노
	if (client.get_request().get_post_filename().find(".png") != std::string::npos ||
		client.get_request().get_post_filename().find(".jpg") != std::string::npos ||
		client.get_request().get_post_filename().find(".jpeg") != std::string::npos ||
		client.get_request().get_post_filename().find(".gif") != std::string::npos)
		cgi_map["CONTENT_TYPE"] = "multipart/form-data; boundary=" + client.get_request().get_boundary();
	else if (client.get_request().get_contentType() != "")
		cgi_map["CONTENT_TYPE"] = client.get_request().get_contentType();
	else
		cgi_map["CONTENT_TYPE"] = "text/html";

	char **cgi_env;
	cgi_env = new char *[cgi_map.size() + 1];

	size_t i = 0;
	for (std::map<std::string, std::string>::iterator it = cgi_map.begin(); it != cgi_map.end(); it++)
	{
		std::string temp = (it->first + "=" + it->second);
		while (*(temp.end() - 1) == '\n' || *(temp.end() - 1) == '\r')
			temp.pop_back();
		cgi_env[i] = strdup(temp.c_str());
		i++;
	}
	cgi_env[i] = NULL;
	return cgi_env;
}

void Webserv::run_cgi(const Server &server, const std::string &index_root, Client &client)
{
	int read_fd[2];
	int write_fd[2];
	char **cgi_env;

	if (pipe(read_fd) == -1)
	{
		std::cerr << "pipe error\n";
		exit(-1);
	}
	pipe(write_fd);

	cgi_env = make_env(client, server);
	int pid = fork();
	if (pid == -1)
	{
		std::cerr << "fork error\n";
		exit(-1);
	}
	else if (pid == 0) // child
	{
		// ------------------
		char *ar[3];
		ar[0] = const_cast<char *>((server.get_cgi_path()).c_str()); // ("./cgiBinary/php-cgi"); //"./cgiBinary/php-cgi"); // /bin/cat
		ar[1] = const_cast<char *>(index_root.c_str());				 // php file (./file)
		ar[2] = 0;

		dup2(write_fd[0], STDIN_FILENO);
		dup2(read_fd[1], STDOUT_FILENO);
		close(write_fd[1]);
		close(read_fd[0]);
		int ret = execve(ar[0], ar, cgi_env); // "/bin/cat"
		if (ret == -1)
		{
			std::cerr << "***************\nexecve not run!\n***************\n"
					  << std::endl;
			if (cgi_env != NULL)
			{
				for (int i = 0; cgi_env[i] != NULL; i++)
				{
					delete cgi_env[i];
				}
			}
			delete cgi_env;
			exit(-1);
		}
		if (cgi_env != NULL)
		{
			for (int i = 0; cgi_env[i] != NULL; i++)
			{
				delete cgi_env[i];
			}
		}
		delete cgi_env;
		exit(0);
	}
	if (cgi_env != NULL)
	{
		for (int i = 0; cgi_env[i] != NULL; i++)
		{
			delete cgi_env[i];
		}
	}
	delete cgi_env;
	close(write_fd[0]);
	close(read_fd[1]);

	client.set_pid(pid);
	client.set_read_fd(read_fd[0]);
	client.set_write_fd(write_fd[1]);
}

void Webserv::set_error_page(std::map<int, Client> &clients, const int &id, const int &status)
{
	clients[id].set_RETURN(status);
	std::string route = "./status_pages/" + std::to_string(status) + ".html";
	if (status == 0) // default
		route = "./status_pages/Default_error.html";
	int open_fd = open(route.c_str(), O_RDONLY);
	if (open_fd < 0)
	{
		std::cerr << "open error - " << route << std::endl;
		open_fd = open("./status_pages/Default_error.html", O_RDONLY);
		if (open_fd < 0)
		{
			std::cerr << "No File To Show\n";
			close(id);
			clients.erase(id);
		}
	}
	clients[id].set_open_file_name(route);
	clients[open_fd].set_read_fd(id); // event_fd:6 -> open_fd:10  발생된10->6
	clients[open_fd].set_status(need_error_read);

	clients[id].set_status(WAIT);
	change_events(kq.get_change_list(), id, EVFILT_READ, EV_DELETE | EV_ENABLE, 0, 0, NULL); // read event 추가
	fcntl(open_fd, F_SETFL, O_NONBLOCK);
	change_events(kq.get_change_list(), open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // read event 추가
}

void Webserv::set_indexing(Client &client)
{
	DIR *dir;
	int is_root = 0;
	struct dirent *ent;
	if (client.get_location_id() < 0 && client.get_is_file() != 1) // /abc
		dir = opendir(('.' + client.get_request().get_referer()).c_str());
	else// (client.get_request().get_referer() == "/")
	{
		is_root = 1;
		dir = opendir(('.' + client.get_request().get_referer()).c_str());
	}
	if (dir != NULL)
	{
		/* print all the files and directories within directory */
		client.get_response().get_response_str() += "<!DOCTYPE html>\n";
		client.get_response().get_response_str() += "<html>\n";
		client.get_response().get_response_str() += "<head>\n</head>\n";
		client.get_response().get_response_str() += "<body>\n";
		client.get_response().get_response_str() += "<h1>Index of ." + client.get_request().get_referer() + "</h1>\n";
		client.get_response().get_response_str() += "</a><br>\n";
		while ((ent = readdir(dir)) != NULL)
			client.get_response().set_autoindex(client.get_request().get_referer(), ent->d_name, is_root);
		closedir(dir);
	}
}

void Webserv::read_index(std::map<int, Client> &clients, int &id, int &server_id, const Config &Config)
{
	clients[id].set_RETURN(200);
	if (clients[id].get_route().find(".php") != std::string::npos || // CGI
		clients[id].get_route().find(".py") != std::string::npos)
	{
		this->run_cgi(Config.get_v_server()[server_id], clients[id].get_route(), clients[id]); // envp have to fix
		close(clients[id].get_write_fd());
		clients[clients[id].get_read_fd()].set_read_fd(id);
		clients[clients[id].get_read_fd()].set_status(need_to_cgi_read);
		clients[clients[id].get_read_fd()].set_pid(clients[id].get_pid());
		clients[id].set_status(WAIT);
		change_events(this->get_kq().get_change_list(), id, EVFILT_READ, EV_DELETE | EV_ENABLE, 0, 0, NULL);
		fcntl(clients[id].get_read_fd(), F_SETFL, O_NONBLOCK);
		change_events(this->get_kq().get_change_list(), clients[id].get_read_fd(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
		return ;
	}
	std::string referer;
	std::string root;
	if (clients[id].get_request().get_referer() == "/")
	{
		referer = Config.get_v_server()[server_id].get_index(); // config
		root = Config.get_v_server()[server_id].get_root();
	}
	else
	{
		referer = clients[id].get_request().get_referer(); // request
		root = Config.get_v_server()[server_id].get_root();
	}
	if (*referer.begin() == '/')
		referer.erase(referer.begin(), referer.begin() + 1);
	if (root != "" && *(root.end() - 1) != '/')
		root += '/';
	clients[id].set_route(root + referer); // ./ + /View/file.php

	std::cout << "route: " << clients[id].get_route() << std::endl; //////////////////////////


	int open_fd = open(('.' + clients[id].get_route()).c_str(), O_RDONLY);
	clients[id].set_open_file_name('.' + clients[id].get_route());
	if (open_fd < 0)
	{
		this->set_error_page(clients, id, 404);
		return ;
	}
	if (clients[id].get_request().get_method() == "DELETE")
	{
		clients[id].set_status(DELETE_ok);
		close(open_fd);
		return ;
	}
	clients[open_fd].set_read_fd(id); // event_fd:6 -> open_fd:10  발생된10->6
	clients[open_fd].set_status(need_to_is_file_read);

	clients[id].set_status(WAIT);
	change_events(this->get_kq().get_change_list(), id, EVFILT_READ, EV_DELETE | EV_ENABLE, 0, 0, NULL);
	fcntl(open_fd, F_SETFL, O_NONBLOCK);
	change_events(this->get_kq().get_change_list(), open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // read event 추가
}


std::map<std::string, std::string> &Webserv::get_mimes(void)
{
	return mimes;
}

Kqueue &Webserv::get_kq(void)
{
	return this->kq;
}

