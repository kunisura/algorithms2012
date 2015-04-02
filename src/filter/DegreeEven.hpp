/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: DegreeEven.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef DEGREEEVEN_HPP_
#define DEGREEEVEN_HPP_

#include <iostream>

#include "TdZddPool.hpp"
#include "graph/Graph.hpp"
#include "util/ShiftedArray.hpp"

class DegreeEven {
    typedef short DegreeCount;

    Graph const& graph;
    ShiftedArray<DegreeCount> dc;

    DegreeEven(DegreeEven const&);
    DegreeEven& operator=(DegreeEven const&);

public:
    DegreeEven(Graph const& graph)
            : graph(graph), dc(graph.maxFrontierSize(), 1, 0) {
    }

    DegreeEven(DegreeEven const& o, TdZddPool& pool)
            : graph(o.graph), dc(o.dc, pool) {
    }

    size_t hashCode() const {
        return dc.hashCode();
    }

    bool equals(DegreeEven const& o) const {
        return dc.equals(o.dc);
    }

    int down(bool take, int fromIndex, int toIndex);

    friend std::ostream& operator<<(std::ostream& os, DegreeEven const& o) {
        return os << o.dc;
    }
};

#endif /* DEGREEEVEN_HPP_ */
