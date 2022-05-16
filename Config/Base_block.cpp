#include "../includes/Base_block.hpp"
#include "../includes/parseUtils.hpp"

const std::string &Base_block::get_user(void) const { return user; }
const std::string &Base_block::get_worker_processes(void) const { return worker_processes; }
const std::string &Base_block::get_root(void) const { return root; }
const std::string &Base_block::get_index(void) const { return index; }
const std::string &Base_block::get_autoindex(void) const { return autoindex; }
const std::string &Base_block::get_return_n(void) const { return return_n; }
const std::string &Base_block::get_error_page(void) const { return error_page; }
const std::string &Base_block::get_cgi_info(void) const { return cgi_info; }
const std::string &Base_block::get_allow_methods(void) const { return allow_methods; }
const std::string &Base_block::get_auth_key(void) const { return auth_key; }
const int &Base_block::get_client_limit_body_size(void) const { return client_limit_body_size; }
const int &Base_block::get_request_limit_header_size(void) const { return request_limit_header_size; }

void Base_block::set_user(std::string str) { user = str; }
void Base_block::set_worker_processes(std::string str) { worker_processes = str; }
void Base_block::set_root(std::string str) { root = str; }
void Base_block::set_index(std::string str) { index = str; }
void Base_block::set_autoindex(std::string str) { autoindex = str; }
void Base_block::set_return_n(std::string str) { return_n = str; }
void Base_block::set_error_page(std::string str) { error_page = str; }
void Base_block::set_cgi_info(std::string str) { cgi_info = str; }
void Base_block::set_allow_methods(std::string str) { allow_methods = str; }
void Base_block::set_auth_key(std::string str) { auth_key = str; }
void Base_block::set_client_limit_body_size(int i) { client_limit_body_size = i; }
void Base_block::set_request_limit_header_size(int i) { request_limit_header_size = i; }

void Base_block::print_all(void) const
{
	std::cout << "client_limit_body_size " << client_limit_body_size << std::endl
			  << "request_limit_header_size " << request_limit_header_size << std::endl
			  << "user " << user << std::endl
			  << "worker_processes " << worker_processes
			  << std::endl
			  << "root " << root << std::endl
			  << "index " << index << std::endl
			  << "autoindex " << autoindex << std::endl
			  << "return_n " << return_n << std::endl
			  << "error_page " << error_page << std::endl
			  << "cgi_info " << cgi_info << std::endl
			  << "allow_methods " << allow_methods << std::endl
			  << "auth_key " << auth_key << std::endl;
}

void Base_block::config_parsing(std::vector<std::string> lists) //, Config_base config_base)
{
	std::vector<std::string>::iterator it;
	for (it = lists.begin(); it != lists.end(); it++)
	{
		std::cout << "(" << *it << ")" << find_key(*it) << std::endl;
		switch (find_key(*it))
		{
		case 0:
			this->set_client_limit_body_size(atoi((*(++it)).c_str()));
			break;
		case 1:
			this->set_request_limit_header_size(atoi((*(++it)).c_str()));
			break;
		case 2:
			this->set_user(*(++it));
			break;
		case 3:
			this->set_worker_processes(*(++it));
			break;
		case 6:
			this->set_root(*(++it));
			break;
		case 7:
			this->set_index(*(++it));
			break;
		case 8:
			this->set_autoindex(*(++it));
			break;
		case 9:
			this->set_return_n(*(++it));
			break;
		case 10:
			this->set_error_page(*(++it));
			break;
		case 11:
			this->set_cgi_info(*(++it));
			break;
		case 12:
			this->set_allow_methods(*(++it));
			break;
		case 13:
			this->set_auth_key(*(++it));
			break;
		case 14:													 // server
			servers.push_back(Server_block(*this));					 // vec servers push_back
			servers[servers.size() - 1].config_parsing(++it, lists); // servers[i].server_block_parsing((&)it, conf_lists)
			break;
		// case 15: // location
		// 	this->set_auth_key(*(++it));
		// 	break;
		default:
			break;
		}
	}
}