/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: zsligen.cpp 9 2011-11-16 06:38:04Z iwashita $
 */

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <string>
#include <typeinfo>

#include "TdZdd.hpp"

#include "dd/cudd_BDD.hpp"
#include "filter/DDBuilder.hpp"
#include "filter/DDFilter.hpp"
#include "filter/Degree0or2.hpp"
#include "filter/DegreeEven.hpp"
#include "filter/LLNumOfItems.hpp"
#include "filter/UnivAbstract.hpp"
#include "filter/MinimalItems.hpp"
#include "filter/NumOfItems.hpp"
#include "filter/Simpath.hpp"
//#include "filter/SlilinGen.hpp"
#include "filter/ULNumOfItems.hpp"
#include "graph/SlilinQuiz.hpp"
#include "util/MessageHandler.hpp"
#include "util/demangle.hpp"

bool opt_0 = false;
bool opt_1 = false;
bool opt_2 = false;
bool opt_3 = false;
bool opt_4 = false;
bool opt_graph = false;
bool opt_dump = false;
bool opt_dump1 = false;
bool opt_dump2 = false;
bool opt_dump3 = false;
bool opt_m = false;
bool opt_o = false;
bool opt_n = false;
bool opt_s = false;
bool opt_d = false;
bool opt_a = false;
bool opt_l = false;
bool opt_r = false;
bool opt_csv = false;
bool opt_tex = false;
bool opt_noreport = false;

void usage(char const* cmd) {
    std::cerr << "usage: " << cmd << " <option>... <quiz_file>\n";
    std::cerr << "options\n";
    std::cerr << "  -m:   Allow disjoint multiple cycles\n";
    std::cerr << "  -o:   Allow overlapped multiple cycles\n";
    std::cerr << "  -n:   Do not process\n";
    std::cerr << "  -s:   Select sparse puzzles\n";
    std::cerr << "  -d:   Select difficult puzzles\n";
    std::cerr << "  -a:   Output all puzzles\n";
    std::cerr << "  -l:   Rotate output left\n";
    std::cerr << "  -r:   Rotate output right\n";
    std::cerr << "  -csv: Generate CSV\n";
    std::cerr << "  -tex: Generate LaTeX picture\n";
}

void dump(std::ostream& os, TdZdd const& dd, Graph const& g) {
    dd.dump(os, [g](int i) {return g.arcName(i);});
}

