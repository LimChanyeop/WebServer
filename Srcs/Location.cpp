#include "../includes/Location.hpp"

Location::Location(Server sb) : redi_status(0)
{
	this->set_client_limit_body_size(sb.get_client_limit_body_size());
	this->set_request_limit_header_size(sb.get_request_limit_header_size());
	this->set_user(sb.get_user());
	this->set_worker_processes(sb.get_worker_processes());
	this->set_listen(sb.get_listen());
	this->set_server_name(sb.get_server_name());
	this->set_root(sb.get_root());
	this->set_index(sb.get_index());
	this->set_autoindex(sb.get_autoindex());
	this->set_auth_key(sb.get_auth_key());
	this->set_cgi_path(sb.get_cgi_path());
	this->set_allow_methods(sb.get_allow_methods());
}

const std::string &Location::get_user(void) const { return user; }
const std::string &Location::get_worker_processes(void) const { return worker_processes; }
const std::string &Location::get_listen(void) const { return listen; }
const std::string &Location::get_server_name(void) const { return server_name; }
const std::string &Location::get_root(void) const { return root; }
const std::string &Location::get_index(void) const { return index; }
const std::string &Location::get_autoindex(void) const { return autoindex; }
const std::string &Location::get_error_page(void) const { return error_page; }
const std::string &Location::get_cgi_path(void) const { return cgi_path; }
const std::string &Location::get_allow_methods(void) const { return allow_methods; }
const std::string &Location::get_auth_key(void) const { return auth_key; }
const std::string &Location::get_location(void) const { return location; }
const int &Location::get_client_limit_body_size(void) const { return client_limit_body_size; }
const int &Location::get_request_limit_header_size(void) const { return request_limit_header_size; }
const int &Location::get_redi_status(void) const { return redi_status; }
const std::string &Location::get_redi_root(void) const { return redi_root; }

void Location::set_user(std::string str) { user = str; }
void Location::set_worker_processes(std::string str) { worker_processes = str; }
void Location::set_listen(std::string str) { listen = str; }
void Location::set_server_name(std::string str) { server_name = str; }
void Location::set_root(std::string str) { root = str; }
void Location::set_index(std::string str) { index = str; }
void Location::set_autoindex(std::string str) { autoindex = str; }
void Location::set_error_page(std::string str) { error_page = str; }
void Location::set_cgi_path(std::string str) { cgi_path = str; }
void Location::set_allow_methods(std::string str) { allow_methods = str; }
void Location::set_auth_key(std::string str) { auth_key = str; }
void Location::set_client_limit_body_size(int i) { client_limit_body_size = i; }
void Location::set_request_limit_header_size(int i) { request_limit_header_size = i; }
void Location::set_redi_return(int i, std::string root)
{
	redi_status = i;
	redi_root = root;
}


void Location::config_parsing(std::vector<std::string>::iterator &it, std::vector<std::string> &lists) //, Config_base config_base)
{
	this->location = *it;
	for (; it != lists.end() && *it != "}"; it++)
	{
		int status = 0;
		std::string temp = "";
		// std::cerr << "it: " << *it << ", find_key: " << find_key(*it) << std::endl;
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
			status = atoi((++it)->c_str());
			temp = *(++it);
			if (*(temp.end() - 1) == ';')
				temp.pop_back();
			this->set_redi_return(status, temp);
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
			break;
		// case 15: // location
		// 	break;
		default:
			break;
		}
	}
}