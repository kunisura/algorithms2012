/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: NumlinQuiz.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef NUMLINQUIZ_HPP_
#define NUMLINQUIZ_HPP_

#include <map>
#include <set>
#include <vector>

#include "GridGraph.hpp"

class NumlinQuiz: public GridGraph {
    std::vector<int> hint_;
    std::map<int,VertexNumber> hint2vertex;

public:
    int hint(int y, int x) const {
        assert(0 <= y && y < rows());
        assert(0 <= x && x < cols());
        return hint_[getVertex(y, x)];
    }

    virtual void resize(int rows, int cols);
    void putNumber(int y, int x, int n);
    void readQuiz(std::istream& is);

    virtual void printAnswer(std::ostream& os,
            std::set<ArcNumber> const& answer) const;

    friend std::ostream& operator<<(std::ostream& os, NumlinQuiz const& g);
};

#endif /* NUMLINQUIZ_HPP_ */
