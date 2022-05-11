#include "./utils/parseUtils.hpp"
#include "Server.hpp"

std::string remove_annotaion(char *argv) {
    std::string file_str;
    std::ifstream config_file(argv);
    std::string line;
    while (getline(config_file, line)) {
        size_t anno_pos = ((line.find("#") < line.find("//")) ? line.find("#") : line.find("//"));
        if (anno_pos != std::string::npos) {
            line.erase(line.begin() + anno_pos, line.end());
        }
        file_str += line;
        file_str += "\t";
    }
    config_file.close();
    return file_str;
}