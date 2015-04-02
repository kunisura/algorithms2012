/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: SlilinQuiz.cpp 9 2011-11-16 06:38:04Z iwashita $
 */

#include "SlilinQuiz.hpp"

#include <vector>
#include <sstream>

void SlilinQuiz::resize(int rows, int cols) {
    GridGraph::resize(rows, cols);

    hint_.resize(rows - 1);
    for (int y = 0; y < rows - 1; ++y) {
        hint_[y].resize(cols - 1);
        for (int x = 0; x < cols - 1; ++x) {
            hint_[y][x] = -1;
        }
    }

    arcConstraints_.clear();
    arcConstraints_.resize(arcSize());
    arcTaken_.clear();
    arcTaken_.resize(arcSize());
}

void SlilinQuiz::clearHint(int y, int x) {
    hint_[y][x] = -1;
}

void SlilinQuiz::putNumber(int y, int x, int n) {
    assert(0 <= y && y < rows() - 1);
    assert(0 <= x && x < cols() - 1);
    assert(0 <= n);
    hint_[y][x] = n;
    initialCount_.push_back(n);
    HintIndex const index = initialCount_.size() - 1;

    ArcNumber an = getArc(getVertex(y, x), getVertex(y, x + 1));
    ArcNumber aw = getArc(getVertex(y, x), getVertex(y + 1, x));
    ArcNumber ae = getArc(getVertex(y, x + 1), getVertex(y + 1, x + 1));
    ArcNumber as = getArc(getVertex(y + 1, x), getVertex(y + 1, x + 1));

    arcConstraints_[an].push_back(Constraint { index, 3 });
    arcConstraints_[aw].push_back(Constraint { index, 2 });
    arcConstraints_[ae].push_back(Constraint { index, 1 });
    arcConstraints_[as].push_back(Constraint { index, 0 });
}

void SlilinQuiz::sortHintIndices() {
    int const n = initialCount_.size();
    std::vector<HintIndex> perm(n);

    int i = 0;
    for (int y = 0; y < rows() - 1; ++y) {
        for (int x = 0; x < cols() - 1; ++x) {
            ArcNumber as = getArc(getVertex(y + 1, x), getVertex(y + 1, x + 1));
            for (Constraint const& c : arcConstraints_[as]) {
                if (c.maxCount == 0) perm[c.index] = i++;
            }
        }
    }

    std::vector<int> tmp(n);
    for (int i = 0; i < n; ++i) {
        tmp[perm[i]] = initialCount_[i];
    }
    initialCount_ = tmp;

    for (auto& constraints : arcConstraints_) {
        for (Constraint& c : constraints) {
            c.index = perm[c.index];
        }
    }
}

void SlilinQuiz::readQuiz(std::istream& is) {
    int nx;
    int ny;
    char c;

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
    is >> nx;

    while (isspace(c = is.get()))
        ;
    if (c == ',') {
        ny = nx;
        while (isspace(c = is.get()))
            ;
        if (!isdigit(c)) {
            std::cerr << "format error\n";
            return;
        }
        is.unget();
        is >> nx;
    }
    else {
        if (!isdigit(c)) {
            std::cerr << "format error\n";
            return;
        }
        is.unget();
        is >> ny;
    }

    if (nx < 1 || ny < 1) {
        std::cerr << nx << " x " << ny << ": illegal size\n";
        return;
    }

    if (ny < nx) {
        resize(nx + 1, ny + 1);
        rot = -1;
    }
    else {
        resize(ny + 1, nx + 1);
        rot = 0;
    }

    int x = 0;
    int y = -1;

    while (is) {
        while (isspace(c = is.get())) {
            if (c == '\n') x = 0, ++y;
        }
        if (!is) break;

        if (x >= nx || y < 0 || y >= ny) {
            std::cerr << "(" << x << "," << y << "): out of bounds\n";
            continue;
        }

        if (isdigit(c)) {
            is.unget();
            int v;
            is >> v;
            if (v < 0 || v > 4) {
                std::cerr << "(" << x << "," << y << ") = " << v
                        << ": illegal hint\n";
            }
            if (rot) {
                putNumber(x, ny - y - 1, v);
            }
            else {
                putNumber(y, x, v);
            }
        }
        ++x;
    }

    if (rot) sortHintIndices();
}

