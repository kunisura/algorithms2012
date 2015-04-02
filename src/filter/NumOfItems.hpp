/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: NumOfItems.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef NUMOFITEMS_HPP_
#define NUMOFITEMS_HPP_

struct NumOfItems {
    typedef struct Range {
        int min;
        int max;
    } ValueType;

    Range value0() const {
        Range r = { -1, -1 };
        return r;
    }

    Range value1() const {
        Range r = { 0, 0 };
        return r;
    }

    Range value(int k0, Range r0, int k1, Range r1, int k) const {
        if (r0.max >= 0 && r1.max >= 0) {
            int min = std::min(r0.min, r1.min + 1);
            int max = std::max(r0.max, r1.max + 1);
            Range r = { min, max };
            return r;
        }
        else if (r1.max >= 0) {
            Range r = { r1.min + 1, r1.max + 1 };
            return r;
        }
        else {
            return r0;
        }
    }
};

#endif /* NUMOFITEMS_HPP_ */
