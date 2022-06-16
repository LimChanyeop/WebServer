// #include <sys/types.h>
// #include <sys/stat.h>

#include "../includes/Webserv.hpp"
#include "../includes/Fd.hpp"
#include "../includes/ParseUtils.hpp"
#include "../includes/Request.hpp"

#include <dirent.h>

Webserv::Webserv(/* args */) {}

Webserv::~Webserv() {}

void change_events(std::vector<struct kevent> &change_list, uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data,
                   void *udata) // 이벤트를 생성하고 이벤트 목록에 추가하는 함수
{
    struct kevent temp_event;

    EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
    change_list.push_back(temp_event);
}

std::string &Webserv::mime_read(std::string &default_mime) {
    int open_fd;
    open_fd = open(default_mime.c_str(), O_RDONLY);
    if (open_fd < 0) {
        std::cerr << "mime open error!!\n";
        exit(-1);
    }
    char buff[1024];
    std::string read_str;
    int read_val = 0;
    memset(buff, 0, 1024);
    while ((read_val = read(open_fd, buff, 1023)) > 0) {
        buff[read_val] = 0;
        read_str += buff;
    }
    if (read_val < 0) {
        std::cerr << "mime read error!!\n";
        exit(-1);
    }
    default_mime = read_str;
    return default_mime;
}

void Webserv::mime_parsing(std::string &default_mime) {
    std::vector<std::string> split_mime;
    std::string mime_str;
    std::string::iterator it = default_mime.begin();
    while (it != default_mime.end()) {
        if (*it == '\n') {
            split_mime.push_back(mime_str);
            mime_str = "";
            it++;
            continue;
        }
        mime_str += *it;
        it++;
    }
    std::vector<std::string>::iterator v_it2 = split_mime.begin(); // print
    std::cerr << "*mime split (v_it)*\n";
    while (v_it2 != split_mime.end()) {
        std::cerr << *v_it2 << std::endl;
        v_it2++;
    }
    std::vector<std::string>::iterator v_it;
    std::string mime_str2;
    for (v_it = split_mime.begin(); v_it != split_mime.end(); v_it++) {
        it = (*v_it).begin();
        while (it != (*v_it).end() && *it != ' ') // str1
        {
            mime_str += *it;
            it++;
        }
        while (it != (*v_it).end() && *it == ' ') // ' '
            it++;
        while (it != (*v_it).end()) // str2
        {
            mime_str2 += *it;
            it++;
        }
        this->mimes[mime_str2] = mime_str;
        std::cerr << mime_str2 << " = " << mime_str << std::endl;
        mime_str.clear();
        mime_str2.clear();
    }
}

