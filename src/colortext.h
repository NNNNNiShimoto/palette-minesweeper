#ifndef COLORTEXT_H
#define COLORTEXT_H

#define ESC_RED "\x1b[31m"
#define ESC_GREEN "\x1b[32m"
#define ESC_YELLOW "\x1b[33m"
#define ESC_BLUE "\x1b[34m"
#define ESC_MAGENTA "\x1b[35m"
#define ESC_CYAN "\x1b[96m"
#define ESC_WHITE "\x1b[37m"

#include <string>
#include <sstream>

template <typename T>
std::string redText(const T& text) {
    std::ostringstream oss;
    oss << ESC_RED << std::string(text) << "\x1b[39m";
    return oss.str();
}

template <>
std::string redText<int>(const int& text) {    
    std::ostringstream oss;
    oss << ESC_RED << std::to_string(text) << "\x1b[39m";
    return oss.str();
}

template <typename T>
std::string greenText(const T& text) {
    std::ostringstream oss;
    oss << ESC_GREEN << std::string(text) << "\x1b[39m";
    return oss.str();
}

template <>
std::string greenText<int>(const int& text) {
    std::ostringstream oss;
    oss << ESC_GREEN << std::to_string(text) << "\x1b[39m";
    return oss.str();
}

template <typename T>
std::string yellowText(const T& text) {
    std::ostringstream oss;
    oss << ESC_YELLOW << std::string(text) << "\x1b[39m";
    return oss.str();
}

template <>
std::string yellowText<int>(const int& text) {
    std::ostringstream oss;
    oss << ESC_YELLOW << std::to_string(text) << "\x1b[39m";
    return oss.str();
}

template <typename T>
std::string blueText(const T& text) {
    std::ostringstream oss;
    oss << ESC_BLUE << std::string(text) << "\x1b[39m";
    return oss.str();
}

template <>
std::string blueText<int>(const int& text) {
    std::ostringstream oss;
    oss << ESC_BLUE << std::to_string(text) << "\x1b[39m";
    return oss.str();
}

template <typename T>
std::string magentaText(const T& text) {
    std::ostringstream oss;
    oss << ESC_MAGENTA << std::string(text) << "\x1b[39m";
    return oss.str();
}

template <>
std::string magentaText<int>(const int& text) {
    std::ostringstream oss;
    oss << ESC_MAGENTA << std::to_string(text) << "\x1b[39m";
    return oss.str();
}

template <typename T>
std::string cyanText(const T& text) {
    std::ostringstream oss;
    oss << ESC_CYAN << std::string(text) << "\x1b[39m";
    return oss.str();
}

template <>
std::string cyanText<int>(const int& text) {
    std::ostringstream oss;
    oss << ESC_CYAN << std::to_string(text) << "\x1b[39m";
    return oss.str();
}

template <typename T>
std::string whiteText(const T& text) {
    std::ostringstream oss;
    oss << ESC_WHITE << std::string(text) << "\x1b[39m";
    return oss.str();
}

template <>
std::string whiteText<int>(const int& text) {
    std::ostringstream oss;
    oss << ESC_WHITE << std::to_string(text) << "\x1b[39m";
    return oss.str();
}

template <typename T>
std::string boldText(const T& text) {
    std::ostringstream oss;
    oss << "\x1b[1m" << std::string(text) << "\x1b[0m";
    return oss.str();
}

template <typename T>
std::string underlineText(const T& text) {
    std::ostringstream oss;
    oss << "\x1b[4m" << std::string(text) << "\x1b[0m";
    return oss.str();
}

// template <>
// std::string boldText<int>(const int& text) {
//     return "\x1b[1m" + std::to_string(text) + "\x1b[0m";
// }

#endif