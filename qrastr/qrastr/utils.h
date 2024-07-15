#ifndef UTILS_H
#define UTILS_H

#include <sstream>      // std::istringstream
#include <iostream>     // std::cerr
#include <string>
#include <vector>
inline std::vector<std::string> split(const std::string& s, char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter))
    {
        tokens.push_back(token);
    }
    return tokens;
}
#endif // UTILS_H
