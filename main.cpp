#include "./includes/Client.hpp"
#include "./includes/Fd.hpp"
#include "./includes/Kqueue.hpp"
#include "./includes/Webserv.hpp"
#include <dirent.h>
#include <errno.h>
#include <netinet/in.h>
#define DEFAULT_CONF "./conf/default.conf"
#define DEFAULT_ERR_PAGE "./status_pages/404.html"

int open_default_err_page(std::map<int, Client> clients, int id) {
    int open_fd = open(DEFAULT_ERR_PAGE, O_RDONLY);
    if (open_fd < 0) {
        std::cerr << "open error - " << clients[id].get_route() << std::endl;
        return (-1);
    }
    clients[open_fd].set_read_fd(id); // event_fd:6 -> open_fd:10  발생된10->6
    clients[open_fd].set_status(r_opened);
    return (open_fd);
}

int is_client(Config config, int id) {
    std::vector<Server>::iterator it = config.v_server.begin();
    for (; it != config.v_server.end(); it++) {
        if (it->get_socket_fd() == id)
            return 0;
    }
    return 1;
}

int find_server(Config Config, Client &client, int id) {
    std::vector<Server>::iterator it;
    for (it = Config.v_server.begin(); it != Config.v_server.end(); it++) {
        std::cout << "listen:" << it->get_socket_fd() << " vs " << id << std::endl;
        if (it->get_socket_fd() == id) {
            std::cout << "It is server!!\n";
            client.set_server_id(it - Config.v_server.begin());
            client.set_server_sock(id);
            return 1; // 드디어 어떤 서버인지 찾음
        }
    }
    if (it == Config.v_server.end()) {
        // std::cerr << "Can not found Server\n";
        return 0;
    }
    return 0;
}

