#include "Server.hpp"

void print_event(struct kevent event_list) {
    std::cout << "ident : " << event_list.ident << std::endl
              << "filter : " << std::bitset<16>(event_list.filter) << std::endl // 모든 bit = 1 출력
              << "flags : " << std::bitset<16>(event_list.flags) << std::endl
              << "fflags : " << std::bitset<32>(event_list.fflags) << std::endl
              << "data : " << std::bitset<16>(event_list.data) << std::endl
              << "udata : " << event_list.udata << std::endl;
}
