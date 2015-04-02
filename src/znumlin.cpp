/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: znumlin.cpp 9 2011-11-16 06:38:04Z iwashita $
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
#include "filter/Degree2.hpp"
#include "filter/NumlinFilter.hpp"
#include "filter/NumOfItems.hpp"
#include "graph/NumlinQuiz.hpp"
#include "util/MessageHandler.hpp"

void usage(char const* cmd) {
    std::cerr << "usage: " << cmd << " <option>... <quiz_file>\n";
    std::cerr << "options\n";
    std::cerr << "  -kansai:   Do not assume using all cells\n";
    std::cerr << "  -graph:    Dump input graph to STDOUT in DOT format\n";
    std::cerr << "  -dump:     Dump result ZDD to STDOUT in DOT format\n";
    std::cerr << "  -noreport: Do not print final report\n";
}

void dump(std::ostream& os, TdZdd const& dd, Graph const& g) {
    dd.dump(os, [g](int i) {return g.arcName(i);});
}

int main(int argc, char *argv[]) {
    std::string filename;
    bool opt_kansai = false;
    bool opt_0 = false;
    bool opt_1 = false;
    bool opt_graph = false;
    bool opt_dump = false;
    bool opt_dump1 = false;
    bool opt_dump2 = false;
    bool opt_dump3 = false;
    bool opt_noreport = false;

    for (int i = 1; i < argc; ++i) {
        std::string s = argv[i];
        if (s[0] == '-') {
            if (s == "-kansai") {
                opt_kansai = true;
            }
            else if (s == "-0") {
                opt_0 = true;
            }
            else if (s == "-1") {
                opt_1 = true;
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

    NumlinQuiz g;
    if (filename.empty()) {
        m1 << " STDIN ...";
        g.readQuiz(std::cin);
    }
    else {
        m1 << " \"" << filename << "\" ...";
        std::ifstream fin(filename, std::ios::in);
        if (!fin) {
            m1 << " " << strerror(errno) << "\n";
            return 1;
        }
        g.readQuiz(fin);
    }

    m1.end();
    if (g.arcSize() == 0) {
        m1 << "ERROR: Empty input\n";
        return 1;
    }

    if (opt_graph) {
        std::cout << g;
        return 0;
    }

    {
        int const n = g.arcSize();
        TdZdd dd(n);
        MessageHandler mh;

        m1.begin("solving") << " ...";
        if (opt_0) {
            if (opt_kansai) {
                mh.begin("NumlinFilter") << " ...";
                dd.subset(NumlinFilter(g));
                mh.end(dd.size());
            }
            else {
                mh.begin("NumlinFilter2") << " ...";
                dd.subset(NumlinFilter2(g));
                mh.end(dd.size());
            }

            if (opt_dump1) dump(std::cout, dd, g);

            mh.begin("reduction") << " ...";
            dd.reduce();
            mh.end(dd.size());
        }
        else if (opt_1) {
            if (opt_kansai) {
                mh.begin("Degree0or2 & NumlinFilter") << " ...";
                Degree0or2 f1(g);
                NumlinFilter f2(g);
                AND<Degree0or2,NumlinFilter> filter(n, f1, f2);
                dd.subset(filter);
                mh.end(dd.size());
            }
            else {
                mh.begin("Degree2 & NumlinFilter2") << " ...";
                Degree2 f1(g);
                NumlinFilter2 f2(g);
                AND<Degree2,NumlinFilter2> filter(n, f1, f2);
                dd.subset(filter);
                mh.end(dd.size());
            }

            if (opt_dump1) dump(std::cout, dd, g);

            mh.begin("reduction") << " ...";
            dd.reduce();
            mh.end(dd.size());
        }
        else {
            if (opt_kansai) {
                mh.begin("Degree0or2") << " ...";
                dd.subset(Degree0or2(g));
                mh.end(dd.size());
            }
            else {
                mh.begin("Degree2") << " ...";
                dd.subset(Degree2(g));
                mh.end(dd.size());
            }

            if (opt_dump1) dump(std::cout, dd, g);

            mh.begin("reduction") << " ...";
            dd.reduce();
            mh.end(dd.size());

            if (opt_dump2) dump(std::cout, dd, g);

            if (opt_kansai) {
                mh.begin("NumlinFilter") << " ...";
                dd.subset(NumlinFilter(g));
                mh.end(dd.size());
            }
            else {
                mh.begin("NumlinFilter2") << " ...";
                dd.subset(NumlinFilter2(g));
                mh.end(dd.size());
            }

            if (opt_dump3) dump(std::cout, dd, g);

            mh.begin("reduction") << " ...";
            dd.reduce();
            mh.end(dd.size());
        }
        m1.end();

        if (opt_dump) dump(std::cout, dd, g);

        if (!opt_noreport) {
            auto length = dd.evaluate(NumOfItems());
            m0 << "#node = " << dd.size() << ", #solution = "
                    << std::setprecision(6) << dd.pathCount() << ", length = ["
                    << length.min << "," << length.max << "]\n";

            for (auto p = dd.begin(); p != dd.end(); ++p) {
                std::set<Graph::ArcNumber> answer(p->begin(), p->end());
                g.printAnswer(std::cout, answer);
            }
        }

#ifdef DEBUG
        dd.printDebugInfo(std::cerr);
#endif
    }

    m0.end("finished");
    return 0;
}