int main(int argc, char *argv[]) {
    Webserv webserv;
    Config Config;
    Kqueue kq;
    std::map<int, Client> clients;
    std::vector<std::string> vec_attr;

    if (argc == 2)
        split_config(remove_annotaion(argv[1]), vec_attr);
    else if (argc == 1)
        split_config(remove_annotaion(const_cast<char *>(DEFAULT_CONF)), vec_attr);
    else {
        std::cerr << "input error argc-(" << argc << ")\n";
        exit(0);
    }
    Config.config_parsing(vec_attr);

    webserv.ready_webserv(Config);
    kq.kq = kqueue();
    for (std::vector<Server>::iterator it = Config.v_server.begin(); it != Config.v_server.end(); it++) {
        change_events(kq.change_list, it->get_socket_fd(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    }
    while (1) {
        std::string str_buf;

        int num_of_event = kq.set_event();
        for (int i = 0; i < num_of_event; i++) {
            int id = kq.event_list[i].ident;
            // std::cout << "event id:" << id << \
			// 	" , event filter:" << kq.event_list[i].filter << \
			// 	" , status:" << clients[id].get_status() << std::endl;
            if (kq.event_list[i].flags & EV_ERROR) {

                if (is_client(Config, id)) {
                    if (clients[id].pid == -1) {
                        if (clients[id].request.requests.size() > 0)
                            close(clients[id].read_fd); // fclose by jwoo
                        if (clients[id].response.response_str.length() > 0)
                            close(clients[id].write_fd);
                    }
                }
                close(kq.event_list[i].ident);
                continue;
            } else if (kq.event_list[i].filter == EVFILT_READ) {
                // std::cout << "accept READ Event / ident :" << id << std::endl;
                if (clients[id].get_status() == r_opened) {
                    int seek = 0;
                    int file_size;
                    char buf[1024];
                    int read_fd = clients[id].get_read_fd();
                    // std::cout << "FILE READ, id:" << id << " ,read_id:" << clients[id].read_fd << "\n"; // open->read->write fopen->fread->fwrite
                    FILE *file_ptr = fdopen(id, "r"); /////////////
                    fseek(file_ptr, 0, SEEK_END);
                    file_size = ftell(file_ptr);
                    rewind(file_ptr); ////////////// 뭔가 잘못됬던듯..?
                    memset(buf, 0, 1024);
                    while (seek < file_size) {
                        seek += fread(buf, sizeof(char), 1023, file_ptr);
                        clients[read_fd].response.response_str += buf;
                    }
                    clients[read_fd].set_status(response_ok);
                    // std::cout << "READ_ok\n";
                    clients.erase(id);
                    close(id);
                } else if (clients[id].get_status() == r_cgi_opened) // 이벤트 주체가 READ open // file read->fread
                {
                    int status;
                    int ret = waitpid(clients[id].pid, &status, WNOHANG);
                    if (ret == 0)
                        continue;
                    else if (WIFSIGNALED(status) == true) {
                        // cgi error
                        clients.erase(id);
                        close(id);
                        continue;
                    } else if (WIFEXITED(status) != true) {
                        continue;
                    }

                    // std::cout << "FILE READ, id:" << id << " ,read_id:" << clients[id].read_fd << "\n"; // open->read->write fopen->fread->fwrite
                    int read_fd = clients[id].get_read_fd(); //
                    int seek = 0;
                    int valfread = 0;
                    char buf[1024];
                    memset(buf, 0, 1024);
                    while ((seek = read(id, buf, 1023)) > 0) { // read
                        clients[read_fd].response.response_str += buf;
                    }
                    std::string header;
                    std::string temp = clients[read_fd].response.response_str;
                    int find;
                    if ((find = temp.find("X-Powered-By:")) != std::string::npos) // if cgi -> header parsing
                    {
                        if ((find = temp.find("<")) != std::string::npos) {
                            header = temp.erase(find - 1, temp.end() - temp.begin());
                            clients[read_fd].response.response_str = clients[read_fd].response.response_str.erase(0, find);
                        }
                    }
                    // std::cout << "header:" << header << std::endl;
                    // std::cout << "**clients[read_fd].response.response_str:" << clients[read_fd].response.response_str << "\n\n";
                    clients[clients[id].read_fd].header = header;

                    clients[read_fd].set_status(cgi_response_ok);
                    // std::cout << "cgi_READ_ok\n";
                    close(id);
                    clients.erase(id);
                } else if (find_server(Config, clients[id], id)) // 이벤트 주체가 server
                {
                    std::cout << "clients[" << id << "].get_server_id():" << clients[id].get_server_id() << std::endl;
                    webserv.accept_add_events(id, Config.v_server[clients[id].get_server_id()], kq, clients);
                } else if (clients.find(id) != clients.end()) // 이벤트 주체가 client
                {
                    clients[id].request_parsing(id);
                    // std::cout << "PORT:" << clients[id].request.get_host() << std::endl;
                    int server_id = webserv.find_server_id(id, Config, clients[id].request, clients);
                    std::cout << "server_id:" << clients[id].get_server_id() << ", req host:" << clients[id].request.get_host() << std::endl;
                    if (clients[id].request.get_method() == "GET") {
                        int location_id = webserv.find_location_id(server_id, Config, clients[id].request, clients[id]); // /abc가 있는가?
                        // std::cout << server_id << " " << location_id << std::endl;
                        if (location_id == 404) // is not found
                        {
                            std::cout << "404\n";
                            int open_fd = open_default_err_page(clients, id);
                            change_events(kq.change_list, open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // read event 추가
                            break;
                        } else if (location_id == -1) // is file
                        {
                            std::cout << "is file\n";
                            std::string referer = clients[id].request.get_referer();
                            if (*referer.begin() == '/')
                                referer.erase(referer.begin(), referer.begin() + 1);
                            std::string root = Config.v_server[server_id].v_location[location_id].get_root();
                            if (*(root.end() - 1) == '/')
                                root.erase(root.end() - 1, root.end());
                            clients[id].set_route("." + root + "/" + referer);
                            std::cout << "route-" << clients[id].get_route() << std::endl;

                            int open_fd = open(clients[id].get_route().c_str(), O_RDONLY);
                            if (open_fd < 0) {
                                open_fd = open_default_err_page(clients, id);
                                change_events(kq.change_list, open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // read event 추가
                                break;
                            }
                            std::cout << "isfile-my fd::" << id << ", open fd::" << open_fd << std::endl;
                            clients[open_fd].set_read_fd(id); // event_fd:6 -> open_fd:10  발생된10->6
                            clients[open_fd].set_status(r_opened);

                            clients[id].RETURN = 200;
                            change_events(kq.change_list, open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // read event 추가
                            break;
                        }
                        clients[id].set_location_id(location_id);
                        std::string index = Config.v_server[server_id].v_location[location_id].get_index();
                        std::cout << "there is index?" << index << std::endl;
                        if (Config.v_server[server_id].v_location[location_id].get_index() != "" &&
                            (index.find("php") == std::string::npos && index.find("py") == std::string::npos)) {
                            // if (autoindex)
                            std::string index = Config.v_server[server_id].v_location[location_id].get_index();
                            std::string root = Config.v_server[server_id].v_location[location_id].get_root();
                            if (*(root.end() - 1) != '/')
                                root += '/';
                            clients[id].set_route("." + root + index);
                            //											/View + / + Default.html
                            // std::cout << "route: " << clients[id].get_route() << std::endl;

                            int open_fd = open(clients[id].get_route().c_str(), O_RDONLY);
                            if (open_fd < 0) {
                                open_fd = open_default_err_page(clients, id);
                                change_events(kq.change_list, open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // read event 추가
                                break;
                            }

                            std::cout << "fd::" << id << ", open fd::" << open_fd << std::endl;
                            clients[open_fd].set_read_fd(id); // event_fd:6 -> open_fd:10  발생된10->6
                            clients[open_fd].set_status(r_opened);
                            change_events(kq.change_list, open_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // read event 추가
                        } else if ((index.find("php") == std::string::npos && index.find("py") == std::string::npos)) {
                            clients[id].set_status(complete);
                            clients[id].RETURN = 200;
                        } else /////////////////// cgi
                        {
                            std::cout << "im cgi!!\n";
                            std::string root = '.' + Config.v_server[server_id].v_location[location_id].get_root();
                            if (*(root.end() - 1) != '/')
                                root += '/';
                            std::string index_root = root + Config.v_server[server_id].v_location[location_id].get_index();
                            std::cout << "index_root: " << index_root << std::endl;
                            std::cout << "cgi-file: " << Config.v_server[server_id].get_cgi_path() << std::endl;
                            webserv.run_cgi(Config.v_server[server_id], index_root, clients[id]); // envp have to fix
                            close(clients[id].write_fd);
                            clients[clients[id].read_fd].set_read_fd(id);
                            clients[clients[id].read_fd].set_status(r_cgi_opened);
                            clients[clients[id].read_fd].pid = clients[id].pid;
                            std::cout << "clients[clients[" << id << "].read_fd].get_read_fd() :" << clients[clients[id].read_fd].get_read_fd()
                                      << std::endl;
                            std::cout << "read_fd : " << clients[id].read_fd << std::endl;
                            change_events(kq.change_list, clients[id].read_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
                        }
                    } else if (clients[id].request.get_method() == "POST") {
                        int is_dir = webserv.is_dir(Config.v_server[server_id], clients[id].request, clients[id]);
                        if (is_dir == 1) // is dir
                        {
                            std::cout << "ISDIR\n";
                            clients[id].RETURN = 201;
                            std::string referer = clients[id].request.get_referer();
                            if (*(referer.end() - 1) != '/')
                                referer += '/';
                            std::string route = "." + clients[id].request.get_referer() + "NEW_FILE";
                            FILE *file_ptr;
                            int i = 0;
                            while ((file_ptr = fopen((route + std::to_string(i)).c_str(), "r"))) {
                                fclose(file_ptr);
                                i++;
                            }
                            int open_fd = open((route + std::to_string(i)).c_str(), O_RDWR | O_CREAT | O_APPEND | O_SYNC, S_IWUSR | S_IRUSR);
                            if (open_fd < 0)
                                std::cerr << "open error - " << route << std::endl;
                            std::cout << "POST-my fd::" << id << ", open fd::" << open_fd << std::endl;
                            clients[open_fd].set_status(w_opened);
                            clients[open_fd].write_fd = id;
                            if (clients[id].request.get_referer().find("php") != std::string::npos ||
                                clients[id].request.get_referer().find("py") != std::string::npos) {
                                // cgi
                                std::string index_root = route + std::to_string(i);
                                webserv.run_cgi(Config.v_server[server_id], index_root, clients[id]);
                                clients[clients[id].read_fd].pid = clients[id].pid;
                                clients[clients[id].read_fd].set_read_fd(id);
                                clients[clients[id].read_fd].set_status(w_opened);
                                std::cout << "clients[clients[" << id << "].read_fd].get_read_fd() :" << clients[clients[id].read_fd].get_read_fd()
                                          << std::endl;
                                std::cout << "read_fd : " << clients[id].read_fd << std::endl;
                                change_events(kq.change_list, clients[id].read_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL); // cgi result 읽기
                                change_events(kq.change_list, clients[id].write_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0,
                                              NULL); // cgi에 post_body 쓰기
                            } else
                                clients[open_fd].request.post_body = clients[id].request.post_body;

                            change_events(kq.change_list, open_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL); // write event 추가
                        } else {
                            std::cout << "not dir\n";
                            std::string route = "." + clients[id].request.get_referer();
                            int open_fd;
                            if (clients[id].RETURN == 200)
                                open_fd = open(route.c_str(), O_RDWR | O_APPEND | O_SYNC, S_IWUSR | S_IRUSR);
                            else
                                open_fd = open(route.c_str(), O_RDWR | O_CREAT | O_APPEND | O_SYNC, S_IWUSR | S_IRUSR);
                            if (open_fd < 0)
                                std::cerr << "open error - " << route << std::endl;
                            std::cout << "POST-my fd!!" << id << ", open fd!!" << open_fd << std::endl;
                            clients[open_fd].set_status(w_opened);
                            clients[open_fd].write_fd = id;
                            clients[open_fd].request.post_body = clients[id].request.post_body;

                            std::cout << "event add ok~\n";
                            // fcntl(open_fd, F_SETFL, O_NONBLOCK);
                            change_events(kq.change_list, open_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL); // write event 추가
                        }
                    } // end POST
                }
            }
            if (clients[id].get_server_id() < -1)
                continue;
            if (kq.event_list[i].filter == EVFILT_WRITE) {
                if (clients[id].get_status() == w_opened || clients[clients[id].read_fd].get_status() == w_opened) {
                    FILE *fp;
                    if (clients[id].get_status() == w_opened) {
                        fp = fdopen(id, "w");
                    } else {
                        fp = fdopen(clients[clients[id].read_fd].get_read_fd(), "w");
                    }
                    if (fp == NULL) {
                        std::cout << "fdopen error" << std::endl;
                        continue;
                    }
                    fwrite(clients[id].request.post_body.c_str(), 1, clients[id].request.post_body.length(), fp); // POST write
                    std::cout << "write-" << id << ":" << clients[id].request.post_body << std::endl;
                    clients[clients[id].get_write_fd()].set_status(response_ok);
                    close(id);
                    fclose(fp);
                    clients.erase(id);
                    break;
                }
                if (clients[id].get_status() >= response_ok) {
                    std::cout << "accept WRITE Event / ident :" << id << std::endl;
                    if (Config.v_server[clients[id].get_server_id()].get_autoindex() == "on") // location on?
                    {
                        std::string root;
                        if (Config.v_server[clients[id].get_server_id()].get_autoindex() != "")
                            root = Config.v_server[clients[id].get_server_id()].get_autoindex();
                        if (Config.v_server[clients[id].get_server_id()].v_location[clients[id].get_location_id()].get_autoindex() != "")
                            root = Config.v_server[clients[id].get_server_id()].v_location[clients[id].get_location_id()].get_autoindex();
                        std::vector<Location>::iterator it = Config.v_server[clients[id].get_server_id()].v_location.begin();
                        for (; it != Config.v_server[clients[id].get_server_id()].v_location.end(); it++) ////// why root no?
                        {
                            clients[id].response.set_autoindex(it->location);
                        }
                    }
                    clients[id].response.set_header(clients[id].RETURN, "");
                    if (clients[id].get_status() == response_ok) {
                        FILE *fp = fdopen(id, "wb");
                        if (fp == NULL) {
                            std::cout << "fdopen error" << std::endl;
                            continue;
                        }
                        std::cerr << "response :: " << clients[id].response.get_send_to_response().c_str() << std::endl;
                        int count = 0;
                        while (count < clients[id].response.get_send_to_response().length() * sizeof(char)) {
                            count += fwrite(clients[id].response.get_send_to_response().c_str(), sizeof(char),
                                            clients[id].response.get_send_to_response().size(), fp);
                        }
                        fclose(fp);
                    } else if (clients[id].get_status() >= cgi_response_ok) {
                        int count = 0;
                        std::cerr << "response :: " << clients[id].response.get_send_to_response().c_str() << std::endl;
                        while (count < clients[id].response.get_send_to_response().length() * sizeof(char)) {
                            count += write(id, clients[id].request.post_body.c_str(), clients[id].request.post_body.length());
                        }
                    }
                    std::cout << "return:" << clients[id].RETURN << std::endl;
                    clients.erase(id);
                    close(id);
                }
            }
        }
    }
    exit(0);
    return 0;
}