#include "parseUtils.hpp"

void split_config(std::string lines, std::vector<std::string> &vec_attr)
{
    std::string delim = " \t{};";
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