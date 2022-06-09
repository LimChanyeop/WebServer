#include "../includes/Client.hpp"

Client::Client(/* args */) : server_sock(-1), status(no), read_fd(-1), server_id(-1) {}

Client::~Client() {}

void Client::request_parsing(int event_ident) {
    char READ[1024] = {0};
    int valread;
    std::string request;
    // int valread = recv(acc_socket, request, 1024, 0);

    while ((valread = read(event_ident, READ, 1023)) == 1023) {
        request += READ;
    }
    if (valread >= 0) {
        request += READ;
    }
    std::cout << "Client::request:" << request << std::endl;
    this->request.split_request(request);
    this->request.request_parsing(this->request.requests);
}

const int &Client::get_server_sock(void) const { return this->server_sock; }
std::vector<Server>::iterator &Client::get_server_it(void) // not const
{
    return this->server_it;
}
const int &Client::get_status(void) const { return this->status; }
const int &Client::get_read_fd(void) const { return this->read_fd; }
const std::string &Client::get_route(void) const { return this->route; }
const int &Client::get_server_id(void) const { return this->server_id; }
const int &Client::get_location_id(void) const { return this->location_id; }

void Client::set_server_sock(int fd) { this->server_sock = fd; }
void Client::set_status(int ok) { this->status = ok; }
void Client::set_read_fd(int fd) { this->read_fd = fd; }
void Client::set_server_it(std::vector<Server>::iterator server_it_) { this->server_it = server_it_; }
void Client::set_route(std::string str) { this->route = str; }
void Client::set_server_id(int i) { this->server_id = i; }
void Client::set_location_id(int i) { this->location_id = i; }