//template<typename Gen>
//void slilinGen(Gen const& gen, TdZdd& dd) {
//    MessageHandler mh;
//    mh.begin(typenameof(gen)) << " ...";
//    auto f = gen.getQuizSet(dd.evaluate(gen));
//    mh.end(f.size());
//
//    if (!opt_noreport) {
//        mh << "#node = " << f.size() << ", #puzzle = " << std::setprecision(6)
//                << f.pathCount() << "\n";
//    }
//
//    mh.begin("transforming to TdZdd") << " ...";
//    dd.initialize(gen.variables());
//    dd.subset(DDFilter<decltype(f)>(f));
//    dd.reduce();
//    mh.end(dd.size());
//
//    if (opt_dump) {
//        dd.dump(std::cout, [gen](int i) -> std::string {
//            return gen.var2name(i);
//        });
//    }
//
//    auto length = dd.evaluate(NumOfItems());
//    if (!opt_noreport) {
//        mh << "#node = " << dd.size() << ", #puzzle = " << std::setprecision(6)
//                << dd.pathCount() << ", #hint = [" << length.min << ","
//                << length.max << "]\n";
//    }
//
//    if (length.min != length.max) {
//        mh.begin("selecting minimum hint puzzles") << " ...";
//        dd.evalAndSubset(ULNumOfItems(length.min));
//        dd.reduce();
//        mh.end(dd.size());
//
//        if (!opt_noreport) {
//            auto length = dd.evaluate(NumOfItems());
//            mh << "#node = " << dd.size() << ", #puzzle = "
//                    << std::setprecision(6) << dd.pathCount() << ", #hint = ["
//                    << length.min << "," << length.max << "]\n";
//        }
//    }
//
//    if (opt_a) {
//        for (auto p = dd.begin(); p != dd.end(); ++p) {
//            std::vector<std::vector<int>> hint(gen.rows());
//            for (int y = 0; y < gen.rows(); ++y) {
//                hint[y].resize(gen.cols());
//                for (int x = 0; x < gen.cols(); ++x) {
//                    hint[y][x] = -1;
//                }
//            }
//
//            for (auto q = p->begin(); q != p->end(); ++q) {
//                int y = gen.var2row(*q);
//                int x = gen.var2col(*q);
//                int h = gen.var2val(*q);
//                hint[y][x] = h;
//            }
//
//            gen.printQuiz(std::cout, hint);
//        }
//    }
//}
//
//template<typename Gen>
//void slilinGen4(Gen const& gen, TdZdd& dd) {
//    struct RTA {
//        SlilinQuiz const& quiz;
//        bool notTarget;
//    public:
//        RTA(SlilinQuiz const& quiz)
//                : quiz(quiz), notTarget(false) {
//        }
//        RTA(RTA const& o, TdZddPool& pool)
//                : quiz(o.quiz), notTarget(o.notTarget) {
//        }
//        size_t hashCode() const {
//            return notTarget;
//        }
//        bool equals(RTA const& o) const {
//            return notTarget == o.notTarget;
//        }
//        int down(bool take, int fromIndex, int toIndex) {
//            if (notTarget) return toIndex;
//            for (int i = fromIndex; i < toIndex; ++i) {
//                if (take != quiz.arcTaken(i)) {
//                    notTarget = true;
//                    return toIndex;
//                }
//                take = false;
//            }
//            if (size_t(toIndex) >= quiz.arcSize()) return 0;
//            return toIndex;
//        }
//    };
//
//    MessageHandler mh;
//    mh.begin("removing the target answer") << " ...";
//    dd.subset(RTA(gen.getQuiz()));
//    mh.end(dd.size());
//
//    if (opt_dump3) dump(std::cout, dd, gen.getQuiz());
//
//    if (!opt_noreport) {
//        auto length = dd.evaluate(NumOfItems());
//        mh << "#node = " << dd.size() << ", #puzzle = " << std::setprecision(6)
//                << dd.pathCount() << ", #hint = [" << length.min << ","
//                << length.max << "]\n";
//    }
//
//    mh.begin(typenameof(gen)) << " ...";
//    auto f = gen.getQuizSet(dd.evaluate(gen));
//    mh.end(f.size());
//
//    if (!opt_noreport) {
//        mh << "#node = " << f.size() << ", #puzzle = " << std::setprecision(6)
//                << f.pathCount() << "\n";
//    }
//
//    mh.begin("transforming to TdZdd") << " ...";
//    dd.initialize(gen.variables());
//    dd.subset(DDFilter<decltype(f)>(f));
//    dd.reduce();
//    mh.end(dd.size());
//
//    if (opt_dump) {
//        dd.dump(std::cout, [gen](int i) -> std::string {
//            return gen.var2name(i);
//        });
//    }
//
//    auto length = dd.evaluate(NumOfItems());
//    if (!opt_noreport) {
//        mh << "#node = " << dd.size() << ", #puzzle = " << std::setprecision(6)
//                << dd.pathCount() << ", #hint = [" << length.min << ","
//                << length.max << "]\n";
//    }
//
//    if (length.min != length.max) {
//        mh.begin("selecting minimum hint puzzles") << " ...";
//        dd.evalAndSubset(ULNumOfItems(length.min));
//        dd.reduce();
//        mh.end(dd.size());
//
//        if (!opt_noreport) {
//            auto length = dd.evaluate(NumOfItems());
//            mh << "#node = " << dd.size() << ", #puzzle = "
//                    << std::setprecision(6) << dd.pathCount() << ", #hint = ["
//                    << length.min << "," << length.max << "]\n";
//        }
//    }
//
//    if (opt_a) {
//        for (auto p = dd.begin(); p != dd.end(); ++p) {
//            std::vector<std::vector<int>> hint(gen.rows());
//            for (int y = 0; y < gen.rows(); ++y) {
//                hint[y].resize(gen.cols());
//                for (int x = 0; x < gen.cols(); ++x) {
//                    hint[y][x] = -1;
//                }
//            }
//
//            for (auto q = p->begin(); q != p->end(); ++q) {
//                int h = *q;
//                hint[gen.var2row(h)][gen.var2col(h)] = gen.var2val(h);
//            }
//
//            gen.printQuiz(std::cout, hint);
//        }
//    }
//}

