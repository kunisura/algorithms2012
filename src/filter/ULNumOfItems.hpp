/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: ULNumOfItems.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef ULNUMOFITEMS_HPP_
#define ULNUMOFITEMS_HPP_

#include "TdZddPool.hpp"
#include "NumOfItems.hpp"

class ULNumOfItems: public NumOfItems {
    int count;

public:
    ULNumOfItems(int n)
            : count(n) {
    }

    ULNumOfItems(ULNumOfItems const& o, TdZddPool& pool)
            : count(o.count) {
    }

    size_t hashCode() const {
        return count;
    }

    bool equals(ULNumOfItems const& o) const {
        return count == o.count;
    }

    int down(bool take, int fromIndex, int toIndex, Range r) {
        if (r.max < 0) return 0;
        if (count < r.min) return 0;
        if (take && --count < r.min) return 0;
        if (count > r.max) count = r.max + 1;
        return toIndex;
    }
};

#endif /* ULNUMOFITEMS_HPP_ */
