#ifndef SERVER_BLOCK_HPP
#define SERVER_BLOCK_HPP

#include "Location_block.hpp"
#include "vector"

class Location_block;

class Server_block {
  public:
    std::vector<Location_block> locations;

    int client_limit_body_size;
    int request_limit_header_size;
    std::string user;
    std::string worker_processes;
    std::string listen;
    std::string server_name;
    std::string root;
    std::string index;
    std::string autoindex;
    std::string return_n;
    std::string error_page;
    std::string cgi_info;
    std::string allow_methods;
    std::string auth_key;

  public:
    const std::string &get_user(void) const { return user; }
    const std::string &get_worker_processes(void) const { return worker_processes; }
    const std::string &get_listen(void) const { return listen; }
    const std::string &get_server_name(void) const { return server_name; }
    const std::string &get_root(void) const { return root; }
    const std::string &get_index(void) const { return index; }
    const std::string &get_autoindex(void) const { return autoindex; }
    const std::string &get_return_n(void) const { return return_n; }
    const std::string &get_error_page(void) const { return error_page; }
    const std::string &get_cgi_info(void) const { return cgi_info; }
    const std::string &get_allow_methods(void) const { return allow_methods; }
    const std::string &get_auth_key(void) const { return auth_key; }
    const int &get_client_limit_body_size(void) const { return client_limit_body_size; }
    const int &get_request_limit_header_size(void) const { return request_limit_header_size; }

    void set_user(std::string &str) { user = str; }
    void set_worker_processes(std::string &str) { worker_processes = str; }
    void set_listen(std::string &str) { listen = str; }
    void set_server_name(std::string &str) { server_name = str; }
    void set_root(std::string &str) { root = str; }
    void set_index(std::string &str) { index = str; }
    void set_autoindex(std::string &str) { autoindex = str; }
    void set_return_n(std::string &str) { return_n = str; }
    void set_error_page(std::string &str) { error_page = str; }
    void set_cgi_info(std::string &str) { cgi_info = str; }
    void set_allow_methods(std::string &str) { allow_methods = str; }
    void set_auth_key(std::string &str) { auth_key = str; }
    void set_client_limit_body_size(int i) { client_limit_body_size = i; }
    void set_request_limit_header_size(int i) { request_limit_header_size = i; }
};

#endif