class Score {
    unsigned int count[5];
public:
    Score()
            : count { 0, 0, 0, 0, 0 } {
    }
    Score(unsigned int n)
            : count { n, n, n, n, n } {
    }
    void add(int h) {
        if (0 <= h && h <= 4) ++count[h];
    }
    bool operator<(Score const& o) const {
        if (count[4] != o.count[4]) return count[4] > o.count[4];
        if (count[0] != o.count[0]) return count[0] > o.count[0];
        if (count[3] != o.count[3]) return count[3] > o.count[3];
        if (count[1] != o.count[1]) return count[1] > o.count[1];
        return count[2] > o.count[2];
    }
    bool operator==(Score const& o) const {
        for (int i = 0; i <= 4; ++i) {
            if (count[i] != o.count[i]) return false;
        }
        return true;
    }
    friend std::ostream& operator<<(std::ostream& os, Score const& s) {
        for (int i = 0;; ++i) {
            os << s.count[i];
            if (i == 4) break;
            os << "-";
        }
        return os;
    }
};

void slilinGenByBDD(SlilinQuiz const& quiz, TdZdd& dd) {
    typedef cudd::BDD BDD;
    MessageHandler mh;
    mh.begin("transforming to BDD") << " ...";
    BDD f = dd.evaluate(DDBuilder<BDD>());
    mh.end(f.size());

    if (!opt_noreport) {
        mh << "#node = " << f.size() << ", #cycle = " << std::setprecision(6)
                << f.pathCount() << "\n";
    }

    mh.begin("removing the target answer") << " ...";
    int arcSize = quiz.arcSize();
    std::vector<BDD> arcVar(arcSize);
    for (int i = 0; i < arcSize; ++i) {
        arcVar[i] = BDD(i, 1, 0);
    }
    {
        BDD g(1);
        for (int i = arcSize - 1; i >= 0; --i) {
            g &= quiz.arcTaken(i) ? arcVar[i] : ~arcVar[i];
        }
        f -= g;
    }
    mh.end(f.size());

    //BDD::reorder();

    mh.begin("mapping to the hint space") << " ...";
    std::vector<std::string> hintName;
    std::vector<BDD> hintVar;
    {
        std::vector<BDD> relation;
        BDD g(1);
        for (int row = 0; row < quiz.rows() - 1; ++row) {
            //mh << "\n";
            for (int col = 0; col < quiz.cols() - 1; ++col) {
                //mh << ".";
                std::string name = "(" + std::to_string(row) + ","
                        + std::to_string(col) + ")";
                hintName.push_back(name);
                auto& arcs = quiz.getArcs(row, col);
                int n = arcs.size();
                std::vector<BDD> vars(n);
                for (int a = 0; a < n; ++a) {
                    vars[a] = arcVar[arcs[a]];
                }
                BDD v = BDD(vars[0].insertNewVar(), 1, 0);
                hintVar.push_back(v);

                int m = quiz.hint(row, col);
                if (m < 0) {
                    g &= ~v;
                }
                else if (f.isConst0()) {
                    g &= v;
                }
                else {
                    BDD h = ~v | BDD::nHot(m, vars);
                    relation.push_back(h.constrain(f));
                }
            }
        }

        if (!f.isConst0()) {
            g &= ~BDD::andAbstractAll(relation, BDD::andAll(arcVar));
        }
        f = g;
    }
    mh.end(f.size());

    mh.begin("transforming to TdZdd") << " ...";
    dd.initialize(hintVar.size());
    dd.subset(DDFilter<BDD,std::vector<BDD>>(f, hintVar));
    dd.reduce();
    mh.end(dd.size());

    if (opt_dump) {
        dd.dump(std::cout, [hintName](int i) {return hintName[i];});
    }
}

