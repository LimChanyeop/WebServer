#ifndef PARSEUTILS_HPP
#define PARSEUTILS_HPP
#include <iostream>
#include <vector>

#include "Config.hpp"
#include "Webserv.hpp"

std::string remove_annotaion(char *argv);

int find_key(const std::string &key);

int find_semi(std::string &str);

void split_config(std::string lines, std::vector<std::string> &vec_attr);

std::string remove_delim(const std::string &str);

#endif