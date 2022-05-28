#include "./includes/Webserv.hpp"
#include <netinet/in.h>

int main(int argc, char *argv[]) {
	Webserv webserv;
    std::vector<std::string> vec_attr;
    split_config(remove_annotaion(argv[1]), vec_attr);
    Config Config;
    Config.config_parsing(vec_attr);
    // std::cout << "________________________________\n";
    // Config.v_server[1].print_all();
    // std::cout << "________________________________\n";
    // Config.v_server[1].v_location[0].print_all();
    // std::cout << "________________________________\n";
    for (std::vector<Server>::iterator it = Config.v_server.begin(); it != Config.v_server.end(); it++)
    {
        std::cout << it - Config.v_server.begin() << "-";
    	std::cout << it->get_listen() << std::endl;
    }
    Config.print_all();

	webserv.ready_webserv(Config);
	std::map<int, std::string> clients;
    while (1) {
        std::string str_buf;

        std::cout << "waiting for new connection...\n";

        for (std::vector<Server>::iterator it = Config.v_server.begin(); it != Config.v_server.end(); it++)
        {
		    int num_of_event = webserv.set_event(Config);
            for (int i = 0; i < num_of_event; i++) {
                // print_event(it->event_list[i]);
                if (it->event_list[i].filter == EVFILT_READ) {
                    std::cout << "accept READ Event / ident :" << it->event_list[i].ident << std::endl;
                    if (it->event_list[i].ident == it->get_socket_fd()) {
                        int acc_fd;
                        if ((acc_fd = accept(it->get_socket_fd(), (sockaddr *)&it->get_address(),
                                            (socklen_t *)&it->get_address_len())) == -1) //
                        {
                            std::cerr << "accept error " << acc_fd << std::endl;
                            exit(0);
                        }
                        fcntl(acc_fd, F_SETFL, O_NONBLOCK);
                        change_events(it->change_list, acc_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
                        change_events(it->change_list, acc_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
                        clients[acc_fd] = "";
                    } else if (clients.find(it->event_list[i].ident) != clients.end()) {
                        char request[1024] = {0};
                        int valread = read(it->event_list[i].ident, request, 1024);
                        // int valread = recv(acc_socket, request, 1024, 0);
                        str_buf = request;
                        std::cout << str_buf << std::endl;
                    }
                } else if (it->event_list[i].filter == EVFILT_WRITE) {
                    std::cout << "accept WRITE Event / ident :" << it->event_list[i].ident << std::endl;
                    it->response.set_response(i, str_buf);
                    write(it->event_list[i].ident, it->response.get_response().c_str(), it->response.get_response().length());
                    // write(it->event_list[i].ident, "<head><link rel=\"shortcut icon\" href=\"test.ico\"></head>", 54);

                    close(it->event_list[i].ident);
                }
            }
        }
    }
    return 0;
}
