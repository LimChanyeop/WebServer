#include "./utils/parseUtils.hpp"
#include "Server.hpp"

std::string remove_annotaion(char *argv)
{
	std::string file_str;
	std::ifstream config_file(argv);
	std::string line;
	if (config_file.is_open() < 1)
		std::cerr << "not opened\n";
	while (getline(config_file, line))
	{
		size_t anno_pos = ((line.find("#") < line.find("//")) ? line.find("#") : line.find("//"));
		if (anno_pos != std::string::npos)
		{
			line.erase(line.begin() + anno_pos, line.end());
		}
		file_str += line;
		file_str += "\t";
	}
	config_file.close();
	return file_str;
}

int find_key(std::string key)
{
	std::vector<std::string> keys;
	keys.push_back("server");
	keys.push_back("location");
	keys.push_back("user");
	keys.push_back("worker_processes");
	keys.push_back("listen");
	keys.push_back("server_name");
	keys.push_back("root");
	keys.push_back("index");
	keys.push_back("autoindex");
	keys.push_back("return_n");
	keys.push_back("error_page");
	keys.push_back("chi_info");
	keys.push_back("allow_nethods");
	keys.push_back("auth_key");

	for (std::vector<std::string>::iterator it = keys.begin(); it != keys.end(); it++)
	{
		if (*it == key)
			return (it - keys.begin());
	}
	return (-1);
}

void config_parsing(std::vector<std::string> lists) //, Config_base config_base)
{
	std::vector<std::string>::iterator it;
	for (it = lists.begin(); it != lists.end(); it++)
	{
		// if (*if == "user")
		// {
		//     config_base.set_user(++(*it));
		// }
		std::cout << find_key(*it) << std::endl;
	}
}
