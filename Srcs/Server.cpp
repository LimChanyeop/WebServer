#include "../includes/Server.hpp"

Server::Server(Config conf) : Fd()
{
	conf.set_client_limit_body_size(this->get_client_limit_body_size());
	conf.set_request_limit_header_size(this->get_request_limit_header_size());
	conf.set_user(this->get_user());
	conf.set_worker_processes(this->get_worker_processes());
	conf.set_root(this->get_root());
	conf.set_index(this->get_index());
	conf.set_autoindex(this->get_autoindex());
	conf.set_return_n(this->get_return_n());
	conf.set_error_page(this->get_error_page());
	conf.set_auth_key(this->get_auth_key());
	conf.set_cgi_info(this->get_cgi_info());
	conf.set_allow_methods(this->get_allow_methods());
}

const std::string &Server::get_user(void) const { return user; }
const std::string &Server::get_worker_processes(void) const { return worker_processes; }
const std::string &Server::get_listen(void) const { return listen; }
const std::string &Server::get_server_name(void) const { return server_name; }
const std::string &Server::get_root(void) const { return root; }
const std::string &Server::get_index(void) const { return index; }
const std::string &Server::get_autoindex(void) const { return autoindex; }
const std::string &Server::get_return_n(void) const { return return_n; }
const std::string &Server::get_error_page(void) const { return error_page; }
const std::string &Server::get_cgi_info(void) const { return cgi_info; }
const std::string &Server::get_allow_methods(void) const { return allow_methods; }
const std::string &Server::get_auth_key(void) const { return auth_key; }
const int &Server::get_client_limit_body_size(void) const { return client_limit_body_size; }
const int &Server::get_request_limit_header_size(void) const { return request_limit_header_size; }

struct kevent *Server::get_event_list(void) {return event_list;}

void Server::set_user(std::string str)
{
	user = str;
}
void Server::set_worker_processes(std::string str) { worker_processes = str; }
void Server::set_listen(std::string str) { listen = str; }
void Server::set_server_name(std::string str) { server_name = str; }
void Server::set_root(std::string str) { root = str; }
void Server::set_index(std::string str) { index = str; }
void Server::set_autoindex(std::string str) { autoindex = str; }
void Server::set_return_n(std::string str) { return_n = str; }
void Server::set_error_page(std::string str) { error_page = str; }
void Server::set_cgi_info(std::string str) { cgi_info = str; }
void Server::set_allow_methods(std::string str) { allow_methods = str; }
void Server::set_auth_key(std::string str) { auth_key = str; }
void Server::set_client_limit_body_size(int i) { client_limit_body_size = i; }
void Server::set_request_limit_header_size(int i) { request_limit_header_size = i; }

void Server::print_all(void) const
{
	std::cout << "client_limit_body_size " << client_limit_body_size << std::endl
			  << "request_limit_header_size " << request_limit_header_size << std::endl
			  << "user " << user << std::endl
			  << "worker_processes " << worker_processes
			  << std::endl
			  << "listen " << listen << std::endl
			  << "server_name " << server_name << std::endl
			  << "root " << root << std::endl
			  << "index " << index << std::endl
			  << "autoindex " << autoindex << std::endl
			  << "return_n " << return_n << std::endl
			  << "error_page " << error_page << std::endl
			  << "cgi_info " << cgi_info << std::endl
			  << "allow_methods " << allow_methods << std::endl
			  << "auth_key " << auth_key << std::endl;
}

void Server::config_parsing(std::vector<std::string>::iterator &it, std::vector<std::string> &lists) //, Config_base config_base)
{
	for (; it != lists.end() && *it != "}"; it++)
	{
		std::cout << "ser it (" << *it << ")"<< std::endl;
		std::string temp = "";
		switch (find_key(*it))
		{
		case 0:
			this->set_client_limit_body_size(atoi((*(it + 1)).c_str()));
			break;
		case 1:
			this->set_request_limit_header_size(atoi((*(it + 1)).c_str()));
			break;
		case 2:
			while (find_semi(*(it + 1)))
			{
				temp += *it;
				temp += ' ';
				it++;
			}
			temp += *(it + 1);
			this->set_user(temp);
			break;
		case 3:
			while (find_semi(*(it + 1)))
			{
				temp += *it;
				temp += ' ';
				it++;
			}
			temp += *(it + 1);
			this->set_worker_processes(temp);
			break;
		case 4:
			while (find_semi(*(it + 1)))
			{
				temp += *it;
				temp += ' ';
				it++;
			}
			temp += *(it + 1);
			this->set_listen(temp);
			break;
		case 5:
			while (find_semi(*(it + 1)))
			{
				temp += *it;
				temp += ' ';
				it++;
			}
			temp += *(it + 1);
			this->set_server_name(temp);
			break;
		case 6:
			while (find_semi(*(it + 1)))
			{
				temp += *it;
				temp += ' ';
				it++;
			}
			temp += *(it + 1);
			this->set_root(temp);
			break;
		case 7:
			while (find_semi(*(it + 1)))
			{
				temp += *it;
				temp += ' ';
				it++;
			}
			temp += *(it + 1);
			this->set_index(temp);
			break;
		case 8:
			while (find_semi(*(it + 1)))
			{
				temp += *it;
				temp += ' ';
				it++;
			}
			temp += *(it + 1);
			this->set_autoindex(temp);
			break;
		case 9:
			while (find_semi(*(it + 1)))
			{
				temp += *it;
				temp += ' ';
				it++;
			}
			temp += *(it + 1);
			this->set_return_n(temp);
			break;
		case 10:
			while (find_semi(*(it + 1)))
			{
				temp += *it;
				temp += ' ';
				it++;
			}
			temp += *(it + 1);
			this->set_error_page(temp);
			break;
		case 11:
			while (find_semi(*(it + 1)))
			{
				temp += *it;
				temp += ' ';
				it++;
			}
			temp += *(it + 1);
			this->set_cgi_info(temp);
			break;
		case 12:
			while (find_semi(*(it + 1)))
			{
				temp += *it;
				temp += ' ';
				it++;
			}
			temp += *(it + 1);
			this->set_allow_methods(temp);
			break;
		case 13:
			while (find_semi(*(it + 1)))
			{
				temp += *it;
				temp += ' ';
				it++;
			}
			temp += *(it + 1);
			this->set_auth_key(temp);
			break;
		case 14: // server
			std::cerr << "double server!\n";
			break;
		case 15:														 // location
			v_location.push_back(Location(*this));					 // vec v_location push_back
			v_location[v_location.size() - 1].config_parsing(++it, lists); // v_location[i].server_parsing((&)it, conf_lists)
			break;

		default:
			break;
		}
	}
}

void Server::set_socket_fd(int fd_)
{
	fd = fd_;
}

const int &Server::get_socket_fd(void) const
{
	return fd;
}

void Server::set_kq_fd(int fd_)
{
	fd = fd_;
}

const int &Server::get_kq_fd(void) const
{
	return fd;
}

