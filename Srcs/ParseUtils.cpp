#include "../includes/ParseUtils.hpp"

void error_exit(std::string error)
{
	std::cerr << error << " error!\n";
	exit(-1);
}

void split_config(std::string lines, std::vector<std::string> &vec_attr)
{
	std::string delim = " \t";
	std::string::iterator it;
	std::string attr = "";
	for (it = lines.begin(); it != lines.end(); it++)
	{
		if (delim.find(*it) == std::string::npos)
		{
			attr += *it;
		}
		else
		{
			if (attr != "")
			{
				vec_attr.push_back(attr);
				attr.clear();
			}
		}
	}
	if (attr.length() > 0)
	{
		vec_attr.push_back(attr);
	}
}

std::string remove_annotaion(char *argv)
{
	std::string file_str;
	std::ifstream config_file(argv);
	std::string line;
	if (config_file.is_open() < 1)
	{
		error_exit("is open");
	}
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

int find_semi(std::string &str)
{
	if (*(str.end() - 1) == ';')
	{
		str.erase(str.end() - 1);
		return 0;
	}
	else
		return 1;
}

int find_key(const std::string &key)
{
	std::vector<std::string> keys;
	keys.push_back("client_limit_body_size"); // 0
	keys.push_back("request_limit_header_size");
	keys.push_back("user");
	keys.push_back("worker_processes");
	keys.push_back("listen");
	keys.push_back("server_name");
	keys.push_back("root");
	keys.push_back("index");
	keys.push_back("autoindex");
	keys.push_back("return");
	keys.push_back("error_page"); // 10
	keys.push_back("cgi_path");
	keys.push_back("allow_nethods");
	keys.push_back("auth_key");
	keys.push_back("server");
	keys.push_back("location");
	keys.push_back("limit_except");

	for (std::vector<std::string>::iterator it = keys.begin(); it != keys.end(); it++)
	{
		if (*it == key)
			return (it - keys.begin());
	}
	return (-1);
}

std::string remove_delim(const std::string &str)
{
	std::string temp = str;
	std::string::iterator it = temp.begin();
	while (*it == ' ')
		it++;
	temp.erase(temp.begin(), it);
	// if (temp.end() - 1 == ';')
	// 	temp.erase(temp.end() - 1);
	return temp;
}