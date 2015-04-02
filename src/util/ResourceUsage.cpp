/*
 * CPU Resource Usage
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: ResourceUsage.cpp 9 2011-11-16 06:38:04Z iwashita $
 */

#include "ResourceUsage.hpp"

#include <cctype>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <sys/time.h>
#include <sys/resource.h>

using std::string;
using std::ifstream;
using std::istringstream;
using std::ostream;
using std::ios;
using std::ios_base;
using std::setprecision;
using std::max;

ResourceUsage::ResourceUsage() {
    update();
}

ResourceUsage::ResourceUsage(double utime, double stime, long maxrss) :
    utime(utime), stime(stime), maxrss(maxrss) {
}

namespace {
long readMemoryStatus(string key) {
    ifstream ifs("/proc/self/status");
    string buf;

    while (ifs.good()) {
        getline(ifs, buf);
        if (buf.compare(0, key.length(), key) == 0) {
            istringstream iss(buf.substr(key.length()));
            double size;
            string unit;
            iss >> size >> unit;
            switch (tolower(unit[0])) {
            case 'b':
                size *= 1e-3;
                break;
            case 'm':
                size *= 1e+3;
                break;
            case 'g':
                size *= 1e+6;
                break;
            case 't':
                size *= 1e+9;
                break;
            }
            return long(size);
        }
    }

    return 0;
}
}

ResourceUsage& ResourceUsage::update() {
    struct rusage s;
    getrusage(RUSAGE_SELF, &s);
    utime = s.ru_utime.tv_sec + s.ru_utime.tv_usec * 1e-6;
    stime = s.ru_stime.tv_sec + s.ru_stime.tv_usec * 1e-6;
    maxrss = s.ru_maxrss;
    if (maxrss == 0) maxrss = readMemoryStatus("VmHWM:");
    return *this;
}

ResourceUsage ResourceUsage::operator+(ResourceUsage const& u) const {
    return ResourceUsage(utime + u.utime, stime + u.stime,
            max(maxrss, u.maxrss));
}

ResourceUsage& ResourceUsage::operator+=(ResourceUsage const& u) {
    utime += u.utime;
    stime += u.stime;
    if (maxrss < u.maxrss) maxrss = u.maxrss;
    return *this;
}

ResourceUsage ResourceUsage::operator-(ResourceUsage const& u) const {
    return ResourceUsage(utime - u.utime, stime - u.stime,
            max(maxrss, u.maxrss));
}

ResourceUsage& ResourceUsage::operator-=(ResourceUsage const& u) {
    utime -= u.utime;
    stime -= u.stime;
    if (maxrss < u.maxrss) maxrss = u.maxrss;
    return *this;
}

ostream& operator<<(ostream& os, ResourceUsage const& u) {
    ios_base::fmtflags backup = os.flags(ios::fixed);

    os << setprecision(2) << u.utime << "s + ";
    os << setprecision(2) << u.stime << "s, ";
    os << setprecision(0) << u.maxrss / 1024.0 << "MB";

    os.flags(backup);
    return os;
}
