/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: SlilinQuiz.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef SLILINQUIZ_HPP_
#define SLILINQUIZ_HPP_

#include <set>
#include <vector>

#include "GridGraph.hpp"

class SlilinQuiz: public GridGraph {
public:
    typedef int HintIndex; // -1 used

    struct Constraint {
        HintIndex index;
        int maxCount;
    };

private:
    std::vector<std::vector<int>> hint_;
    std::vector<std::vector<Constraint>> arcConstraints_;
    std::vector<int> initialCount_;
    std::vector<bool> arcTaken_;
    int rot;

    void sortHintIndices();

public:
    SlilinQuiz()
            : rot(0) {
    }

    void rotate(int d = 1) {
        rot += d;
    }

    int rotation() const{
        return rot;
    }

    int hint(int y, int x) const {
        assert(0 <= y && y < rows() - 1);
        assert(0 <= x && x < cols() - 1);
        return hint_[y][x];
    }

    HintIndex hints() const {
        return initialCount_.size();
    }

    int initialCount(HintIndex i) const {
        assert(i < HintIndex(initialCount_.size()));
        return initialCount_[i];
    }

    std::vector<Constraint> const& arcConstraints(ArcNumber a) const {
        assert(0 <= a && a < arcSize());
        return arcConstraints_[a];
    }

    bool arcTaken(ArcNumber a) const {
        assert(0 <= a && a < arcSize());
        return arcTaken_[a];
    }

    bool northArcTaken(int y, int x) const {
        VertexNumber v1 = getVertex(y, x, rot);
        VertexNumber v2 = getVertex(y, x + 1, rot);
        return arcTaken(getArc(v1, v2));
    }

    bool westArcTaken(int y, int x) const {
        VertexNumber v1 = getVertex(y, x, rot);
        VertexNumber v2 = getVertex(y + 1, x, rot);
        return arcTaken(getArc(v1, v2));
    }

    virtual void resize(int rows, int cols);
    void clearHint(int y, int x);
    void putNumber(int y, int x, int n);
    void readQuiz(std::istream& is);
    void readPicture(std::istream& is);
    void readAnswer(std::istream& is);
    void readAnswerOrQuiz(std::istream& is);
    void readAnswerOrPicture(std::istream& is);
    HintIndex maxHintWindowSize() const;

    virtual void printQuiz(std::ostream& os, std::vector<std::vector<int>> hint,
            std::set<ArcNumber> const& answer) const;
    virtual void printQuiz(std::ostream& os,
            std::vector<std::vector<int>> hint) const;
    virtual void printQuiz(std::ostream& os) const;
    virtual void printAnswer(std::ostream& os,
            std::set<ArcNumber> const& answer) const;

    friend std::ostream& operator<<(std::ostream& os, SlilinQuiz const& g);
};

#endif /* SLILINQUIZ_HPP_ */
