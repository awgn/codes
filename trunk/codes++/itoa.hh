/* $Id$ */
/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <bonelli@antifork.org> wrote this file. As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return. Nicola Bonelli 
 * ----------------------------------------------------------------------------
 */

#ifndef ITOA_HH
#define ITOA_HH

#include <iostream>
#include <cstdio>

namespace more {
    template <typename T>
    static inline std::string itoa(const T &value, const char *format) {
            char buff[32];
            snprintf(buff,32, format, value);
            return buff;
        }
    template <typename T> 
    static inline std::string itoa(const T &value) {
        class template_not_fully_specified;
        template_not_fully_specified();
        return std::string();
    }
    template <>
    inline std::string itoa(const char &value) {
            char buff[32];
            snprintf(buff,32, "%hhd", value);
            return buff;
        }
    template <>
    inline std::string itoa(const unsigned char &value) {
            char buff[32];
            snprintf(buff,32, "%hhu", value);
            return buff;
        }
    template <>
    inline std::string itoa(const short int &value) {
            char buff[32];
            snprintf(buff,32, "%hd", value);
            return buff;
        }
    template <>
    inline std::string itoa(const unsigned short int &value) {
            char buff[32];
            snprintf(buff,32, "%hu", value);
            return buff;
        }
    template <>
    inline std::string itoa(const int &value) {
            char buff[32];
            snprintf(buff,32, "%d", value);
            return buff;
        }    
    template <>
    inline std::string itoa(const unsigned int &value) {
            char buff[32];
            snprintf(buff,32, "%u", value);
            return buff;
        }
    template <>
    inline std::string itoa(const long int &value) {
            char buff[32];
            snprintf(buff,32, "%ld", value);
            return buff;
        }    
    template <>
    inline std::string itoa(const unsigned long int &value) {
            char buff[32];
            snprintf(buff,32, "%lu", value);
            return buff;
        }
    template <>
    inline std::string itoa(const long long int &value) {
            char buff[32];
            snprintf(buff,32, "%lld", value);
            return buff;
        }    
    template <>
    inline std::string itoa(const unsigned long long int &value) {
            char buff[32];
            snprintf(buff,32, "%llu", value);
            return buff;
        }

} 

#endif /* ITOA_HH */
