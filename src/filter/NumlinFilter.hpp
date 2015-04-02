/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: NumlinFilter.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef NUMLINFILTER_HPP_
#define NUMLINFILTER_HPP_

#include <iostream>

#include "TdZddPool.hpp"
#include "graph/Graph.hpp"
#include "util/ShiftedArray.hpp"

class NumlinFilter {
    Graph const& graph;
    Graph::PathNumber pathCount;
    ShiftedArray<Graph::VertexNumber> mate;

    NumlinFilter(NumlinFilter const&);
    NumlinFilter& operator=(NumlinFilter const&);

public:
    NumlinFilter(Graph const& graph)
            : graph(graph), pathCount(graph.numPath()),
              mate(graph.maxFrontierSize(), 1, 0) {
    }

    NumlinFilter(NumlinFilter const& o, TdZddPool& pool)
            : graph(o.graph), pathCount(o.pathCount), mate(o.mate, pool) {
    }

    size_t hashCode() const {
        return mate.hashCode();
    }

    bool equals(NumlinFilter const& o) const {
        return mate.equals(o.mate);
    }

    int down(bool take, int fromIndex, int toIndex);
};

class NumlinFilter2 {
    Graph const& graph;
    Graph::PathNumber pathCount;
    ShiftedArray<Graph::VertexNumber> mate;

    NumlinFilter2(NumlinFilter2 const&);
    NumlinFilter2& operator=(NumlinFilter2 const&);

public:
    NumlinFilter2(Graph const& graph)
            : graph(graph), pathCount(graph.numPath()),
              mate(graph.maxFrontierSize(), 1, 0) {
    }

    NumlinFilter2(NumlinFilter2 const& o, TdZddPool& pool)
            : graph(o.graph), pathCount(o.pathCount), mate(o.mate, pool) {
    }

    size_t hashCode() const {
        return mate.hashCode();
    }

    bool equals(NumlinFilter2 const& o) const {
        return mate.equals(o.mate);
    }

    int down(bool take, int fromIndex, int toIndex);
};

#endif /* NUMLINFILTER_HPP_ */
