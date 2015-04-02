/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: Degree2.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef DEGREE2_HPP_
#define DEGREE2_HPP_

#include <iostream>

#include "TdZddPool.hpp"
#include "graph/Graph.hpp"
#include "util/ShiftedArray.hpp"

class Degree2 {
    typedef short DegreeCount;

    Graph const& graph;
    ShiftedArray<DegreeCount> dc;

    Degree2(Degree2 const&);
    Degree2& operator=(Degree2 const&);

public:
    Degree2(Graph const& graph)
            : graph(graph), dc(graph.maxFrontierSize(), 1, 0) {
    }

    Degree2(Degree2 const& o, TdZddPool& pool)
            : graph(o.graph), dc(o.dc, pool) {
    }

    size_t hashCode() const {
        return dc.hashCode();
    }

    bool equals(Degree2 const& o) const {
        return dc.equals(o.dc);
    }

    int down(bool take, int fromIndex, int toIndex);

    friend std::ostream& operator<<(std::ostream& os, Degree2 const& o) {
        return os << o.dc;
    }
};

#endif /* DEGREE2_HPP_ */
