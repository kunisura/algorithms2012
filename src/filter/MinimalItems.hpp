/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: MinimalItems.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef MINIMALITEMS_HPP_
#define MINIMALITEMS_HPP_

#include <cstring>

#include "TdZdd.hpp"
#include "TdZddPool.hpp"

class MinimalItems {
    struct NodePair {
        TdZddNode const* case0;
        TdZddNode const* case1;
    };

    TdZddNode const* f;
    int capacity;
    int size;
    NodePair* array;

public:
    MinimalItems(TdZdd const& dd)
            : f(dd.getTop()), capacity(0), size(0), array(0) {
    }

    MinimalItems(MinimalItems const& o, TdZddPool& pool)
            : f(o.f), capacity(o.size + 1), size(o.size),
              array(pool.allocate<NodePair>(capacity)) {
        std::memcpy(array, o.array, o.size * sizeof(*array));
    }

    size_t hashCode() const {
        size_t h = 0;
        for (int i = 0; i < size; ++i) {
            h = h * 31 + reinterpret_cast<size_t>(array[i].case0);
            h = h * 31 + reinterpret_cast<size_t>(array[i].case1);
        }
        return h;
    }

    bool equals(MinimalItems const& o) const {
        if (size != o.size) return false;
        for (int i = 0; i < size; ++i) {
            if (array[i].case0 != o.array[i].case0) return false;
            if (array[i].case1 != o.array[i].case1) return false;
        }
        return true;
    }

    int down(bool take, int fromIndex, int toIndex) {
        if (fromIndex == 0 && toIndex == f->getIndex() && !take) return toIndex;
        assert(fromIndex == f->getIndex());

        for (int i = 0; i < size; ++i) {
            auto& c0 = array[i].case0;
            auto& c1 = array[i].case1;
            if (c0 == 0) continue;
            assert(c0->getIndex() >= fromIndex);
            assert(c1->getIndex() >= fromIndex);

            if (c0->getIndex() == fromIndex) {
                c0 = take ? c0->getChild1() : c0->getChild0();
            }
            if (c1->getIndex() == fromIndex) {
                c1 = take ? c1->getChild1() : c1->getChild0();
            }
            if (c0->isConst0() || c1->isConst0()) {
                c0 = c1 = 0;
            }
            else {
                if (c0 == c1) return 0;
            }
        }

        if (take) {
            auto f0 = f->getChild0();
            auto f1 = f->getChild1();
            if (!f0->isConst0() && !f1->isConst0()) {
                if (f0 == f1) return 0;
                int i = size++;
                if (size > capacity) abort();
                array[i] = NodePair { f0, f1 };
            }
            f = f1;
        }
        else {
            f = f->getChild0();
        }

        return toIndex;
    }
};

#endif /* MINIMALITEMS_HPP_ */
