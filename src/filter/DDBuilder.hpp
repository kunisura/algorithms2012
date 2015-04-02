/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: DDBuilder.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef DDBUILDER_HPP_
#define DDBUILDER_HPP_

template<typename DD>
struct DDBuilder {
    typedef DD ValueType;

    DD value0() const {
        return DD(0);
    }

    DD value1() const {
        return DD(1);
    }

    DD value(int k0, DD f0, int k1, DD f1, int k) const {
        if (!DD::isZDD) {
            while (--k1 > k) {
                f1 = DD(DD::level2index(k1), 0, f1);
            }
            while (--k0 > k) {
                f0 = DD(DD::level2index(k0), 0, f0);
            }
        }
        return DD(DD::level2index(k), f1, f0);
    }
};

#endif /* DDBUILDER_HPP_ */
