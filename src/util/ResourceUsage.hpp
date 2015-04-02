/*
 * CPU Resource Usage
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: ResourceUsage.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef RESOURCEUSAGE_HPP_
#define RESOURCEUSAGE_HPP_

#include <iostream>

struct ResourceUsage {
    double utime;
    double stime;
    long maxrss;
    ResourceUsage();
    ResourceUsage(double utime, double stime, long maxrss);
    ResourceUsage& update();
    ResourceUsage operator+(ResourceUsage const& u) const;
    ResourceUsage& operator+=(ResourceUsage const& u);
    ResourceUsage operator-(ResourceUsage const& u) const;
    ResourceUsage& operator-=(ResourceUsage const& u);
    friend std::ostream& operator<<(std::ostream& os, ResourceUsage const& u);
};

#endif /* RESOURCEUSAGE_HPP_ */
