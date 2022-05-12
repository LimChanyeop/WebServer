#include "parseConfig.hpp"

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

int find_key(const std::string &key)
{
	std::vector<std::string> keys;
	keys.push_back("client_limit_body_size");
	keys.push_back("request_limit_header_size");
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
	keys.push_back("server");
	keys.push_back("location");

	for (std::vector<std::string>::iterator it = keys.begin(); it != keys.end(); it++)
	{
		if (*it == key)
			return (it - keys.begin());
	}
	return (-1);
}

void config_parsing(std::vector<std::string> lists, Config_base &config_base) //, Config_base config_base)
{
	std::vector<std::string>::iterator it;
	for (it = lists.begin(); it != lists.end(); it++)
	{
		std::cout << find_key(*it) << std::endl;
		switch (find_key(*it))
		{
		case 0:
			config_base.set_client_limit_body_size(atoi((*(++it)).c_str()));
			break;
		case 1:
			config_base.set_request_limit_header_size(atoi((*(++it)).c_str()));
			break;
		case 2:
			config_base.set_user(*(++it));
			break;
		case 3:
			config_base.set_worker_processes(*(++it));
			break;
		// case 4:
		// 	config_base.set_listen(*(++it));
		// 	break;
		// case 5:
		// 	config_base.set_server_name(*(++it));
		// 	break;
		case 6:
			config_base.set_root(*(++it));
			break;
		case 7:
			config_base.set_index(*(++it));
			break;
		case 8:
			config_base.set_autoindex(*(++it));
			break;
		case 9:
			config_base.set_return_n(*(++it));
			break;
		case 10:
			config_base.set_error_page(*(++it));
			break;
		case 11:
			config_base.set_cgi_info(*(++it));
			break;
		case 12:
			config_base.set_allow_methods(*(++it));
			break;
		case 13:
			config_base.set_auth_key(*(++it));
			break;

		default:
			break;
		}
	}
}
