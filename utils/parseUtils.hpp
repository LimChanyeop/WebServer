#ifndef PARSEUTILS_HPP
#define PARSEUTILS_HPP
#include <iostream>
#include <vector>

#include "../Server.hpp"
#include "../Base_block.hpp"

std::string remove_annotaion(char *argv);

int find_key(const std::string &key);

void config_parsing(std::vector<std::string> lists, Base_block &config_base); //, Config_base config_base)

void split_config(std::string lines, std::vector<std::string> &vec_attr);

#endif