namespace {

static char const* const table = "--||--||--||++++"
        "++++++++++++++++"
        "++++++++++++++++"
        "++++++++++++++++"
        "++++++++++++--||"
        "-|++++++++++++++"
        "++++++++++++++++"
        "++++-|-|-|-|-|-|";

std::string getAnswerLine(std::istream& is) {
    std::stringstream ss;

    while (is) {
        int c = is.get();
        if (c == 0xe2) {
            c = is.get();
            if (c == 0x94) {
                c = is.get();
                if (0x80 <= c && c <= 0xff) {
                    c = table[c - 0x80];
                }
            }
        }
        if (c == '\n' || c == EOF) break;
        ss.put(c);
    }

    return ss.str();
}

}

void SlilinQuiz::readPicture(std::istream& is) {
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

    if (rows < cols) {
        std::swap(rows, cols);
        rot = -1;
    }
    else {
        rot = 0;
    }

    resize(rows + 1, cols + 1);

    while (is && is.get() != '\n')
        ;

    class Pict {
        int const rows;
        int const cols;
        std::vector<std::vector<bool>> val;
    public:
        Pict(int rows, int cols)
                : rows(rows), cols(cols), val(rows + 2) {
            for (int y = -1; y <= rows; ++y) {
                val[y + 1].resize(cols + 2);
            }
        }
        void set(int y, int x) {
            assert(-1 <= y && y <= rows);
            assert(-1 <= x && x <= cols);
            val[y + 1][x + 1] = true;
        }
        bool get(int y, int x) {
            assert(-1 <= y && y <= rows);
            assert(-1 <= x && x <= cols);
            return val[y + 1][x + 1];
        }
    } pict(rows, cols);

    for (int x = (rot ? cols - 1 : 0), y = 0; is;) {
        c = is.get();
        if (!is) break;

        if (c == '\n') {
            if (rot) --x, y = 0;
            else ++y, x = 0;
            continue;
        }

        if (c != ' ') {
            if (x >= cols || y >= rows) {
                std::cerr << "(" << x << "," << y << "): out of bounds\n";
            }
            else {
                pict.set(y, x);
            }
        }
        if (rot) ++y;
        else ++x;
    }

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            int n = 0;
            bool p = pict.get(y, x);
            if (p ^ pict.get(y - 1, x)) ++n;
            if (p ^ pict.get(y, x - 1)) ++n;
            if (p ^ pict.get(y + 1, x)) ++n;
            if (p ^ pict.get(y, x + 1)) ++n;
            putNumber(y, x, n);
        }
    }

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            ArcNumber an = getArc(getVertex(y, x), getVertex(y, x + 1));
            arcTaken_[an] = pict.get(y, x) ^ pict.get(y - 1, x);
            ArcNumber aw = getArc(getVertex(y, x), getVertex(y + 1, x));
            arcTaken_[aw] = pict.get(y, x) ^ pict.get(y, x - 1);
        }
        ArcNumber ae = getArc(getVertex(y, cols), getVertex(y + 1, cols));
        arcTaken_[ae] = pict.get(y, cols - 1) ^ pict.get(y, cols);
    }
    for (int x = 0; x < cols; ++x) {
        ArcNumber as = getArc(getVertex(rows, x), getVertex(rows, x + 1));
        arcTaken_[as] = pict.get(rows - 1, x) ^ pict.get(rows, x);
    }

    if (rot) sortHintIndices();
}