void Webserv::ready_webserv(Config &Config) {
    std::vector<Server>::iterator it = Config.v_server.begin();
    for (; it != Config.v_server.end(); it++) {
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
        // struct timespec timeout;
        // timeout.tv_sec = 10; // 초 (seconds)
        // timeout.tv_nsec = 0;
        // setsockopt(it->get_socket_fd(), SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(struct timeval));
        // setsockopt(it->get_socket_fd(), SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(struct timeval));

        struct timeval tv;
        tv.tv_sec = 60;
        tv.tv_usec = 0;
        setsockopt(it->get_socket_fd(), SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tv, sizeof(struct timeval));
        setsockopt(it->get_socket_fd(), SOL_SOCKET, SO_SNDTIMEO, (struct timeval *)&tv, sizeof(struct timeval));

        setsockopt(it->get_socket_fd(), SOL_SOCKET, SO_REUSEADDR, &optvalue,
                   sizeof(optvalue)); // to solve bind error
        setsockopt(it->get_socket_fd(), SOL_SOCKET, SO_REUSEPORT, &optvalue,
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

std::vector<Server>::iterator Webserv::find_server_it(Config &Config, Client &client) {
    std::vector<Server>::iterator it;
    for (it = Config.v_server.begin(); it != Config.v_server.end(); it++) {
        // std::cout << "listen:" << it->get_socket_fd() << " vs " << client.get_server_sock() << std::endl;
        if (it->get_socket_fd() == client.get_server_sock()) {
            // client.set_server_it(it);
            return it; // 드디어 어떤 서버인지 찾음
        }
    }
    if (it == Config.v_server.end()) {
        std::cerr << "Can not found Server\n";
        exit(-1);
    }
    return it;
}

int Webserv::find_server_id(const int &event_ident, const Config &config, const Request &rq, std::map<int, Client> &clients) {
    std::string port = ""; // 왜 포트를 못찾았을까? -> 포트를 파싱 안했었넹~ok -> 근데도 못찾네~
    int server_id;
    // std::cout << "*find*" << clients.find(event_ident)->second << std::endl;
    if (clients.find(event_ident) != clients.end()) {
        server_id = 0;
        while (server_id < config.v_server.size()) {
            if (atoi(config.v_server[server_id].get_listen().c_str()) == atoi(rq.get_host().c_str())) // 못찾는게,, 이게 다르데;;
            {
                port = rq.get_host(); // ?
                clients[event_ident].set_server_id(server_id);
                return server_id;
            }
            server_id++;
        }
        if (server_id == config.v_server.size()) {
            std::cerr << "Not matched server. check the request.\n";
        }
    }
    return -1;
}

int Webserv::is_dir(const Server &server, const Request &rq, Client &client) {
    std::string referer = rq.get_referer();
    if (*referer.begin() == '/')
        referer.erase(referer.begin(), referer.begin() + 1);
    client.set_route(server.get_root() + referer);
    std::string route = "." + client.get_route();
    if (rq.get_method() == "POST") {
        // is dir?
        DIR *dir_ptr = NULL;
        struct dirent *file = NULL;
        if ((dir_ptr = opendir(route.c_str())) != NULL) {
            std::cerr << "It is directory, 201\n";
            client.RETURN = 201;
            closedir(dir_ptr);
            return 1;
        }
    }
    FILE *file_ptr;
    if ((file_ptr = fopen(route.c_str(), "r"))) { // wb
        std::cout << "file exits!\n";             // exist
        client.is_file = 1;
        client.RETURN = 200;
        fclose(file_ptr);
        return 0;
    } else {
        std::cout << "file not exits!\n";
        client.RETURN = 201; // created
        return 0;
    }
    return -1;
}

int Webserv::find_location_id(const int &server_id, const Config &config, const Request &rq, Client &client) {
    int location_id;
    std::cout << "webserv::server_id : " << server_id << std::endl;
    // std::cout << "Webserv::" << config.v_server[server_id].v_location.size() << std::endl;
    if (rq.get_method() == "GET") {
        for (location_id = 0; location_id < config.v_server[server_id].v_location.size(); location_id++) {
            // std::cout << "Webserv::TEST-location:" << config.v_server[server_id].v_location[location_id].location << "vs" << rq.referer <<
            // std::endl;
            if (config.v_server[server_id].v_location[location_id].location == rq.get_referer()) {
                return location_id;
            }
        }
    }
    std::string referer = rq.get_referer();
    if (*referer.begin() == '/')
        referer.erase(referer.begin(), referer.begin() + 1);
    client.set_route(config.v_server[server_id].get_root() + referer);
    std::string route = "." + config.v_server[server_id].get_root() + referer;
    std::cout << "webserv::route-" << route << std::endl;
    // is dir?
    DIR *dir_ptr = NULL;
    struct dirent *ent = NULL;
    if ((dir_ptr = opendir(route.c_str())) != NULL) {
        std::cout << "It is directory, 201\n";
        client.RETURN = 200;
        closedir(dir_ptr);
        return -1;
    }
    FILE *file;                                       // file exist?
    if ((file = fopen(route.c_str(), "r")) != NULL) { // exist
        fclose(file);
        client.is_file = 1;
        client.RETURN = 200;
        return -2;
    }
    return 404; // 404에러
}

void Webserv::accept_add_events(const int &event_ident, Server &server, Kqueue &kq, std::map<int, Client> &clients) {
    // std::cout << "client_id:" << event_ident << " vs server_id:" << server_it->get_socket_fd() << std::endl;
    int acc_fd;
    if ((acc_fd = accept(server.get_socket_fd(), (sockaddr *)&(server.get_address()),
                         (socklen_t *)&(server.get_address_len()))) == -1) //
    {
        std::cerr << "accept error " << acc_fd << std::endl;
        exit(0);
    }
    std::cout << "acc_fd: " << acc_fd << std::endl;
    fcntl(acc_fd, F_SETFL, O_NONBLOCK);
    change_events(kq.change_list, acc_fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    change_events(kq.change_list, acc_fd, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, NULL);
    // std::cout << "hi\n";
    inet_ntop(AF_INET, (sockaddr *)&(server.get_address()).sin_addr, clients[acc_fd].ip, INET_ADDRSTRLEN);
    clients[acc_fd].set_server_sock(event_ident);
    clients[acc_fd].set_status(server_READ_ok);
    // std::cout << "hi2\n";
}
char **make_env(Client &client, const Server &server) {
    std::map<std::string, std::string> cgi_map;
    // std::string extension = client.getRequest()->getPath().substr(client->getRequest()->getPath().rfind(".") + 1);
    cgi_map["SERVER_PROTOCOL"] = "HTTP/1.1";
    cgi_map["GATEWAY_INTERFACE"] = "CGI/1.1";
    cgi_map["SERVER_SOFTWARE"] = "nginx server";
    cgi_map["REQUEST_METHOD"] = client.request.get_method();
    cgi_map["REQUEST_SCHEME"] = client.request.get_method();
    cgi_map["SERVER_PORT"] = client.request.get_host();
    cgi_map["SERVER_NAME"] = "localhost";
    cgi_map["DOCUMENT_ROOT"] = server.get_cgi_path();           //"./cgiBinary/php-cgi"; //
    cgi_map["DOCUMENT_URI"] = client.get_route();               //"/View/file.php"; // 리퀘스트에 명시된 전체 주소가 들어가야 함 //
    cgi_map["REQUEST_URI"] = client.get_route();                // "/View/file.php";	// 리퀘스트에 명시된 전체 주소가 들어가야 함 //
    cgi_map["SCRIPT_NAME"] = client.get_route();                // "/View/file.php";	// 실행파일 전체 주소가 들어가야함 //
    cgi_map["SCRIPT_FILENAME"] = '.' + client.get_route();      //"./View/file.php";
    cgi_map["QUERY_STRING"] = client.request.get_query();       //
    cgi_map["REMOTE_ADDR"] = client.ip;                         //
    cgi_map["REDIRECT_STATUS"] = std::to_string(client.RETURN); // 200
    if (client.request.get_method() == "POST")
        cgi_map["CONTENT_LENGTH"] = client.request.get_contentLength(); // GET은 노노
    cgi_map["CONTENT_TYPE"] = client.request.get_contentType();

    char **cgi_env;
    cgi_env = new char *[cgi_map.size() + 1];

    size_t i = 0;
    for (std::map<std::string, std::string>::iterator it = cgi_map.begin(); it != cgi_map.end(); it++) {

        // cgi_env[i] = new char[(it->first + "=" + it->second).size()];
        cgi_env[i] = strdup((it->first + "=" + it->second).c_str());
        std::cout << "cgi: " << cgi_env[i] << std::endl;
        i++;
    }
    cgi_env[i] = NULL;
    return cgi_env;
}

void Webserv::run_cgi(const Server &server, const std::string &index_root, Client &client) {
    char buff[1024] = {0};
    int read_fd[2];
    int write_fd[2];
    char **cgi_env;

    if (pipe(read_fd) == -1) {
        std::cerr << "pipe error\n";
        exit(-1);
    }
    pipe(write_fd);

    // std::cout << "ar[0]" << server.get_cgi_path().c_str() << std::endl;
    // std::cout << "ar[1]" << server.v_location[location_id].get_root() + "/" + server.v_location[location_id].get_index() << std::endl;
    cgi_env = make_env(client, server);
    int pid = fork();
    if (pid == -1) {
        std::cerr << "fork error\n";
        exit(-1);
    } else if (pid == 0) // child
    {
        // ------------------
        char *ar[3];
        ar[0] = const_cast<char *>((server.get_cgi_path()).c_str()); // ("./cgiBinary/php-cgi"); //"./cgiBinary/php-cgi"); // /bin/cat
        ar[1] = const_cast<char *>(index_root.c_str());              // php file (./file)
        ar[2] = 0;
        // if (qury)
        // 	ar[2] = qury

        // cgi_env->client.request_referer->url(parsing), client.request.get_ ~~
        // ------------------
        // ar[0] = strdup("/Users/minsikim/Desktop/42seoul/B2C/WebServer/View/CGI.drawio");
        // ar[1] = strdup("/Users/minsikim/Desktop/42seoul/B2C/WebServer/View/CGI.png");
        dup2(write_fd[0], STDIN_FILENO);
        dup2(read_fd[1], STDOUT_FILENO);
        close(write_fd[1]);
        close(read_fd[0]);
        int ret = execve(ar[0], ar, cgi_env); // "/bin/cat"
        if (ret == -1) {
            std::cerr << "***************\nexecve not run!\n***************\n" << std::endl;
            exit(-1);
        }
        exit(0);
    }
    delete cgi_env;
    close(write_fd[0]);
    close(read_fd[1]);

    client.pid = pid;
    client.read_fd = read_fd[0];
    client.write_fd = write_fd[1];
}
