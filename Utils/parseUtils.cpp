#include <string>

int parseUrlQuery(std::string query, std::string key, std::string &value) {
    std::string::size_type pos = query.find(key);
    if (pos == std::string::npos)
        return -1;
    pos += key.length();
    std::string::size_type pos2 = query.find("&", pos);
    if (pos2 == std::string::npos)
        pos2 = query.length();
    value = query.substr(pos, pos2 - pos);
    return 0;
}