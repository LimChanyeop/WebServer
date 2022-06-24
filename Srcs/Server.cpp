#include "../includes/Server.hpp"

Server::Server(Config conf) : Fd(), listen("7777")
{
	this->set_client_limit_body_size(conf.get_client_limit_body_size());
	this->set_request_limit_header_size(conf.get_request_limit_header_size());
	this->set_user(conf.get_user());
	this->set_worker_processes(conf.get_worker_processes());
	this->set_root(conf.get_root());
	this->set_index(conf.get_index());
	this->set_autoindex(conf.get_autoindex());
	this->set_return_n(conf.get_return_n());
	std::map<int, std::string> err = conf.get_error_page();
	this->set_error_page(err);
	this->set_auth_key(conf.get_auth_key());
	this->set_cgi_path(conf.get_cgi_path());
	this->set_allow_methods(conf.get_allow_methods());
}

const std::string &Server::get_user(void) const { return user; }
const std::string &Server::get_worker_processes(void) const { return worker_processes; }
const std::string &Server::get_listen(void) const { return listen; }
const std::string &Server::get_server_name(void) const { return server_name; }
const std::string &Server::get_root(void) const { return root; }
const std::string &Server::get_index(void) const { return index; }
const std::string &Server::get_autoindex(void) const { return autoindex; }
const std::string &Server::get_return_n(void) const { return return_n; }
const std::map<int, std::string> &Server::get_error_page(void) const { return error_page; }
const std::string &Server::get_cgi_path(void) const { return cgi_path; }
const std::string &Server::get_allow_methods(void) const { return allow_methods; }
const std::string &Server::get_auth_key(void) const { return auth_key; }
const int &Server::get_client_limit_body_size(void) const { return client_limit_body_size; }
const int &Server::get_request_limit_header_size(void) const { return request_limit_header_size; }
const std::vector<Location> &Server::get_v_location(void) const { return v_location; }
const std::string &Server::get_limit_except(void) const { return limit_except; }

void Server::set_user(std::string str) { user = str; }
void Server::set_worker_processes(std::string str) { worker_processes = str; }
void Server::set_listen(std::string str) { listen = str; }
void Server::set_server_name(std::string str) { server_name = str; }
void Server::set_root(std::string str) { root = str; }
void Server::set_index(std::string str) { index = str; }
void Server::set_autoindex(std::string str) { autoindex = str; }
void Server::set_return_n(std::string str) { return_n = str; }
void Server::set_error_page(std::map<int, std::string> err) { error_page = err; }
void Server::set_error_page(int i, std::string err) { error_page[i] = err; }

void Server::set_cgi_path(std::string str)
{
	str.erase(0, str.find("./"));
	cgi_path = str;
}
void Server::set_allow_methods(std::string str) { allow_methods = str; }
void Server::set_auth_key(std::string str) { auth_key = str; }
void Server::set_client_limit_body_size(int i) { client_limit_body_size = i; }
void Server::set_request_limit_header_size(int i) { request_limit_header_size = i; }
void Server::set_limit_except(std::string str) { limit_except = str; }

void Server::config_parsing(std::vector<std::string>::iterator &it, std::vector<std::string> &lists) //, Config_base config_base)
{
	for (; it != lists.end() && *it != "}"; it++)
	{
		// std::cerr << "ser::it: " << *it << ", find_key: " << find_key(*it) << std::endl;
		std::string temp = "";
		unsigned int i = 0;
		unsigned long find;
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
		case 9: // return n
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
			i = atoi((++it)->c_str());
			temp = '.' + *(++it);
			while ((find = temp.find(";")) != std::string::npos)
				temp.erase(find, find + 1);
			this->set_error_page(i, temp);
			break;
		case 11:
			while (find_semi(*(it + 1)))
			{
				temp += *it;
				temp += ' ';
				it++;
			}
			temp += *(it + 1);
			this->set_cgi_path(temp);
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
			std::cerr << "Config Parsing Error, Server in Server\n";
			exit(-1);
			break;
		case 15:														   // location
			v_location.push_back(Location(*this));						   // vec v_location push_back
			v_location[v_location.size() - 1].config_parsing(++it, lists); // v_location[i].server_parsing((&)it, conf_lists)
			break;
		case 16: // limit
			while (find_semi(*(it + 1)))
			{
				temp += *(++it);
				temp += ' ';
			}
			temp += *(it + 1);
			this->limit_except = temp;
			break;
		default:
			break;
		}
	}
}
