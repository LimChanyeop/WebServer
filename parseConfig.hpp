#ifndef PARSECONFIG_HPP
#define PARSECONFIG_HPP

#include "Server.hpp"
#include "./utils/parseUtils.hpp"

std::string remove_annotaion(char *argv);

int find_key(const std::string &key);

void config_parsing(std::vector<std::string> lists, Config_base &config_base); //, Config_base config_base)

#endif