/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: SlilinFilter.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef SLILINFILTER_HPP_
#define SLILINFILTER_HPP_

#include <iostream>

#include "TdZddPool.hpp"
#include "graph/SlilinQuiz.hpp"
#include "util/ShiftedArray.hpp"

class SlilinFilter {
    typedef short SlitherCount;

    SlilinQuiz const& graph;
    ShiftedArray<SlitherCount> sc;

    SlilinFilter(SlilinFilter const&);
    SlilinFilter& operator=(SlilinFilter const&);

public:
    SlilinFilter(SlilinQuiz const& graph)
            : graph(graph), sc(graph.maxHintWindowSize(), 0, -1) {
    }

    SlilinFilter(SlilinFilter const& o, TdZddPool& pool)
            : graph(o.graph), sc(o.sc, pool) {
    }

    size_t hashCode() const {
        return sc.hashCode();
    }

    bool equals(SlilinFilter const& o) const {
        return sc.equals(o.sc);
    }

    int down(bool take, int fromIndex, int toIndex);

    friend std::ostream& operator<<(std::ostream& os, SlilinFilter const& o) {
        return os << o.sc;
    }
};

#endif /* SLILINFILTER_HPP_ */
