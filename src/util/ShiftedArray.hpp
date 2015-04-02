/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: ShiftedArray.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef SHIFTEDARRAY_HPP_
#define SHIFTEDARRAY_HPP_

#include "TdZddPool.hpp"

#include <cassert>
#include <cstring>

template<typename Data>
class ShiftedArray {
    int const psize;    // physical array size
    Data* parray;       // physical array pointer
    bool toBeDeleted;   // need to delete parray
    int vmin;           // minimum virtual index
    int vmax;           // maximum virtual index
    Data* varray;       // virtual array pointer

    ShiftedArray(ShiftedArray const&);
    ShiftedArray& operator=(ShiftedArray const&);

public:
    ShiftedArray(int psize, int vmin, int vmax)
            : psize(psize), parray(new Data[psize]), toBeDeleted(true),
              vmin(vmin), vmax(vmax), varray(parray - vmin) {
        assert(psize >= 1);
        assert(vmin <= vmax + 1);
        assert(vmax - vmin + 1 <= psize);
    }

    ShiftedArray(ShiftedArray const& o, TdZddPool& pool)
            : psize(o.psize),
              parray(new (pool.allocate<Data>(psize)) Data[psize]),
              toBeDeleted(false), vmin(o.vmin), vmax(o.vmax),
              varray(parray - vmin) {
        std::memcpy(parray, o.varray + o.vmin,
                (vmax - vmin + 1) * sizeof(Data));
    }

    virtual ~ShiftedArray() {
        if (toBeDeleted) delete[] parray;
        parray = 0;
    }

    bool hasIndex(int index) const {
        return vmin <= index && index <= vmax;
    }

    int minIndex() const {
        return vmin;
    }

    int maxIndex() const {
        return vmax;
    }

    size_t size() const {
        return vmax - vmin + 1;
    }

    void setMinIndex(int n) {
        assert(vmin <= n);
        vmin = n;
        if (vmin > vmax) {
            vmax = vmin - 1;
            varray = parray - vmin;
        }
    }

    void setMaxIndex(int n) {
        assert(vmax <= n);
        vmax = n;
//        if (vmax < vmin) {
//            vmin = vmax + 1;
//            varray = parray - vmin;
//        }
//        else
        if (varray + vmax >= parray + psize) {
            std::memmove(parray, varray + vmin,
                    (vmax - vmin + 1) * sizeof(Data));
            varray = parray - vmin;
            assert(varray + vmax < parray + psize);
        }
    }

    Data& operator[](int index) {
        assert(vmin <= index && index <= vmax);
        return varray[index];
    }

    Data operator[](int index) const {
        assert(vmin <= index && index <= vmax);
        return varray[index];
    }

    size_t hashCode() const {
        size_t h = vmin;
        for (int i = vmin; i <= vmax; ++i) {
            h = h * 31 + varray[i];
        }
        return h;
    }

    bool equals(ShiftedArray const& o) const {
        if (vmin != o.vmin) return false;
        if (vmax != o.vmax) return false;
        for (int i = vmin; i <= vmax; ++i) {
            if (varray[i] != o.varray[i]) return false;
        }
        return true;
    }

    friend std::ostream& operator<<(std::ostream& os, ShiftedArray const& o) {
        os << "[";
        for (int i = o.vmin; i <= o.vmax; ++i) {
            os << "(" << i << "," << o.varray[i] << ")";
        }
        return os << "]";
    }
};

#endif /* SHIFTEDARRAY_HPP_ */
