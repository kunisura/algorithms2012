/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: UnivAbstract.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef UNIVABSTRACT_HPP_
#define UNIVABSTRACT_HPP_

#include <cstring>
#include <set>
#include <vector>

#include "TdZdd.hpp"
#include "TdZddPool.hpp"

class UnivAbstract {
    std::set<TdZddNode const*> set;
    std::vector<bool> const& abst;
    TdZddPool* pool;

public:
    UnivAbstract(TdZdd const& dd, std::vector<bool> const& abst)
            : set(), abst(abst), pool(0) {
        if (!dd.getTop()->isConst0()) set.insert(dd.getTop());
    }

    UnivAbstract(UnivAbstract const& o, TdZddPool& pool)
            : set(o.set), abst(o.abst), pool(&pool) {
    }

    size_t hashCode() const {
        size_t h = 0;
        for (auto p = set.begin(); p != set.end(); ++p) {
            h = h * 31 + reinterpret_cast<size_t>(*p);
        }
        return h;
    }

    bool equals(UnivAbstract const& o) const {
        return set == o.set;
    }

    int down(bool take, int fromIndex, int toIndex) {
        assert(0 <= fromIndex);
        assert(fromIndex < toIndex);
        assert(size_t(toIndex) <= abst.size());
        if (abst[fromIndex] && take) return 0; // ZDD default

        for (int i = fromIndex; i < toIndex; ++i) {
            std::set<TdZddNode const*> tmp(set);
            set.clear();
            bool const doAbst = abst[i];

            for (auto p = tmp.begin(); p != tmp.end(); ++p) {
                TdZddNode const* f = *p;
                assert(!f->isConst0());
                assert(f->getIndex() >= i);

                if (f->getIndex() == i) {
                    if (doAbst || !take) {
                        TdZddNode const* f0 = f->getChild0();
                        if (f0->isConst0()) return 0;
                        set.insert(f0);
                    }

                    if (doAbst || take) {
                        TdZddNode const* f1 = f->getChild1();
                        if (f1->isConst0()) return 0;
                        set.insert(f1);
                    }
                }
                else if (doAbst || take) {
                    return 0;
                }
                else {
                    set.insert(f);
                }
            }

            take = false;
        }

        return toIndex;
    }

    friend std::ostream& operator<<(std::ostream& os, UnivAbstract const& o) {
        bool sep = false;
        os << "{";
        for (auto p = o.set.begin(); p != o.set.end(); ++p) {
            if (sep) os << ", ";
            os << *p;
            sep = true;
        }
        return os << "}";
    }
};

class UnivAbstract2 {
    TdZddNode const* f0;
    TdZddNode const* f1;
    int const abst;

public:
    UnivAbstract2(TdZdd const& dd, int abst)
            : f0(dd.getTop()), f1(0), abst(abst) {
    }

    UnivAbstract2(UnivAbstract2 const& o, TdZddPool& pool)
            : f0(o.f0), f1(o.f1), abst(o.abst) {
    }

    size_t hashCode() const {
        return reinterpret_cast<size_t>(f0) + reinterpret_cast<size_t>(f1);
    }

    bool equals(UnivAbstract2 const& o) const {
        return (f0 == o.f0 && f1 == o.f1) || (f0 == o.f1 && f1 == o.f0);
    }

    int down(bool take, int fromIndex, int toIndex) {
        assert(0 <= fromIndex);
        assert(fromIndex < toIndex);
        if (take && fromIndex == abst) return 0; // ZDD default

        for (int i = fromIndex; i < toIndex; ++i) {
            assert(i <= f0->getIndex());

            if (f1) {
                assert(i > abst);
                assert(i <= f1->getIndex());

                if (i == f1->getIndex()) {
                    f1 = take ? f1->getChild1() : f1->getChild0();
                    if (f1->isConst0()) return 0;
                }
                else if (take) {
                    return 0;
                }
            }

            if (i == f0->getIndex()) {
                if (i == abst) {
                    assert(f1 == 0);
                    f1 = f0->getChild1();
                    if (f1->isConst0()) return 0;
                    f0 = f0->getChild0();
                    if (f0->isConst0()) return 0;
                }
                else {
                    f0 = take ? f0->getChild1() : f0->getChild0();
                    if (f0->isConst0()) return 0;
                }
            }
            else if (i == abst || take) {
                return 0;
            }

            take = false;
        }

        int min = f0->getIndex();
        int max = min;
        if (f1) {
            int i = f1->getIndex();
            if (i < min) {
                min = i;
            }
            else {
                max = i;
            }
        }
        if (toIndex <= abst && abst < max) return 0;
        assert(min >= toIndex);
        return min;
    }

    friend std::ostream& operator<<(std::ostream& os, UnivAbstract2 const& o) {
        os << "{" << o.f0;
        if (o.f1) os << ", " << o.f1;
        return os << "}";
    }
};

#endif /* UNIVABSTRACT_HPP_ */