//void slilinGenByTdZdd(SlilinQuiz const& quiz, TdZdd& dd) {
//    typedef cudd::BDD BDD;
//    MessageHandler mh;
//    mh.begin("transforming to BDD") << " ...";
//    BDD f = dd.evaluate(DDBuilder<BDD>());
//    mh.end(f.size());
//
//    if (!opt_noreport) {
//        mh << "#node = " << f.size() << ", #cycle = " << std::setprecision(6)
//                << f.pathCount() << "\n";
//    }
//
//    mh.begin("removing the target answer") << " ...";
//    int arcSize = quiz.arcSize();
//    std::vector<BDD> arcVar(arcSize);
//    for (int i = 0; i < arcSize; ++i) {
//        arcVar[i] = BDD(i, 1, 0);
//    }
//    {
//        BDD g(1);
//        for (int i = arcSize - 1; i >= 0; --i) {
//            g &= quiz.arcTaken(i) ? arcVar[i] : ~arcVar[i];
//        }
//        f -= g;
//    }
//    mh.end(f.size());
//
//    mh.begin("mapping to the hint space") << " ...";
//    int hintSize = (quiz.rows() - 1) * (quiz.cols() - 1);
//    std::vector<std::string> hintName(hintSize);
//    std::vector<BDD> hintVar(hintSize);
//    for (int row = quiz.rows() - 2; row >= 0; --row) {
//        mh << "\n";
//        for (int col = quiz.cols() - 2; col >= 0; --col) {
//            int i = row * (quiz.cols() - 1) + col;
//            hintName[i] = "(" + std::to_string(row) + "," + std::to_string(col)
//                    + ")";
//            int m = quiz.hint(row, col);
//            if (m < 0) continue;
//            auto& arcs = quiz.getArcs(row, col);
//            int n = arcs.size();
//            std::vector<BDD> vars(n);
//            for (int a = 0; a < n; ++a) {
//                vars[a] = arcVar[arcs[a]];
//            }
//            BDD h = BDD::nHot(m, vars);
//            hintVar[i] = BDD(h.insertNewVar(), 1, 0);
//            f &= ~hintVar[i] | h;
//            mh << ".";
//        }
//    }
//
//    std::vector<std::string> varName(arcSize + hintSize);
//    std::vector<bool> isArcVar(arcSize + hintSize);
//    for (int i = 0; i < arcSize; ++i) {
//        int k = arcVar[i].level();
//        varName[k] = quiz.arcName(i);
//        isArcVar[k] = true;
//    }
//    for (int i = 0; i < hintSize; ++i) {
//        int k = hintVar[i].level();
//        varName[k] = hintName[i];
//        isArcVar[k] = false;
//    }
//
//    f = ~f;
//    mh.end(f.size());
//
//    mh.begin("transforming to TdZdd") << " ...";
//    TdZdd tmp(arcSize + hintSize);
//    tmp.subset(DDFilter<BDD>(f));
//    tmp.reduce();
//    mh.end(tmp.size());
//
//    //dd.initialize(arcSize + hintSize);
//    dd = tmp;
//    int n = arcSize + hintSize;
//    for (int i = n - 1; i >= 0; --i) {
//        if (isArcVar[i]) {
//            mh.begin("UnivAbstract") << " " << n - i << "/" << n << " ...";
//            dd.subset(UnivAbstract2(dd, i));
//            dd.reduce();
//            mh.end(dd.size());
//        }
//    }
//
//    if (opt_dump) {
//        dd.dump(std::cout, [varName](int i) {return varName[i];});
//    }
//
//    auto length = dd.evaluate(NumOfItems());
//    if (!opt_noreport) {
//        mh << "#node = " << dd.size() << ", #puzzle = " << std::setprecision(6)
//                << dd.pathCount() << ", #hint = [" << length.min << ","
//                << length.max << "]\n";
//    }
//}

