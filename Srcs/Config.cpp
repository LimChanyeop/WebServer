#include "../includes/Config.hpp"
#include "../includes/ParseUtils.hpp"

const std::string &Config::get_user(void) const { return user; }
const std::string &Config::get_worker_processes(void) const { return worker_processes; }
const std::string &Config::get_root(void) const { return root; }
const std::string &Config::get_index(void) const { return index; }
const std::string &Config::get_autoindex(void) const { return autoindex; }
const std::string &Config::get_return_n(void) const { return return_n; }
const std::string &Config::get_error_page(void) const { return error_page; }
const std::string &Config::get_cgi_info(void) const { return cgi_info; }
const std::string &Config::get_allow_methods(void) const { return allow_methods; }
const std::string &Config::get_auth_key(void) const { return auth_key; }
const int &Config::get_client_limit_body_size(void) const { return client_limit_body_size; }
const int &Config::get_request_limit_header_size(void) const { return request_limit_header_size; }

void Config::set_user(std::string str) { user = str; }
void Config::set_worker_processes(std::string str) { worker_processes = str; }
void Config::set_root(std::string str) { root = str; }
void Config::set_index(std::string str) { index = str; }
void Config::set_autoindex(std::string str) { autoindex = str; }
void Config::set_return_n(std::string str) { return_n = str; }
void Config::set_error_page(std::string str) { error_page = str; }
void Config::set_cgi_info(std::string str) { cgi_info = str; }
void Config::set_allow_methods(std::string str) { allow_methods = str; }
void Config::set_auth_key(std::string str) { auth_key = str; }
void Config::set_client_limit_body_size(int i) { client_limit_body_size = i; }
void Config::set_request_limit_header_size(int i) { request_limit_header_size = i; }

void Config::print_all(void) const
{
	std::cout << "client_limit_body_size " << client_limit_body_size << std::endl
			  << "request_limit_header_size " << request_limit_header_size << std::endl
			  << "user " << user << std::endl
			  << "worker_processes " << worker_processes << std::endl
			  << "root " << root << std::endl
			  << "index " << index << std::endl
			  << "autoindex " << autoindex << std::endl
			  << "return_n " << return_n << std::endl
			  << "error_page " << error_page << std::endl
			  << "cgi_info " << cgi_info << std::endl
			  << "allow_methods " << allow_methods << std::endl
			  << "auth_key " << auth_key << std::endl;
}

void Config::config_parsing(std::vector<std::string> lists) //, Config_base config_base)
{
	std::vector<std::string>::iterator it;
	for (it = lists.begin(); it != lists.end(); it++)
	{
		std::cout << "con it (" << *it << ")"<< std::endl;
		// std::cout << "(" << *it << ")" << find_key(*it) << std::endl;
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
			}
			temp += *(it + 1);
			this->set_user(temp);
			break;
		case 3:
			while (find_semi(*(it + 1)))
			{
				temp += *it;
				temp += ' ';
			}
			temp += *(it + 1);
			this->set_worker_processes(temp);
			break;
		case 6:
			while (find_semi(*(it + 1)))
			{
				temp += *it;
				temp += ' ';
			}
			temp += *(it + 1);
			this->set_root(temp);
			break;
		case 7:
			while (find_semi(*(it + 1)))
			{
				temp += *it;
				temp += ' ';
			}
			temp += *(it + 1);
			this->set_index(temp);
			break;
		case 8:
			while (find_semi(*(it + 1)))
			{
				temp += *it;
				temp += ' ';
			}
			temp += *(it + 1);
			this->set_autoindex(temp);
			break;
		case 9:
			while (find_semi(*(it + 1)))
			{
				temp += *it;
				temp += ' ';
			}
			temp += *(it + 1);
			this->set_return_n(temp);
			break;
		case 10:
			while (find_semi(*(it + 1)))
			{
				temp += *it;
				temp += ' ';
			}
			temp += *(it + 1);
			this->set_error_page(temp);
			break;
		case 11:
			while (find_semi(*(it + 1)))
			{
				temp += *it;
				temp += ' ';
			}
			temp += *(it + 1);
			this->set_cgi_info(temp);
			break;
		case 12:
			while (find_semi(*(it + 1)))
			{
				temp += *it;
				temp += ' ';
			}
			temp += *(it + 1);
			this->set_allow_methods(temp);
			break;
		case 13:
			while (find_semi(*(it + 1)))
			{
				temp += *it;
				temp += ' ';
			}
			temp += *(it + 1);
			this->set_auth_key(temp);
			break;
		case 14:												 // server
			v_server.push_back(Server(*this));					 // vec serverpush_back
			v_server[v_server.size() - 1].config_parsing(++it, lists); // v_server[i].server_block_parsing((&)it, conf_lists)
			break;													 // location

		// case 15: // location
		// 	this->set_auth_key(*(++it));
		// 	break;
		default:
			break;
		}
	}
}
