/*
 * Decode "mangled" name
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: demangle.cpp 9 2011-11-16 06:38:04Z iwashita $
 */

#include "demangle.hpp"
#include <cxxabi.h>

std::string demangle(char const* name) {
    char* dName = abi::__cxa_demangle(name, 0, 0, 0);
    if (dName == 0) return name;

    std::string s;
    char* p = dName;
    char c;
    while ((c = *p++) != 0) {
        s += c;
        if (!isalnum(c)) {
            while (std::isspace(*p)) {
                ++p;
            }
        }
    }

    free(dName);
    return s;
}
