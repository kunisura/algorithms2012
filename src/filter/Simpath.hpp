/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: Simpath.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef SIMPATH_HPP_
#define SIMPATH_HPP_

#include <iostream>

#include "TdZddPool.hpp"
#include "graph/Graph.hpp"
#include "util/ShiftedArray.hpp"

class Simpath {
    Graph const& graph;
    ShiftedArray<Graph::VertexNumber> mate;

    Simpath(Simpath const&);
    Simpath& operator=(Simpath const&);

public:
    Simpath(Graph const& graph)
            : graph(graph), mate(graph.maxFrontierSize(), 1, 0) {
    }

    Simpath(Simpath const& o, TdZddPool& pool)
            : graph(o.graph), mate(o.mate, pool) {
    }

    size_t hashCode() const {
        return mate.hashCode();
    }

    bool equals(Simpath const& o) const {
        return mate.equals(o.mate);
    }

    int down(bool take, int fromIndex, int toIndex);

    friend std::ostream& operator<<(std::ostream& os, Simpath const& o) {
        return os << o.mate;
    }
};

#endif /* SIMPATH_HPP_ */
