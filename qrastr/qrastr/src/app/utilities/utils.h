#ifndef UTILS_H
#define UTILS_H
#pragma once

#include <sstream>      // std::istringstream
#include <iostream>     // std::cerr
#include <string>
#include <vector>
#include <map>
#include <regex>
#include <chrono>

template<typename T> class KeyValueRange { //from https://stackoverflow.com/questions/8517853/iterating-over-a-qmap-with-for/77994379#77994379
private:
    T iterable; // This is either a reference or a moved-in value. The map data isn't copied.
public:
    KeyValueRange(T &iterable) : iterable(iterable) { }
    KeyValueRange(std::remove_reference_t<T> &&iterable) noexcept : iterable(std::move(iterable)) { }
    auto begin() const { return iterable.keyValueBegin(); }
    auto end() const { return iterable.keyValueEnd(); }
};

template <typename T> auto asKeyValueRange(T &iterable) { return KeyValueRange<T &>(iterable); }
template <typename T> auto asKeyValueRange(const T &iterable) { return KeyValueRange<const T &>(iterable); }
template <typename T> auto asKeyValueRange(T &&iterable) noexcept { return KeyValueRange<T>(std::move(iterable)); }


template<typename C, typename E>
bool contains(const C& container, E element)
{
    return std::find(container.begin(), container.end(), element) != container.end();
}

template<typename T1, typename T2, typename E>
bool contains(const std::map<T1, T2>& container, E element)
{
    return container.find(element) != container.end();
}

template<typename C, typename E>
int find_index(const C& container, E element)
{
    int index = 0;
    for (auto it = container.begin(); it != container.end(); it++)
    {
        if ((*it) == element)
            return index;
        index++;
    }
    return -1;
}

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

inline std::vector<std::string> split2(std::string& s, const std::string& delimiter) {

    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        tokens.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    tokens.push_back(s);

    return tokens;
}

inline std::vector<std::string> split3(const std::string str,
                                const std::string regex_str) {
    std::regex regexz(regex_str);
    return { std::sregex_token_iterator(str.begin(), str.end(), regexz, -1),
            std::sregex_token_iterator() };
}

inline void replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

inline void replaceAll(std::string& str, const std::string& from, const std::vector<int>& to) {
    if (from.empty())
        return;
    size_t start_pos = 0;
    size_t ind(0);
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), std::to_string(to[ind]));
        start_pos += std::to_string(to[ind]).length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
        ind++;
    }
}

inline std::string trim(std::string& str)
{
    str.erase(0, str.find_first_not_of(' '));       //prefixing spaces
    str.erase(str.find_last_not_of(' ') + 1);         //surfixing spaces
    return str;
}

template <class DT = std::chrono::milliseconds,
         class ClockT = std::chrono::steady_clock>
class Timer
{
    using timep_t = typename ClockT::time_point;
    timep_t _start = ClockT::now(), _end = {};

public:
    Timer()
    {
        tick();
    }
    template <class T = DT>
    auto duration() const {
        tock();
        //gsl_Expects(_end != timep_t{} && "toc before reporting");
        return std::chrono::duration_cast<T>(_end - _start);
    }
    double seconds()
    {
        tock();
        double dur =  std::chrono::duration_cast<DT>(_end - _start).count();
        return dur/1000;
    }
private:
    void tick() {
        _end = timep_t{};
        _start = ClockT::now();
    }
    void tock() { _end = ClockT::now(); }
};

#endif // UTILS_H
