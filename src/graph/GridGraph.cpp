/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: GridGraph.cpp 9 2011-11-16 06:38:04Z iwashita $
 */

#include "GridGraph.hpp"

void GridGraph::resize(int rows, int cols) {
    reset();
    rows_ = rows;
    cols_ = cols;

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            VertexNumber v = getVertex(y, x);
            if (x < cols - 1) addArc(v, v + 1);
            if (y < rows - 1) addArc(v, v + cols);
        }
    }

    arcs.resize((rows - 1) * (cols - 1));

    for (int y = 0; y < rows - 1; ++y) {
        for (int x = 0; x < cols - 1; ++x) {
            auto& a = arcs[(cols - 1) * y + x];
            a.resize(4);
            a[0] = getArc(getVertex(y, x), getVertex(y, x + 1));
            a[1] = getArc(getVertex(y, x), getVertex(y + 1, x));
            a[2] = getArc(getVertex(y, x + 1), getVertex(y + 1, x + 1));
            a[3] = getArc(getVertex(y + 1, x), getVertex(y + 1, x + 1));
        }
    }

    setup();
}

void GridGraph::printAnswer(std::ostream& os,
        std::set<ArcNumber> const& answer) const {
    static char const* connector[] = { " ", "╴", "╶", "─", "╵", "┘", "└", "┴",
            "╷", "┐", "┌", "┬", "│", "┤", "├", "┼" };

    os << "┏";
    for (int x = 0; x < cols(); ++x) {
        os << "━";
    }
    os << "┓\n";

    for (int y = 0; y < rows(); ++y) {
        os << "┃";

        for (int x = 0; x < cols(); ++x) {
            int c = 0;
            if (x - 1 >= 0) {
                ArcNumber a = getArc(getVertex(y, x - 1), getVertex(y, x));
                if (answer.count(a)) c |= 1;
            }
            if (x + 1 < cols()) {
                ArcNumber a = getArc(getVertex(y, x), getVertex(y, x + 1));
                if (answer.count(a)) c |= 2;
            }
            if (y - 1 >= 0) {
                ArcNumber a = getArc(getVertex(y - 1, x), getVertex(y, x));
                if (answer.count(a)) c |= 4;
            }
            if (y + 1 < rows()) {
                ArcNumber a = getArc(getVertex(y, x), getVertex(y + 1, x));
                if (answer.count(a)) c |= 8;
            }
            os << connector[c];
        }

        os << "┃\n";
    }

    os << "┗";
    for (int x = 0; x < cols(); ++x) {
        os << "━";
    }
    os << "┛\n";
}

void GridGraph::printQuiz(std::ostream& os) const {
    printAnswer(os, std::set<ArcNumber>());
}
