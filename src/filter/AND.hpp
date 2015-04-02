/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: AND.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef AND_HPP_
#define AND_HPP_

#include "TdZddPool.hpp"

template<typename F1, typename F2>
class AND {
    TdZddPool* pool;
    int const numVars;
    F1 filter1;
    F2 filter2;

    AND(AND const&);
    AND& operator=(AND const&);

public:
    AND(int numVars, F1 const& filter1, F2 const& filter2)
            : pool(new TdZddPool()), numVars(numVars), filter1(filter1, *pool),
              filter2(filter2, *pool) {
    }

    AND(AND const& o, TdZddPool& pool)
            : pool(0), numVars(o.numVars), filter1(o.filter1, pool),
              filter2(o.filter2, pool) {
    }

    virtual ~AND() {
        delete pool;
        pool = 0;
    }

    size_t hashCode() const {
        return filter1.hashCode() * 31 + filter2.hashCode();
    }

    bool equals(AND const& o) const {
        return filter1.equals(o.filter1) && filter2.equals(o.filter2);
    }

    int down(bool take, int fromIndex, int toIndex) {
        int v1 = filter1.down(take, fromIndex, toIndex);
        if (v1 == 0) return 0;
        if (v1 < 0) v1 = numVars;

        int v2 = filter2.down(take, fromIndex, v1);
        if (v2 == 0) return 0;
        if (v2 < 0) v2 = numVars;

        while (v1 != v2) {
            if (v1 < v2) {
                v1 = filter1.down(false, v1, v2);
                if (v1 == 0) return 0;
                if (v1 < 0) v1 = numVars;
            }
            else {
                v2 = filter2.down(false, v2, v1);
                if (v2 == 0) return 0;
                if (v2 < 0) v2 = numVars;
            }
        }

        return v1;
    }
};

#endif /* AND_HPP_ */
