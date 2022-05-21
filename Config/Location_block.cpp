#include "../includes/Location_block.hpp"

Location_block::Location_block(Server_block sb)
{
	sb.set_client_limit_body_size(this->get_client_limit_body_size());
	sb.set_request_limit_header_size(this->get_request_limit_header_size());
	sb.set_user(this->get_user());
	sb.set_worker_processes(this->get_worker_processes());
	sb.set_listen(this->get_listen());			 // servget_listen()er
	sb.set_server_name(this->get_server_name()); // servget_server_name()er
	sb.set_root(this->get_root());
	sb.set_index(this->get_index());
	sb.set_autoindex(this->get_autoindex());
	sb.set_return_n(this->get_return_n());
	sb.set_error_page(this->get_error_page());
	sb.set_auth_key(this->get_auth_key());
	sb.set_cgi_info(this->get_cgi_info());
	sb.set_allow_methods(this->get_allow_methods());
}

const std::string &Location_block::get_user(void) const { return user; }
const std::string &Location_block::get_worker_processes(void) const { return worker_processes; }
const std::string &Location_block::get_listen(void) const { return listen; }
const std::string &Location_block::get_server_name(void) const { return server_name; }
const std::string &Location_block::get_root(void) const { return root; }
const std::string &Location_block::get_index(void) const { return index; }
const std::string &Location_block::get_autoindex(void) const { return autoindex; }
const std::string &Location_block::get_return_n(void) const { return return_n; }
const std::string &Location_block::get_error_page(void) const { return error_page; }
const std::string &Location_block::get_cgi_info(void) const { return cgi_info; }
const std::string &Location_block::get_allow_methods(void) const { return allow_methods; }
const std::string &Location_block::get_auth_key(void) const { return auth_key; }
const int &Location_block::get_client_limit_body_size(void) const { return client_limit_body_size; }
const int &Location_block::get_request_limit_header_size(void) const { return request_limit_header_size; }

void Location_block::set_user(std::string str) { user = str; }
void Location_block::set_worker_processes(std::string str) { worker_processes = str; }
void Location_block::set_listen(std::string str) { listen = str; }
void Location_block::set_server_name(std::string str) { server_name = str; }
void Location_block::set_root(std::string str) { root = str; }
void Location_block::set_index(std::string str) { index = str; }
void Location_block::set_autoindex(std::string str) { autoindex = str; }
void Location_block::set_return_n(std::string str) { return_n = str; }
void Location_block::set_error_page(std::string str) { error_page = str; }
void Location_block::set_cgi_info(std::string str) { cgi_info = str; }
void Location_block::set_allow_methods(std::string str) { allow_methods = str; }
void Location_block::set_auth_key(std::string str) { auth_key = str; }
void Location_block::set_client_limit_body_size(int i) { client_limit_body_size = i; }
void Location_block::set_request_limit_header_size(int i) { request_limit_header_size = i; }

void Location_block::print_all(void) const
{
	std::cout << "client_limit_body_size " << client_limit_body_size << std::endl
			  << "request_limit_header_size " << request_limit_header_size << std::endl
			  << "user " << user << std::endl
			  << "worker_processes " << worker_processes << std::endl
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

void Location_block::config_parsing(std::vector<std::string>::iterator &it, std::vector<std::string> &lists) //, Config_base config_base)
{
	this->location = *it;
	for (; it != lists.end() && *it != "}"; it++)
	{
		std::string temp = "";
		switch (find_key(*it))
		{
		case 0:
			this->set_client_limit_body_size(atoi((*(++it)).c_str()));
			break;
		case 1:
			this->set_request_limit_header_size(atoi((*(++it)).c_str()));
			break;
		case 2:
			while (find_semi(*(++it)))
			{
				temp += *it;
				temp += ' ';
			}
			temp += *it;
			this->set_user(temp);
			break;
		case 3:
			while (find_semi(*(++it)))
			{
				temp += *it;
				temp += ' ';
			}
			temp += *it;
			this->set_worker_processes(temp);
			break;
		case 6:
			while (find_semi(*(++it)))
			{
				temp += *it;
				temp += ' ';
			}
			temp += *it;
			this->set_root(temp);
			break;
		case 7:
			while (find_semi(*(++it)))
			{
				temp += *it;
				temp += ' ';
			}
			temp += *it;
			this->set_index(temp);
			break;
		case 8:
			while (find_semi(*(++it)))
			{
				temp += *it;
				temp += ' ';
			}
			temp += *it;
			this->set_autoindex(temp);
			break;
		case 9:
			while (find_semi(*(++it)))
			{
				temp += *it;
				temp += ' ';
			}
			temp += *it;
			this->set_return_n(temp);
			break;
		case 10:
			while (find_semi(*(++it)))
			{
				temp += *it;
				temp += ' ';
			}
			temp += *it;
			this->set_error_page(temp);
			break;
		case 11:
			while (find_semi(*(++it)))
			{
				temp += *it;
				temp += ' ';
			}
			temp += *it;
			this->set_cgi_info(temp);
			break;
		case 12:
			while (find_semi(*(++it)))
			{
				temp += *it;
				temp += ' ';
			}
			temp += *it;
			this->set_allow_methods(temp);
			break;
		case 13:
			while (find_semi(*(++it)))
			{
				temp += *it;
				temp += ' ';
			}
			temp += *it;
			this->set_auth_key(temp);
			break;
		case 14: // server
			break;
		// case 15: // location
		// 	break;
		default:
			break;
		}
	}
}