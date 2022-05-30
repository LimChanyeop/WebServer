// #include <sys/types.h>
// #include <sys/stat.h>

#include "../includes/Webserv.hpp"
#include "../includes/Fd.hpp"
#include "../includes/Request.hpp"
#include "../includes/ParseUtils.hpp"


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

		sockaddr_in &address = it->get_address();
        memset((char *)&address, 0, sizeof(address));
        address.sin_family = AF_INET; // tcp
    	address.sin_port = htons(atoi(it->get_listen().c_str()));
    	address.sin_addr.s_addr = htonl(INADDR_ANY); // open inbound restriction
    	memset(address.sin_zero, 0, sizeof(address.sin_zero));
        // std::cout << "port::" << ntohs(it->address.sin_port) << std::endl; // 4242 ok

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
    }
}

int Webserv::find_server_id(int i, Config config, Request rq, Kqueue kq)
{
    std::string port = ""; // 왜 포트를 못찾았을까? -> 포트를 파싱 안했었넹~ok -> 근데도 못찾네~
	int server_id;
	std::cout << "*find*" << kq.clients.find(kq.event_list[i].ident)->second << std::endl;
	if (kq.clients.find(kq.event_list[i].ident) != kq.clients.end())
	{
		server_id = 0;
		while (server_id < config.v_server.size())
		{
			std::cout << "listen:" << config.v_server[server_id].get_listen() << "vs" << rq.host << std::endl;
			// 왜 for문 끼워놓으면 무한루프돌까~~ㅡㅡ while문은 또 왜 되는것인가~
			if (atoi(config.v_server[server_id].get_listen().c_str()) == atoi(rq.get_host().c_str())) // 못찾는게,, 이게 다르데;;
			{				
				std::cout << "==================\n";														 //
				std::cout << "same - " << server_id << "\n";
				std::cout << "==================\n";
				port = rq.get_host();
				break;
			}
			server_id++;
		}
		if (server_id == config.v_server.size())
		{
			std::cerr << "Host not found, Are you Favicon?\n";
			// i = 1;
			// exit(0);
			// kq.clients.erase(kq.event_list[i].ident);
		}
	}
	std::string temp;
	int it = rq.referer.find(port.c_str());
	if (it == rq.referer.size() || it < 0)
	{
		std::cerr << "Cant find ref in port\n";
	}
	else
		rq.referer.erase(0, it);
    return server_id;
}

int Webserv::find_location_id(int server_id, Config config, Request rq, Kqueue kq){
    int location_id;
	std::cout << config.v_server[server_id].v_location.size() << std::endl;
	for (location_id = 0; location_id < config.v_server[server_id].v_location.size(); location_id++)
	{
		std::cout << "TEST-location:" << config.v_server[server_id].v_location[location_id].location << "vs" << rq.referer << std::endl;
		if (config.v_server[server_id].v_location[location_id].location == rq.referer)
		{
			std::cout << "same!!!" << std::endl;
			break;
		}
	}
    return location_id;
}

// int Webserv::set_event(Config &config, Kq kq)
// {
//     int num_of_event;
//     std::vector<Server>::iterator it = config.v_server.begin();
//     for (; it != config.v_server.end(); it++)
//     {
//         int n_changes = kq.change_list.size(); // number of changes = 등록하고자 하는 이벤트 수
//         int n_event_list = 8;
//         if ((num_of_event = kevent(kq.get_kq_fd(), &kq.change_list[0], n_changes, kq.get_event_list(), n_event_list, NULL)) == -1) {
//             std::cerr << "kevent error\n";
//             exit(0);
//         }
//         it->change_list.clear(); // 등록 이벤트 목록 초기화
//     }
//     return num_of_event;
// }