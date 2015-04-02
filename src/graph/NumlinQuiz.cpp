/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: NumlinQuiz.cpp 9 2011-11-16 06:38:04Z iwashita $
 */

#include "NumlinQuiz.hpp"

#include <iomanip>

void NumlinQuiz::resize(int rows, int cols) {
    GridGraph::resize(rows, cols);

    hint_.resize(vertexSize() + 1);
    for (VertexNumber v = 1; v <= vertexSize(); ++v) {
        hint_[v] = -1;
    }
}

void NumlinQuiz::putNumber(int y, int x, int n) {
    assert(0 <= x && x < cols());
    assert(0 <= y && y < rows());
    assert(n >= 0);
    VertexNumber v = getVertex(y, x);
    hint_[v] = n;
    VertexNumber& u = hint2vertex[n];
    if (u == 0) {
        u = v;
    }
    else {
        addTargetPath(u, v);
    }
}

void NumlinQuiz::readQuiz(std::istream& is) {
    int cols;
    int rows;
    int c;

    for (;;) {
        if (!is) {
            std::cerr << "format error\n";
            return;
        }
        while (isspace(c = is.get()))
            ;
        if (isdigit(c)) break;
        while (is && (c = is.get()) != '\n')
            ;
    }
    is.unget();
    is >> cols;

    while (isspace(c = is.get()))
        ;
    if (c == ',') {
        rows = cols;
        while (isspace(c = is.get()))
            ;
        if (!isdigit(c)) {
            std::cerr << "format error\n";
            return;
        }
        is.unget();
        is >> cols;
    }
    else {
        if (!isdigit(c)) {
            std::cerr << "format error\n";
            return;
        }
        is.unget();
        is >> rows;
    }

    if (cols < 1 || rows < 1) {
        std::cerr << cols << " x " << rows << ": illegal size\n";
        return;
    }
    resize(rows, cols);

    int x = 0;
    int y = -1;

    while (is) {
        while (isspace(c = is.get())) {
            if (c == '\n') x = 0, ++y;
        }
        if (!is) break;

        if (x >= cols || y < 0 || y >= rows) {
            std::cerr << "(" << x << "," << y << "): out of bounds\n";
            continue;
        }

        if (isdigit(c)) {
            is.unget();
            int n;
            is >> n;
            putNumber(y, x, n);
        }
        ++x;
    }
}

void NumlinQuiz::printAnswer(std::ostream& os,
        std::set<ArcNumber> const& answer) const {
    static char const* connector[] = { "  ", "  ", "  ", "──", "  ", "─┘", " └",
            "─┴", "  ", "─┐", " ┌", "─┬", " │", "─┤", " ├", "─┼" };

    os << "┏";
    for (int x = 0; x < cols(); ++x) {
        os << "━━";
    }
    os << "━┓\n";

    for (int y = 0; y < rows(); ++y) {
        os << "┃";

        for (int x = 0; x < cols(); ++x) {
            int h = hint(y, x);
            if (h >= 0) {
                os << std::setw(2) << h;
            }
            else {
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
        }

        os << " ┃\n";
    }

    os << "┗";
    for (int x = 0; x < cols(); ++x) {
        os << "━━";
    }
    os << "━┛\n";
}

std::ostream& operator<<(std::ostream& os, NumlinQuiz const& g) {
    os << "graph {\n";
    for (NumlinQuiz::VertexNumber v = 0; v <= g.vertexSize(); ++v) {
        if (g.hint_[v] >= 0) {
            os << "  " << v << " [label=\"" << g.hint_[v] << "\"];\n";
        }
    }
    for (NumlinQuiz::ArcNumber a = 0; a < g.arcSize(); ++a) {
        auto vp = g.vertexPair(a);
        os << "  " << vp.first << " -- " << vp.second << "\n";
    }
    os << "}\n";
    os.flush();
    return os;
}
