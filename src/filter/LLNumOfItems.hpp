/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: LLNumOfItems.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef LLNUMOFITEMS_HPP_
#define LLNUMOFITEMS_HPP_

#include "TdZddPool.hpp"
#include "NumOfItems.hpp"

class LLNumOfItems: public NumOfItems {
    int count;

public:
    LLNumOfItems(int n)
            : count(n) {
    }

    LLNumOfItems(LLNumOfItems const& o, TdZddPool& pool)
            : count(o.count) {
    }

    size_t hashCode() const {
        return count;
    }

    bool equals(LLNumOfItems const& o) const {
        return count == o.count;
    }

    int down(bool take, int fromIndex, int toIndex, Range r) {
        if (r.max < 0) return 0;
        if (count <= r.min) return toIndex;
        if (take && --count <= r.min) return toIndex;
        if (count > r.max) return 0;
        return toIndex;
    }
};

#endif /* LLNUMOFITEMS_HPP_ */