//void slilinGenByTdZdd(SlilinQuiz const& quiz, TdZdd& dd) {
//    struct RTA {
//        SlilinQuiz const& quiz;
//        bool notTarget;
//    public:
//        RTA(SlilinQuiz const& quiz)
//                : quiz(quiz), notTarget(false) {
//        }
//        RTA(RTA const& o, TdZddPool& pool)
//                : quiz(o.quiz), notTarget(o.notTarget) {
//        }
//        size_t hashCode() const {
//            return notTarget;
//        }
//        bool equals(RTA const& o) const {
//            return notTarget == o.notTarget;
//        }
//        int down(bool take, int fromIndex, int toIndex) {
//            if (notTarget) return toIndex;
//            for (int i = fromIndex; i < toIndex; ++i) {
//                if (take != quiz.arcTaken(i)) {
//                    notTarget = true;
//                    return toIndex;
//                }
//                take = false;
//            }
//            if (size_t(toIndex) >= quiz.arcSize()) return 0;
//            return toIndex;
//        }
//    };
//
//    struct MHS {
//        SlilinQuiz const& quiz;
//        bool notTarget;
//    public:
//        MHS(SlilinQuiz const& quiz)
//                : quiz(quiz), notTarget(false) {
//        }
//        MHS(MHS const& o, TdZddPool& pool)
//                : quiz(o.quiz), notTarget(o.notTarget) {
//        }
//        size_t hashCode() const {
//            return notTarget;
//        }
//        bool equals(MHS const& o) const {
//            return notTarget == o.notTarget;
//        }
//        int down(bool take, int fromIndex, int toIndex) {
//            if (notTarget) return toIndex;
//            for (int i = fromIndex; i < toIndex; ++i) {
//                if (take != quiz.arcTaken(i)) {
//                    notTarget = true;
//                    return toIndex;
//                }
//                take = false;
//            }
//            if (size_t(toIndex) >= quiz.arcSize()) return 0;
//            return toIndex;
//        }
//    };
//
//    MessageHandler mh;
//    mh.begin("removing the target answer") << " ...";
//    dd.subset(RTA(quiz));
//    mh.end(dd.size());
//
//    if (opt_dump3) dump(std::cout, dd, quiz);
//
//    if (!opt_noreport) {
//        auto length = dd.evaluate(NumOfItems());
//        mh << "#node = " << dd.size() << ", #puzzle = " << std::setprecision(6)
//                << dd.pathCount() << ", #hint = [" << length.min << ","
//                << length.max << "]\n";
//    }
//
//    mh.begin("mapping to the hint space") << " ...";
//    int hintSize = (quiz.rows() - 1) * (quiz.cols() - 1);
//    std::vector<std::string> hintName(hintSize);
//    for (int row = quiz.rows() - 2; row >= 0; --row) {
//        for (int col = quiz.cols() - 2; col >= 0; --col) {
//            int i = row * (quiz.cols() - 1) + col;
//            hintName[i] = "(" + std::to_string(row) + ","
//                    + std::to_string(col) + ")";
//        }
//    }
//    TdZdd qdd(hintSize);
//    qdd.subset(MHS(quiz));
//    mh.end(qdd.size());
//}

