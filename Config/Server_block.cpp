#include "../includes/Server_block.hpp"
#include "../includes/Http_block.hpp"
#include "../includes/Location_block.hpp"
#include "../includes/parseUtils.hpp"
#include <vector>

const std::string &Server_block::get_user(void) const { return user; }
const std::string &Server_block::get_worker_processes(void) const { return worker_processes; }
const std::string &Server_block::get_listen(void) const { return listen; }
const std::string &Server_block::get_server_name(void) const { return server_name; }
const std::string &Server_block::get_root(void) const { return root; }
const std::string &Server_block::get_index(void) const { return index; }
const std::string &Server_block::get_autoindex(void) const { return autoindex; }
const std::string &Server_block::get_return_n(void) const { return return_n; }
const std::string &Server_block::get_error_page(void) const { return error_page; }
const std::string &Server_block::get_cgi_info(void) const { return cgi_info; }
const std::string &Server_block::get_allow_methods(void) const { return allow_methods; }
const std::string &Server_block::get_auth_key(void) const { return auth_key; }
const int &Server_block::get_client_limit_body_size(void) const { return client_limit_body_size; }
const int &Server_block::get_request_limit_header_size(void) const { return request_limit_header_size; }

void Server_block::set_user(std::string &str) { user = str; }
void Server_block::set_worker_processes(std::string &str) { worker_processes = str; }
void Server_block::set_listen(std::string &str) { listen = str; }
void Server_block::set_server_name(std::string &str) { server_name = str; }
void Server_block::set_root(std::string &str) { root = str; }
void Server_block::set_index(std::string &str) { index = str; }
void Server_block::set_autoindex(std::string &str) { autoindex = str; }
void Server_block::set_return_n(std::string &str) { return_n = str; }
void Server_block::set_error_page(std::string &str) { error_page = str; }
void Server_block::set_cgi_info(std::string &str) { cgi_info = str; }
void Server_block::set_allow_methods(std::string &str) { allow_methods = str; }
void Server_block::set_auth_key(std::string &str) { auth_key = str; }
void Server_block::set_client_limit_body_size(int i) { client_limit_body_size = i; }
void Server_block::set_request_limit_header_size(int i) { request_limit_header_size = i; }

void Server_block::print_all(void) const {
    std::cout << "client_limit_body_size " << client_limit_body_size << std::endl
              << "request_limit_header_size " << request_limit_header_size << std::endl
              << "user " << user << std::endl
              << "worker_processes " << worker_processes
              << std::endl
              //   << "listen " << listen << std::endl
              //   << "server_name " << server_name << std::endl
              << "root " << root << std::endl
              << "index " << index << std::endl
              << "autoindex " << autoindex << std::endl
              << "return_n " << return_n << std::endl
              << "error_page " << error_page << std::endl
              << "cgi_info " << cgi_info << std::endl
              << "allow_methods " << allow_methods << std::endl
              << "auth_key " << auth_key << std::endl;
}

void Server_block::config_parsing(std::vector<std::string>::iterator &it, std::vector<std::string> lists) //, Config_base config_base)
{
    // std::vector<std::string>::iterator it;
    for (; it != lists.end() && *it != "}"; it++) {
        std::cout << "(" << *it << ")" << find_key(*it) << std::endl;
        switch (find_key(*it)) {
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
        // case 4:
        // 	this->set_listen(*(++it));
        // 	break;
        // case 5:
        // 	this->set_server_name(*(++it));
        // 	break;
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
        // case 14: // server
        //     this->set_auth_key(*(++it));
        //     break;
        case 15: // location
            // locations.push_back(Location_block());
            // locations[locations.size() - 1].config_parsing(it, lists);
            break;
        default:
            break;
        }
    }
}