void SlilinQuiz::readAnswer(std::istream& is) {
    std::string s;

    while (is) {
        s = getAnswerLine(is);
        if (s.size() >= 1 && s[0] == '+') break;
    }

    int nx = (s.size() - 5) / 2;
    std::vector<std::string> lineLine;
    std::vector<std::string> hintLine;

    while (is) {
        s = getAnswerLine(is);
        if (s.size() == 0 || s[0] == '+') break;

        lineLine.push_back(s);

        s = getAnswerLine(is);
        if (s.size() == 0 || s[0] == '+') break;

        hintLine.push_back(s);
    }

    while (lineLine.size() <= hintLine.size()) {
        lineLine.push_back("");
    }

    int ny = hintLine.size();

    if (nx < 1 || ny < 1) {
        std::cerr << " illegal size (" << nx << " x " << ny << ")\n";
        return;
    }

    if (ny < nx) {
        resize(nx + 1, ny + 1);
        rot = -1;
    }
    else {
        resize(ny + 1, nx + 1);
        rot = 0;
    }

    for (int y = 0; y < ny + 1; ++y) {
        for (int x = 0; x < nx; ++x) {
            s = lineLine[y];
            size_t i = 3 + 2 * x;
            char c = (i < s.size()) ? s[i] : ' ';
            if (c == '-') {
                ArcNumber a = getArc(getVertex(y, x, rot),
                        getVertex(y, x + 1, rot));
                arcTaken_[a] = true;
            }
        }
    }

    for (int y = 0; y < ny; ++y) {
        for (int x = 0; x < nx + 1; ++x) {
            s = hintLine[y];
            size_t i = 2 + 2 * x;
            char c = (i < s.size()) ? s[i] : ' ';
            if (c == '|') {
                ArcNumber a = getArc(getVertex(y, x, rot),
                        getVertex(y + 1, x, rot));
                arcTaken_[a] = true;
            }
        }
    }

    for (int y = 0; y < ny; ++y) {
        for (int x = 0; x < nx; ++x) {
            s = hintLine[y];
            size_t i = 3 + 2 * x;
            int v = (i < s.size()) ? s[i] - '0' : -1;
            if (0 <= v && v <= 4) {
                if (rot) {
                    putNumber(x, ny - y - 1, v);
                }
                else {
                    putNumber(y, x, v);
                }
            }
        }
    }

    if (rot) sortHintIndices();
}

void SlilinQuiz::readAnswerOrQuiz(std::istream& is) {
    int c;

    while (is) {
        while (isspace(c = is.get()))
            ;

        if (isdigit(c)) {
            is.unget();
            readQuiz(is);
            return;
        }
        else if (c == '+' || c >= 0x80) {
            is.unget();
            readAnswer(is);
            return;
        }

        while (is && (c = is.get()) != '\n')
            ;
    }

    std::cerr << "format error\n";
}

void SlilinQuiz::readAnswerOrPicture(std::istream& is) {
    int c;

    while (is) {
        while (isspace(c = is.get()))
            ;

        if (isdigit(c)) {
            is.unget();
            readPicture(is);
            return;
        }
        else if (c == '+' || c >= 0x80) {
            is.unget();
            readAnswer(is);
            return;
        }

        while (is && (c = is.get()) != '\n')
            ;
    }

    std::cerr << "format error\n";
}

SlilinQuiz::HintIndex SlilinQuiz::maxHintWindowSize() const {
    HintIndex minIndex = 0;
    HintIndex maxIndex = 0;
    HintIndex size = 0;

    for (ArcNumber a = 0; a < arcSize(); ++a) {
        auto& ci = arcConstraints_[a];
        for (auto p = ci.begin(); p != ci.end(); ++p) {
            if (p->index >= maxIndex) maxIndex = p->index + 1;
        }
        HintIndex n = maxIndex - minIndex;
        if (n > size) size = n;
        for (auto p = ci.begin(); p != ci.end(); ++p) {
            if (p->maxCount == 0) minIndex = p->index + 1;
        }
    }

    assert(minIndex == maxIndex);
    return size;
}

