// #include <sys/time.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <stdlib.h>
// #include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <memory>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <bitset> //

#include <sys/event.h> // for kqueue

#define PORT 4242

void change_events(std::vector<struct kevent> &change_list, uintptr_t ident, int16_t filter,
                   uint16_t flags, uint32_t fflags, intptr_t data, void *udata)
{
    struct kevent temp_event;

    EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
    change_list.push_back(temp_event);
}

void set_sockaddr(sockaddr_in *address)
{
    memset((char *)address, 0, sizeof(*address)); // ?
    address->sin_family = AF_INET;
    address->sin_port = htons(PORT);
    // std::cout << std::bitset<16>(htons(PORT)) << std::endl; // -> \n
    address->sin_addr.s_addr = htonl(INADDR_ANY);
    memset(address->sin_zero, 0, sizeof(address->sin_zero));
}

int main()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0); // TCP: SOCK_STREAM UDP: SOCK_DGRAM
    sockaddr_in address;
    int listen_fd;

    if (server_fd <= 0)
    {
        std::cerr << "socket error" << std::endl;
        return 0;
    }

    set_sockaddr(&address);

    int optvalue = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optvalue, sizeof(optvalue)); // for bind error

    int address_len = sizeof(address);
    if (int ret = bind(server_fd, (sockaddr *)&address, address_len) < 0) // (socklen_t)sizeof, bind suc:0 fa:-1
    {
        std::cout << ret << std::endl;
        std::cerr << "bind error " << bind(server_fd, (sockaddr *)&address, address_len) << std::endl;
        return 0;
    }

    if ((listen_fd = listen(server_fd, 10)) < 0)
    {
        std::cerr << "listen error" << std::endl;
        exit(0); // why exit?
    }

    int kq_fd = kqueue(); // kqqqqq
    std::vector<struct kevent> change_list;
    struct kevent event_list[8];
    std::map<int, std::string> clients;

    // change_events(change_list, server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    struct kevent temp_event;
    EV_SET(&temp_event, server_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    change_list.push_back(temp_event);

    int kevent_v;
    while (1)
    {
        struct kevent *cur_event;
        std::string str_buf;

        std::cout << "waiting for new connection...\n";

        if ((kevent_v = kevent(kq_fd, &change_list[0], change_list.size(), event_list, 8, NULL)) == -1) // kqqq
        {
            std::cerr << "kevent error\n";
            exit(0);
        }

        change_list.clear();
        for (int i = 0; i < kevent_v; i++)
        {
            cur_event = &event_list[i];
            std::cout << cur_event->ident << std::endl
                      << cur_event->filter << std::endl
                      << cur_event->flags << std::endl
                      << cur_event->fflags << std::endl
                      << cur_event->data << std::endl
                      << cur_event->udata << std::endl;

            if (cur_event->filter == EVFILT_READ)
            {
                if (cur_event->ident == server_fd)
                {
                    int acc_socket;
                    if ((acc_socket = accept(server_fd, NULL, NULL)) == -1) // (sockaddr *)&address, (socklen_t *)&address_len)) < 0) // block fn
                    {
                        std::cerr << "accept error" << std::endl;
                        exit(0);
                    }
                    fcntl(acc_socket, F_SETFL, O_NONBLOCK);
                    change_events(change_list, acc_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
                    change_events(change_list, acc_socket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
                    clients[acc_socket] = "";
                }
                else if (clients.find(cur_event->ident) != clients.end())
                {
                    std::cout << "Connected!\n";

                    char request[1024] = {0};
                    int valread = read(cur_event->ident, request, 1024);
                    str_buf = request;
                    // int valread = recv(acc_socket, request, 1024, 0);
                    std::cout << str_buf << std::endl;
                }
            }
            else if (cur_event->filter == EVFILT_WRITE)
            {
                std::string response_str = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: ";
                response_str += std::to_string(str_buf.length() + 15);
                response_str += "\n\nyour request :\n";
                response_str += str_buf;
                write(cur_event->ident, response_str.c_str(), response_str.length());
                close(cur_event->ident);
            }
        }
        // int acc_socket;
        // if ((acc_socket = accept(server_fd,												  // block fn
        // 						 (sockaddr *)&address, (socklen_t *)&address_len)) == -1) //
        // {
        // 	std::cerr << "accept error" << std::endl;
        // 	exit(0);
        // }

        // std::cout << "Connected!\n";

        // char request[1024] = {0};
        // int valread = read(acc_socket, request, 1024);
        // // int valread = recv(acc_socket, request, 1024, 0);
        // std::string str_buf = request;
        // std::cout << str_buf << std::endl;

        // std::string response_str = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: ";
        // response_str += std::to_string(str_buf.length() + 15);
        // response_str += "\n\nyour request :\n";
        // response_str += str_buf;

        // write(cur_event->ident, response_str.c_str(), response_str.length());
    }
    // close(cur_event->ident);

    return 0;
}
