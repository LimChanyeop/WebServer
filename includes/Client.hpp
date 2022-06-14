#ifndef INFO_HPP
#define INFO_HPP

#include "./Request.hpp"
#include "./Response.hpp"
#include "./Server.hpp"
#include <vector>

class Server;

enum status { r_opened, r_cgi_opened, w_opened, response_ok, cgi_response_ok, complete };

class Client {
  public:
    int server_sock;
    // std::vector<Server>::iterator server_it; // server
    int server_id;
    int location_id;
    int read_fd;
    int write_fd;
    int status;
    int is_file;
    int RETURN;
    int pid;
    // std::string request;
    std::string header;
    Response response;
    Request request;
    std::string route;
    char ip[20];

  public:
    Client(/* args */);
    ~Client();

    void request_parsing(int event_ident);

    const int &get_server_sock(void) const;
    // std::vector<Server>::iterator &get_server_it(void); // not const
    const int &get_status(void) const;
    const int &get_read_fd(void) const;
    const int &get_write_fd(void) const;
    const std::string &get_route(void) const;
    const int &get_server_id(void) const;
    const int &get_location_id(void) const;
    // char *&get_ip();

    void set_server_sock(int fd);
    // void set_server_it(std::vector<Server>::iterator server_it_);
    void set_status(int y_n);
    void set_read_fd(int fd);
    void set_write_fd(int fd);
    void set_route(std::string str);
    void set_server_id(int i);
    void set_location_id(int i);
};

#endif