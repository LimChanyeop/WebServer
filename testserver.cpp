// #include <sys/time.h>
// #include <fcntl.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <stdlib.h>
// #include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <bitset>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include <sys/event.h> // for kqueue

#define PORT 4242

void set_sockaddr(sockaddr_in *address) {
    memset((char *)address, 0, sizeof(*address));
    address->sin_family = AF_INET;
    address->sin_port = htons(PORT);
    address->sin_addr.s_addr = htonl(INADDR_ANY);
    memset(address->sin_zero, 0, sizeof(address->sin_zero));
}

int main() {
    int server_fd =
        socket(AF_INET, SOCK_STREAM, 0); // TCP: SOCK_STREAM UDP: SOCK_DGRAM
    if (server_fd <= 0) {
        std::cerr << "socket error" << std::endl;
        return 0;
    }
    sockaddr_in address;
    set_sockaddr(&address);

    int optvalue = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optvalue,
               sizeof(optvalue)); // to solve bind error

    int address_len = sizeof(address);
    if (int ret = bind(server_fd, (sockaddr *)&address, address_len) <
                  0) // (socklen_t)sizeof, bind suc:0 fa:-1
    {
        std::cerr << "bind error : return value = " << ret << std::endl;
        return 0;
    }

    int listen_fd;
    if ((listen_fd = listen(server_fd, 10)) < 0) {
        std::cerr << "listen error" << std::endl;
        exit(0); // why exit?
    }

    int kq_fd = kqueue(); // kqueue() returns a file descriptor

    std::vector<struct kevent> change_list;
    struct kevent event_list[8];
    struct kevent temp_event;
    EV_SET(&temp_event, server_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    change_list.push_back(temp_event);

    int num_of_event;
    while (1) {
        std::cout << "waiting for new connection...\n";
        struct kevent *cur_event;
        int n_changes =
            change_list.size(); // number of changes = 등록하고자 하는 이벤트 수
        if ((num_of_event = kevent(kq_fd, &change_list[0], n_changes,
                                   event_list, 8, NULL)) == -1) {
            std::cerr << "kevent error\n";
            exit(0);
        }
        std::cout << "num_of_event : " << num_of_event << std::endl;
        for (int i = 0; i < num_of_event; ++i) {
            cur_event = &event_list[i];
            std::cout << "ident : " << cur_event->ident << std::endl
                      << "filter : " << std::bitset<16>(cur_event->filter)
                      << std::endl // 모든 bit = 1 출력
                      << "flags : " << std::bitset<16>(cur_event->flags)
                      << std::endl
                      << "fflags : " << std::bitset<32>(cur_event->fflags)
                      << std::endl
                      << "data : " << std::bitset<16>(cur_event->data)
                      << std::endl
                      << "udata : " << cur_event->udata << std::endl;
            if (event_list->filter == EVFILT_READ) {
                std::cout << "accept READ Event" << std::endl;
            }
        }
        int acc_socket;
        if ((acc_socket = accept(server_fd, // block fn
                                 (sockaddr *)&address,
                                 (socklen_t *)&address_len)) < 0) //
        {
            std::cerr << "accept error" << std::endl;
            exit(0);
        }

        std::cout << "Connected!\n";

        char request[1024] = {0};
        int valread = read(acc_socket, request, 1024);
        // int valread = recv(acc_socket, request, 1024, 0);
        std::string str_buf = request;
        std::cout << str_buf << std::endl;

        std::string response_str =
            "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: ";
        response_str += std::to_string(str_buf.length() + 15);
        response_str += "\n\nyour request :\n";
        response_str += str_buf; // IMPORTANT!!
        write(acc_socket, response_str.c_str(), response_str.length());
        close(acc_socket);
    }
    close(kq_fd);
    return 0;
}
