/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: DDFilter.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef DDFILTER_HPP_
#define DDFILTER_HPP_

#include <vector>
#include "TdZddPool.hpp"

template<typename DD, typename Vars = void>
class DDFilter {
    DD f;
    Vars const& vars;

public:
    DDFilter(DD const& f, Vars const& vars)
            : f(f), vars(vars) {
    }

    DDFilter(DDFilter const& o, TdZddPool& pool)
            : f(o.f), vars(o.vars) {
    }

    size_t hashCode() const {
        return f.id();
    }

    bool equals(DDFilter const& o) const {
        return f == o.f;
    }

    int down(bool take, int fromIndex, int toIndex) {
        assert(size_t(toIndex) <= vars.size());

        DD const& x = vars[fromIndex];
        f = f.constrain(take ? x : ~x);

        for (int i = fromIndex + 1; i < toIndex; ++i) {
            f = f.constrain(~vars[i]);
        }

        if (f.isConst0()) return 0;
        return toIndex;
    }
};

template<typename DD>
class DDFilter<DD,void> {
    DD f;

public:
    DDFilter(DD const& f)
            : f(f) {
    }

    DDFilter(DDFilter const& o, TdZddPool& pool)
            : f(o.f) {
    }

    size_t hashCode() const {
        return f.id();
    }

    bool equals(DDFilter const& o) const {
        return f == o.f;
    }

    int down(bool take, int fromIndex, int toIndex) {
        assert(f.level() >= fromIndex);
//        while (f.top() < fromIndex) {
//            f = f.child0();
//        }

        if (f.level() == fromIndex) {
            f = take ? f.child1() : f.child0();
        }
        else if (DD::isZDD && take) {
            return 0;
        }

        while (f.level() < toIndex) {
            f = f.child0();
        }

        if (f.isConst0()) return 0;
        if (!DD::isZDD) return toIndex;
        if (f.isConst1()) return -1;
        return f.level();
    }
};

#endif /* DDFILTER_HPP_ */
