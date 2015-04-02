/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: Degree0or2.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef DEGREE0OR2_HPP_
#define DEGREE0OR2_HPP_

#include <iostream>

#include "TdZddPool.hpp"
#include "graph/Graph.hpp"
#include "util/ShiftedArray.hpp"

class Degree0or2 {
    typedef short DegreeCount;

    Graph const& graph;
    ShiftedArray<DegreeCount> dc;

    Degree0or2(Degree0or2 const&);
    Degree0or2& operator=(Degree0or2 const&);

public:
    Degree0or2(Graph const& graph)
            : graph(graph), dc(graph.maxFrontierSize(), 1, 0) {
    }

    Degree0or2(Degree0or2 const& o, TdZddPool& pool)
            : graph(o.graph), dc(o.dc, pool) {
    }

    size_t hashCode() const {
        return dc.hashCode();
    }

    bool equals(Degree0or2 const& o) const {
        return dc.equals(o.dc);
    }

    int down(bool take, int fromIndex, int toIndex);

    friend std::ostream& operator<<(std::ostream& os, Degree0or2 const& o) {
        return os << o.dc;
    }
};

#endif /* DEGREE0OR2_HPP_ */
