// #include <sys/types.h>
// #include <sys/stat.h>

#include "../includes/Webserv.hpp"
#include "../includes/Config.hpp"
#include "../includes/Fd.hpp"


Webserv::Webserv(/* args */){}

Webserv::~Webserv(){}

void change_events(std::vector<struct kevent> &change_list, uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data,
                   void *udata) // 이벤트를 생성하고 이벤트 목록에 추가하는 함수
{
    struct kevent temp_event;

    EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
    change_list.push_back(temp_event);
}

void Webserv::ready_webserv(Config &Config)
{
    std::vector<Server>::iterator it = Config.v_server.begin();
    for (; it != Config.v_server.end(); it++)
    {
        (*it).set_socket_fd(socket(AF_INET, SOCK_STREAM, 0)); // TCP: SOCK_STREAM UDP: SOCK_DGRAM

        if (it->get_socket_fd() <= 0) {
            std::cerr << "socket error" << std::endl;
            exit(0);
        }
		sockaddr_in &address = it->address;
        memset((char *)&address, 0, sizeof(address));
        address.sin_family = AF_INET; // tcp
    	address.sin_port = htons(atoi(it->get_listen().c_str()));
    	address.sin_addr.s_addr = htonl(INADDR_ANY); // open inbound restriction
    	memset(address.sin_zero, 0, sizeof(address.sin_zero));

        int optvalue = 1;
        setsockopt(it->get_socket_fd(), SOL_SOCKET, SO_REUSEADDR, &optvalue,
                sizeof(optvalue)); // to solve bind error

        int ret;
        if ((ret = bind(it->get_socket_fd(), (sockaddr *)&address, (socklen_t)sizeof(address))) == -1) {
            std::cerr << "bind error : return value = " << ret << std::endl;
            exit(0);
        }
        if ((listen(it->get_socket_fd(), 10)) < 0) {
            std::cerr << "listen error" << std::endl;
            exit(0); // why exit?
        }

        it->set_kq_fd(kqueue()); // kqueue() returns a file descriptor
        change_events(it->change_list, it->get_socket_fd(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    }
}

int Webserv::set_event(Config &config)
{
    int num_of_event;
    std::vector<Server>::iterator it = config.v_server.begin();
    for (; it != config.v_server.end(); it++)
    {
        int n_changes = it->change_list.size(); // number of changes = 등록하고자 하는 이벤트 수
        int n_event_list = 8;
        if ((num_of_event = kevent(it->get_kq_fd(), &it->change_list[0], n_changes, it->get_event_list(), n_event_list, NULL)) == -1) {
            std::cerr << "kevent error\n";
            exit(0);
        }
        it->change_list.clear(); // 등록 이벤트 목록 초기화
    }
    return num_of_event;
}
