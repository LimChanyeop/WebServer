#ifndef PARSEUTILS_HPP
#define PARSEUTILS_HPP
#include <iostream>
#include <vector>

#include "Base_block.hpp"
#include "Webserv.hpp"

std::string remove_annotaion(char *argv);

int find_key(const std::string &key);

int find_semi(const std::string str);

void split_config(std::string lines, std::vector<std::string> &vec_attr);

#endif