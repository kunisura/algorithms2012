/*
 * Decode "mangled" name
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: demangle.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef DEMANGLE_HPP_
#define DEMANGLE_HPP_

#include <string>
#include <typeinfo>

std::string demangle(char const* name);

template<typename T>
std::string typenameof(T const& obj) {
    return demangle(typeid(obj).name());
}

#endif /* DEMANGLE_HPP_ */
