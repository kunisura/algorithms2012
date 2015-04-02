/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: zslilin.cpp 9 2011-11-16 06:38:04Z iwashita $
 */

#include <cerrno>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <string>

#include "TdZdd.hpp"

#include "filter/AND.hpp"
#include "filter/Degree0or2.hpp"
#include "filter/Simpath.hpp"
#include "filter/SlilinFilter.hpp"
#include "filter/NumOfItems.hpp"
#include "graph/SlilinQuiz.hpp"
#include "util/MessageHandler.hpp"

void usage(char const* cmd) {
    std::cerr << "usage: " << cmd << " <option>... <quiz_file>\n";
    std::cerr << "options\n";
    std::cerr << "  -m:        Allow multi-cycle link\n";
    std::cerr << "  -graph:    Dump input graph to STDOUT in DOT format\n";
    std::cerr << "  -dump:     Dump result ZDD to STDOUT in DOT format\n";
    std::cerr << "  -noreport: Do not print final report\n";
}

void dump(std::ostream& os, TdZdd const& dd, Graph const& g) {
    dd.dump(os, [g](int i) {return g.arcName(i);});
}

int main(int argc, char *argv[]) {
    std::string filename;
    bool opt_1 = false;
    bool opt_2 = false;
    bool opt_3 = false;
    bool opt_m = false;
    bool opt_graph = false;
    bool opt_dump = false;
    bool opt_dump1 = false;
    bool opt_dump2 = false;
    bool opt_dump3 = false;
    bool opt_noreport = false;

    for (int i = 1; i < argc; ++i) {
        std::string s = argv[i];
        if (s[0] == '-') {
            if (s == "-1") {
                opt_1 = true;
            }
            else if (s == "-2") {
                opt_2 = true;
            }
            else if (s == "-3") {
                opt_3 = true;
            }
            else if (s == "-m") {
                opt_m = true;
            }
            else if (s == "-graph") {
                opt_graph = true;
            }
            else if (s == "-dump") {
                opt_dump = true;
                opt_noreport = true;
            }
            else if (s == "-dump1") {
                opt_dump1 = true;
                opt_noreport = true;
            }
            else if (s == "-dump2") {
                opt_dump2 = true;
                opt_noreport = true;
            }
            else if (s == "-dump3") {
                opt_dump3 = true;
                opt_noreport = true;
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
        quiz.readAnswerOrQuiz(std::cin);
    }
    else {
        m1 << " \"" << filename << "\" ...";
        std::ifstream fin(filename, std::ios::in);
        if (!fin) {
            m1 << " " << strerror(errno) << "\n";
            return 1;
        }
        quiz.readAnswerOrQuiz(fin);
    }

    m1.end();
    if (quiz.arcSize() == 0) {
        m1 << "ERROR: Empty input\n";
        return 1;
    }

    if (opt_graph) {
        std::cout << quiz;
        return 0;
    }

    quiz.printQuiz(std::cerr);

    {
        int const n = quiz.arcSize();
        TdZdd dd(n);
        MessageHandler mh;

        m1.begin("solving") << " ...";

        if (opt_1 && !opt_m) {
            mh.begin("SlilinFilter & Simpath") << " ...";
            SlilinFilter f1(quiz);
            Simpath f2(quiz);
            AND<SlilinFilter,Simpath> filter(n, f1, f2);
            dd.subset(filter);
            mh.end(dd.size());

            if (opt_dump1) dump(std::cout, dd, quiz);
        }
        else {
            if (opt_1 || opt_2) {
                mh.begin("SlilinFilter & Degree0or2") << " ...";
                SlilinFilter f1(quiz);
                Degree0or2 f2(quiz);
                AND<SlilinFilter,Degree0or2> filter(n, f1, f2);
                dd.subset(filter);
                mh.end(dd.size());
            }
            else {
                mh.begin("SlilinFilter") << " ...";
                dd.subset(SlilinFilter(quiz));
                mh.end(dd.size());

                if (opt_3 || opt_m) {
                    size_t dead = dd.deadSize();
                    mh << "\n#alive = " << dd.size() - dead << ", #dead = " << dead
                            << "\n";

                    mh.begin("reduction") << " ...";
                    dd.reduce();
                    mh.end(dd.size());

                    mh.begin("Degree0or2") << " ...";
                    dd.subset(Degree0or2(quiz));
                    mh.end(dd.size());
                }
            }

            if (!opt_m) {
                size_t dead = dd.deadSize();
                mh << "\n#alive = " << dd.size() - dead << ", #dead = " << dead
                        << "\n";

                if (opt_dump1) dump(std::cout, dd, quiz);

                mh.begin("reduction") << " ...";
                dd.reduce();
                mh.end(dd.size());

                if (opt_dump2) dump(std::cout, dd, quiz);

                mh.begin("Simpath") << " ...";
                dd.subset(Simpath(quiz));
                mh.end(dd.size());

                if (opt_dump3) dump(std::cout, dd, quiz);
            }
        }
        size_t dead = dd.deadSize();
        mh << "\n#alive = " << dd.size() - dead << ", #dead = " << dead << "\n";

        mh.begin("reduction") << " ...";
        dd.reduce();
        mh.end(dd.size());

        m1.end();

        if (opt_dump) dump(std::cout, dd, quiz);

        if (!opt_noreport) {
            auto length = dd.evaluate(NumOfItems());
            m0 << "#node = " << dd.size() << ", #solution = "
                    << std::setprecision(6) << dd.pathCount() << ", length = ["
                    << length.min << "," << length.max << "]\n";

            for (auto p = dd.begin(); p != dd.end(); ++p) {
                std::set<Graph::ArcNumber> answer(p->begin(), p->end());
                quiz.printAnswer(std::cout, answer);
            }
        }

#ifdef DEBUG
        dd.printDebugInfo(std::cerr);
#endif
    }

    m0.end("finished");
    return 0;
}