int main(int argc, char *argv[]) {
    std::string filename;

    for (int i = 1; i < argc; ++i) {
        std::string s = argv[i];
        if (s[0] == '-') {
            if (s == "-0") {
                opt_0 = true;
            }
            else if (s == "-1") {
                opt_1 = true;
            }
            else if (s == "-2") {
                opt_2 = true;
            }
            else if (s == "-3") {
                opt_3 = true;
            }
            else if (s == "-4") {
                opt_4 = true;
            }
            else if (s == "-graph") {
                opt_graph = true;
            }
            else if (s == "-dump") {
                opt_dump = true;
            }
            else if (s == "-dump1") {
                opt_dump1 = true;
            }
            else if (s == "-dump2") {
                opt_dump2 = true;
            }
            else if (s == "-dump3") {
                opt_dump3 = true;
            }
            else if (s == "-m") {
                opt_m = true;
            }
            else if (s == "-o") {
                opt_o = true;
            }
            else if (s == "-n") {
                opt_n = true;
            }
            else if (s == "-s") {
                opt_s = true;
            }
            else if (s == "-d") {
                opt_d = true;
            }
            else if (s == "-a") {
                opt_a = true;
            }
            else if (s == "-l") {
                opt_l = true;
            }
            else if (s == "-r") {
                opt_r = true;
            }
            else if (s == "-csv") {
                opt_csv = true;
            }
            else if (s == "-tex") {
                opt_tex = true;
            }
            else if (s == "-noreport") {
                opt_noreport = true;
            }
            else {
                usage(argv[0]);
                return 1;
            }
        }
        else if (filename.empty()) {
            filename = s;
        }
        else {
            usage(argv[0]);
            return 1;
        }
    }

    MessageHandler m0;
    m0.begin("started");

    MessageHandler m1;
    m1.begin("reading");

    SlilinQuiz quiz;
    if (filename.empty()) {
        m1 << " STDIN ...";
        quiz.readAnswerOrPicture(std::cin);
    }
    else {
        m1 << " \"" << filename << "\" ...";
        std::ifstream fin(filename, std::ios::in);
        if (!fin) {
            m1 << " " << strerror(errno) << "\n";
            return 1;
        }
        quiz.readAnswerOrPicture(fin);
    }

    m1.end();
    if (quiz.arcSize() == 0) {
        m1 << "ERROR: Empty input\n";
        return 1;
    }
    if (opt_l) quiz.rotate(-1);
    if (opt_r) quiz.rotate(1);

    for (int row = 0; row < quiz.rows() - 1; ++row) {
        for (int col = 0; col < quiz.cols() - 1; ++col) {
            bool flag = false;
            switch (quiz.hint(row, col)) {
            case 0:
                flag = opt_0;
                break;
            case 1:
                flag = opt_1;
                break;
            case 2:
                flag = opt_2;
                break;
            case 3:
                flag = opt_3;
                break;
            case 4:
                flag = opt_4;
                break;
            }
            if (flag) quiz.clearHint(row, col);
        }
    }

    if (opt_graph) {
        std::cout << quiz;
        return 0;
    }

    quiz.printQuiz(std::cerr);

    TdZdd dd(quiz.arcSize());
    MessageHandler mh;

    m1.begin("Enumerating") << " ...";
    dd.initialize(quiz.arcSize());

    if (opt_n) {
        class Filter {
            SlilinQuiz const& quiz;
        public:
            Filter(SlilinQuiz const& quiz)
                    : quiz(quiz) {
            }
            Filter(Filter const& o, TdZddPool& pool)
                    : quiz(o.quiz) {
            }
            size_t hashCode() const {
                return 0;
            }
            bool equals(Filter const& o) const {
                return true;
            }
            int down(bool take, int fromIndex, int toIndex) {
                for (int a = fromIndex; a < toIndex; ++a) {
                    if (quiz.arcTaken(a) != take) return 0;
                    take = false;
                }
                return toIndex;
            }
        };
        dd.subset(Filter(quiz));
    }
    else if (opt_m) {
        mh.begin("Degree0or2") << " ...";
        dd.subset(Degree0or2(quiz));
        mh.end(dd.size());

        mh.begin("reduction") << " ...";
        dd.reduce();
        mh.end(dd.size());
    }
    else if (opt_o) {
        mh.begin("DegreeEven") << " ...";
        dd.subset(DegreeEven(quiz));
        mh.end(dd.size());

        mh.begin("reduction") << " ...";
        dd.reduce();
        mh.end(dd.size());
    }
    else {
        mh.begin("Degree0or2") << " ...";
        dd.subset(Degree0or2(quiz));
        mh.end(dd.size());

        mh.begin("reduction") << " ...";
        dd.reduce();
        mh.end(dd.size());

        if (opt_dump1) dump(std::cout, dd, quiz);

        mh.begin("Simpath") << " ...";
        dd.subset(Simpath(quiz));
        mh.end(dd.size());

        mh.begin("reduction") << " ...";
        dd.reduce();
        mh.end(dd.size());
    }

    m1.end();

    if (opt_dump2) dump(std::cout, dd, quiz);

    if (!opt_noreport) {
        auto length = dd.evaluate(NumOfItems());
        m0 << "#node = " << dd.size() << ", #cycle = " << std::setprecision(6)
                << dd.pathCount() << ", length = [" << length.min << ","
                << length.max << "]\n";
    }

    //slilinGen(SlilinGen1(quiz), dd);
    //slilinGen(SlilinGen2(quiz), dd);
    //slilinGen4(SlilinGen3(quiz), dd);
    //slilinGen4(SlilinGen4(quiz), dd);
    slilinGenByBDD(quiz, dd);
    //slilinGenByTdZdd(quiz, dd);

    auto length = dd.evaluate(NumOfItems());
    if (!opt_noreport) {
        mh << "#node = " << dd.size() << ", #puzzle = " << std::setprecision(6)
                << dd.pathCount() << ", #hint = [" << length.min << ","
                << length.max << "]\n";
    }

    auto pathCount = dd.pathCount();
    if (pathCount < 1) {
        mh << "The puzzle has no solution.\n";
        return 1;
    }
    else if (pathCount < 2) {
        mh << "The puzzle is optimum.\n";
    }
    else if (opt_s) {
        mh.begin("selecting minimum-hint puzzles") << " ...";
        dd.evalAndSubset(ULNumOfItems(length.min));
        dd.reduce();
        mh.end(dd.size());

        if (!opt_noreport) {
            auto length = dd.evaluate(NumOfItems());
            mh << "#node = " << dd.size() << ", #puzzle = "
                    << std::setprecision(6) << dd.pathCount() << ", #hint = ["
                    << length.min << "," << length.max << "]\n";
        }
    }
    else {
        mh.begin("selecting minimal-hint puzzles") << " ...";
        dd.subset(MinimalItems(dd));
        dd.reduce();
        mh.end(dd.size());

        if (!opt_noreport) {
            auto length = dd.evaluate(NumOfItems());
            mh << "#node = " << dd.size() << ", #puzzle = "
                    << std::setprecision(6) << dd.pathCount() << ", #hint = ["
                    << length.min << "," << length.max << "]\n";
        }
    }

    Score top;
    int n;
    if (opt_d) {
        mh.begin("evaluating puzzle difficulty") << " ...";
        n = 0;
        for (auto p = dd.begin(); p != dd.end(); ++p) {
            Score s;
            for (auto q = p->begin(); q != p->end(); ++q) {
                int y = *q / (quiz.cols() - 1);
                int x = *q % (quiz.cols() - 1);
                int h = quiz.hint(y, x);
                s.add(h);
            }

            if (n == 0 || top < s) {
                top = s;
                n = 1;
            }
            else if (top == s) {
                ++n;
            }
        }
        mh << " " << top;
        mh.end("(x" + std::to_string(n) + ")");
    }
    else {
        n = dd.pathCount();
    }

    std::srand(std::time(0));
    int r = std::rand() % n + 1;
    int k = 0;
    int nx = quiz.cols() - 1;
    int ny = quiz.rows() - 1;
    if ((opt_csv || opt_tex) && (quiz.rotation() & 1)) std::swap(nx, ny);

    for (auto p = dd.begin(); p != dd.end(); ++p) {
        std::vector<std::vector<int>> hint(ny);
        for (int y = 0; y < ny; ++y) {
            hint[y].resize(nx);
            for (int x = 0; x < nx; ++x) {
                hint[y][x] = -1;
            }
        }

        Score s;
        for (auto q = p->begin(); q != p->end(); ++q) {
            int row = *q / (quiz.cols() - 1);
            int col = *q % (quiz.cols() - 1);
            int v = quiz.hint(row, col);
            s.add(v);

            switch ((opt_csv || opt_tex) ? quiz.rotation() & 3 : 0) {
            case 1:
                hint[col][nx - row - 1] = v;
                break;
            case 2:
                hint[ny - row - 1][nx - col - 1] = v;
                break;
            case 3:
                hint[ny - col - 1][row] = v;
                break;
            default:
                hint[row][col] = v;
                break;
            }
        }

        if (!opt_d || s == top) {
            ++k;
            if (opt_a) std::cout << "Quiz #" << k << ": " << s << "\n";

            if (opt_a || k == r) {
                if (opt_csv) {
                    for (auto line : hint) {
                        bool c = false;
                        for (int v : line) {
                            if (c) std::cout << ",";
                            if (v >= 0) std::cout << v;
                            c = true;
                        }
                        std::cout << "\n";
                    }
                }
                else if (opt_tex) {
                    std::cout << "\\begin{figure}\\centering\n"
                            << "  \\setlength\\unitlength{" << 1.0 / double(nx)
                            << "\\textwidth}\n"
                            << "  \\linethickness{0.07\\unitlength}\n"
                            << "  \\begin{picture}(" << nx << "," << ny
                            << ")(0,0)\n";

                    for (int y = 0; y <= ny; ++y) {
                        std::cout << "    \\multiput(0," << ny - y << ")(1,0){"
                                << nx + 1 << "}{\\circle*{0.2}}\n";
                    }

                    for (int y = 0; y < ny; ++y) {
                        for (int x = 0; x < nx; ++x) {
                            int v = hint[y][x];
                            if (v < 0) continue;
                            std::cout << "    \\put(" << x << "," << ny - y - 1
                                    << "){\\makebox(1,1){" << v << "}}\n";
                        }
                    }

                    for (int y = 0; y < ny; ++y) {
                        for (int x = 0; x < nx; ++x) {
                            if (quiz.northArcTaken(y, x)) {
                                std::cout << "    \\put(" << x << "," << ny - y
                                        << "){\\line(1,0){1}}\n";
                            }
                            if (quiz.westArcTaken(y, x)) {
                                std::cout << "    \\put(" << x << "," << ny - y
                                        << "){\\line(0,-1){1}}\n";
                            }
                        }
                        if (quiz.westArcTaken(y, nx)) {
                            std::cout << "    \\put(" << nx << "," << ny - y
                                    << "){\\line(0,-1){1}}\n";
                        }
                    }
                    for (int x = 0; x < nx; ++x) {
                        if (quiz.northArcTaken(ny, x)) {
                            std::cout << "    \\put(" << x << "," << 0
                                    << "){\\line(1,0){1}}\n";
                        }
                    }

                    std::cout << "  \\end{picture}\n" << "\\end{figure}\n";
                }
                else {
                    quiz.printQuiz(std::cout, hint);
                }
                if (!opt_a) break;
            }
        }

#ifdef DEBUG
        dd.printDebugInfo(std::cerr);
#endif
    }

    m0.end("finished");
    return 0;
}
