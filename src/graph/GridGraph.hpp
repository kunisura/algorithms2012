/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: GridGraph.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef GRIDGRAPH_HPP_
#define GRIDGRAPH_HPP_

#include <set>
#include <vector>

#include "Graph.hpp"

class GridGraph: public Graph {
    int rows_;
    int cols_;

    std::vector<std::vector<ArcNumber>> arcs;

public:
    GridGraph() {
    }

    GridGraph(int rows, int cols) {
        resize(rows, cols);
    }

    virtual ~GridGraph() {
    }

    int rows() const {
        return rows_;
    }

    int cols() const {
        return cols_;
    }

    VertexNumber getVertex(int y, int x) const {
        assert(0 <= y && y < rows_);
        assert(0 <= x && x < cols_);
        return cols_ * y + x + 1;
    }

    VertexNumber getVertex(int y, int x, int rotate) const {
        switch (rotate & 3) {
        case 1:
            return getVertex(rows_ - x - 1, y);
        case 2:
            return getVertex(rows_ - y - 1, cols_ - x - 1);
        case 3:
            return getVertex(x, cols_ - y - 1);
        default:
            return getVertex(y, x);
        }
    }

    std::vector<ArcNumber> const& getArcs(int y, int x) const {
        assert(0 <= y && y < rows_);
        assert(0 <= x && x < cols_);
        return arcs[(cols_ - 1) * y + x];
    }

    virtual void resize(int rows, int cols);

    virtual void printAnswer(std::ostream& os,
            std::set<ArcNumber> const& answer) const;
    virtual void printQuiz(std::ostream& os) const;
};

#endif /* GRIDGRAPH_HPP_ */