void SlilinQuiz::printQuiz(std::ostream& os, std::vector<std::vector<int>> hint,
        std::set<ArcNumber> const& answer) const {
    static char const* connector[] = { " ", " ", " ", "─", " ", "┘", "└", "┴",
            " ", "┐", "┌", "┬", "│", "┤", "├", "┼" };
    int const ny = (rot & 1) ? cols() : rows();
    int const nx = (rot & 1) ? rows() : cols();

    os << "┏";
    for (int x = 0; x < nx; ++x) {
        os << "━━";
    }
    os << "━┓\n";

    for (int y = 0; y < ny; ++y) {
        os << "┃ ";

        for (int x = 0; x < nx; ++x) {
            int c = 0;
            if (x - 1 >= 0) {
                ArcNumber a = getArc(getVertex(y, x - 1, rot),
                        getVertex(y, x, rot));
                if (answer.count(a)) c |= 1;
            }
            if (x + 1 < nx) {
                ArcNumber a = getArc(getVertex(y, x, rot),
                        getVertex(y, x + 1, rot));
                if (answer.count(a)) c |= 2;
            }
            if (y - 1 >= 0) {
                ArcNumber a = getArc(getVertex(y - 1, x, rot),
                        getVertex(y, x, rot));
                if (answer.count(a)) c |= 4;
            }
            if (y + 1 < ny) {
                ArcNumber a = getArc(getVertex(y, x, rot),
                        getVertex(y + 1, x, rot));
                if (answer.count(a)) c |= 8;
            }
            os << connector[c];

            if (x + 1 < nx) {
                ArcNumber a = getArc(getVertex(y, x, rot),
                        getVertex(y, x + 1, rot));
                os << (answer.count(a) ? "─" : " ");
            }
        }

        os << " ┃\n";

        if (y + 1 < ny) {
            os << "┃ ";

            for (int x = 0; x < nx; ++x) {
                ArcNumber a = getArc(getVertex(y, x, rot),
                        getVertex(y + 1, x, rot));
                os << (answer.count(a) ? "│" : " ");

                if (x + 1 < nx) {
                    int r = rot & 3;
                    int n = (r == 1) ? hint[nx - x - 2][y] :
                            (r == 2) ? hint[ny - y - 2][nx - x - 2] :
                            (r == 3) ? hint[x][ny - y - 2] : hint[y][x];
                    if (n >= 0) os << n;
                    else os << " ";
                }
            }

            os << " ┃\n";
        }
    }

    os << "┗";
    for (int x = 0; x < nx; ++x) {
        os << "━━";
    }
    os << "━┛\n";
}

void SlilinQuiz::printQuiz(std::ostream& os,
        std::vector<std::vector<int>> hint) const {
    std::set<ArcNumber> answer;
    for (ArcNumber a = 0; a < arcSize(); ++a) {
        if (arcTaken_[a]) answer.insert(a);
    }
    printQuiz(os, hint, answer);
}

void SlilinQuiz::printQuiz(std::ostream& os) const {
    printQuiz(os, hint_);
}

void SlilinQuiz::printAnswer(std::ostream& os,
        std::set<ArcNumber> const& answer) const {
    printQuiz(os, hint_, answer);
}

std::ostream& operator<<(std::ostream& os, SlilinQuiz const& g) {
    os << "graph {\n";
    for (SlilinQuiz::ArcNumber a = 0; a < g.arcSize(); ++a) {
        auto vp = g.vertexPair(a);
        os << "  " << vp.first << " -- " << vp.second << " [label=\"";
        auto& ci = g.arcConstraints_[a];
        for (auto p = ci.begin(); p != ci.end(); ++p) {
            int index = p->index;
            os << "{@" << index << "(" << g.initialCount_[index] << "),"
                    << p->maxCount << "}";
        }
        os << "\"];\n";
    }
    os << "}\n";
    os.flush();
    return os;
}
