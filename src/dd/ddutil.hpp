/*
 * DD Base
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: ddutil.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef DDUTIL_HPP_
#define DDUTIL_HPP_

#include <cassert>
#include <iostream>
#include <list>
#include <set>
#include <vector>

#include "util/MessageHandler.hpp" //TODO
//#include "TdZdd.hpp" //TODO
//#include "filter/DDBuilder.hpp" //TODO
//#include "filter/DDFilter.hpp" //TODO
//#include "filter/UnivAbstract.hpp" //TODO

class ddutil {
    template<class DD, class Labeler>
    static void dumpDot(std::set<std::size_t>& mark, const DD& f,
            std::ostream& os, Labeler labeler) {
        if (mark.find(f.id()) != mark.end()) return;
        mark.insert(f.id());
        if (f.isConstant()) {
            if (f.isConst0()) {
                os << "  \"" << f.id() << "\" [shape=square,label=\"0\"];\n";
            }
            else if (f.isConst1()) {
                os << "  \"" << f.id() << "\" [shape=square,label=\"1\"];\n";
            }
            else {
                os << "  \"" << f.id() << "\" [shape=square,label=\"?\"];\n";
            }
        }
        else {
            DD f0 = f.child0();
            DD f1 = f.child1();
            if (!f0.isConst0()) {
                dumpDot(mark, f0, os, labeler);
            }
            if (!f1.isConst0()) {
                dumpDot(mark, f1, os, labeler);
            }
            os << "  \"" << f.id() << "\" [label=\"" << labeler(f.index())
                    << "\"];\n";
            if (!f0.isConst0()) {
                os << "  \"" << f.id() << "\" -> \"" << f0.id()
                        << "\" [style=dashed];\n";
            }
            if (!f1.isConst0()) {
                os << "  \"" << f.id() << "\" -> \"" << f1.id()
                        << "\" [style=solid];\n";
            }
        }
    }

    template<class DD>
    static DD nHot(int n, std::vector<DD> const& f, int i, int j) { //TODO use cache
        if (i >= j) return (n == 0) ? DD(1) : DD(0);
        DD const& x = f[i];
        DD g = ~x & nHot(n, f, i + 1, j);
        if (n >= 1) g |= x & nHot(n - 1, f, i + 1, j);
        return g;
    }

protected:
    struct DefaultLabel {
        int operator()(int i) const {
            return i;
        }
    };

    template<class DD, class Labeler = DefaultLabel>
    static void dumpDot(const DD& f, std::ostream& os = std::cout,
            Labeler labeler = DefaultLabel()) {
        std::set<std::size_t> mark;
        os << "digraph {\n";
        dumpDot(mark, f, os, labeler);
        os << "}\n";
        os.flush();
    }

    template<class DD>
    static std::vector<int> getSupportIndices(DD f) {
        DD s = f.support();
        std::vector<int> indices;
        indices.reserve(s.size());
        while (!s.isConstant()) {
            indices.push_back(s.index());
            s = s.child1();
        }
        return indices;
    }

    template<class DD>
    static std::vector<int> getSupportLevels(DD f) {
        DD s = f.support();
        std::vector<int> levels;
        levels.reserve(s.size());
        while (!s.isConstant()) {
            levels.push_back(s.level());
            s = s.child1();
        }
        return levels;
    }

public:
    template<class DD>
    static DD andAll(std::vector<DD> const& f) {
        DD g(1);
        for (int i = f.size() - 1; i >= 0; --i) {
            g &= f[i];
        }
        return g;
    }

    template<class DD>
    static DD andAbstractAll(std::vector<DD> const& f, DD const& cube) {
        struct Entry {
            DD func;
            DD abst;
            bool target;
            bool mark;
        };

        MessageHandler mh; //TODO
        mh.begin("andAbstract") << " ..."; //TODO
        std::list<Entry> funcList;
        DD allAbst(1);
        DD result(1);
        for (int i = f.size() - 1; i >= 0; --i) {
            DD s = f[i].support();
            DD t = s.existAbstractOthers(cube);
            if (t.isConstant()) {
                result &= f[i];
            }
            else {
                funcList.push_back(Entry { f[i], t });
                allAbst &= t;
            }
        }

        std::vector<DD> ff;
        ff.reserve(f.size());

        while (!funcList.empty()) {
            int numTargets = funcList.size();
            MessageHandler mh2; //TODO
            mh2.begin(std::to_string(numTargets)) << ": "; //TODO

            for (auto p = funcList.begin(); p != funcList.end(); ++p) {
                p->target = true;
            }

            for (DD a = allAbst; !a.isConstant(); a = a.child1()) {
                int v = a.index();
                int n = 0;
                for (auto p = funcList.begin(); p != funcList.end(); ++p) {
                    if (p->abst.hasNodeOfIndex(v)) {
                        if (++n >= numTargets) break;
                        p->mark = true;
                    }
                    else {
                        p->mark = false;
                    }
                }

                assert(n >= 1);
                if (n < numTargets) {
                    numTargets = n;
                    for (auto p = funcList.begin(); p != funcList.end(); ++p) {
                        p->target = p->mark;
                    }
                }
            }

            assert(numTargets >= 1);
            DD targetAbst(1);
            DD nonTargetAbst(1);
            for (auto p = funcList.begin(); p != funcList.end(); ++p) {
                if (p->target) {
                    targetAbst &= p->abst;
                }
                else {
                    nonTargetAbst &= p->abst;
                }
            }
            targetAbst = targetAbst.existAbstract(nonTargetAbst);

            ff.clear();
            for (auto p = funcList.begin(); p != funcList.end();) {
                if (p->target) {
                    ff.push_back(p->func);
                    p = funcList.erase(p);
                }
                else {
                    ++p;
                }
            }

            assert(!ff.empty());
            DD g(1);
            for (int i = ff.size() - 1; i >= 1; --i) {
                mh2 << "<" << ff[i].size() << ">"; //TODO
                g &= ff[i];
            }
            mh2 << "<" << ff[0].size() << ">/" << targetAbst.size() - 1
                    << " ..."; //TODO

            g = g.andAbstract(ff[0], targetAbst);
//            {
//                for (int i = ff.size() - 1; i >= 0; --i) {
//                    g &= ff[i];
//                }
//                //g = g.existAbstract(targetAbst);
//
//                mh2 << "(";
//                int n = getSupportLevels(g).back() + 1;
//                TdZdd dd(n);
//                dd.subset(DDFilter<DD>(~g));
//                dd.reduce();
//
//                auto targets = getSupportLevels(targetAbst);
//                for (auto p = targets.rbegin(); p != targets.rend(); ++p) {
//                    mh2 << "*";
//                    if (*p >= n) continue;
//                    dd.subset(UnivAbstract2(dd, *p));
//                    dd.reduce();
//                }
//
//                mh2 << ")";
//                g = ~dd.evaluate(DDBuilder<DD>());
//            }

            allAbst = allAbst.existAbstract(targetAbst);
            DD t = g.support().existAbstractOthers(allAbst);
            if (t.isConstant()) {
                result &= g;
            }
            else {
                funcList.push_front(Entry { g, t });
                //funcList.push_back(Entry { g, t });
            }
            mh2.end(g.size()); //TODO
            //DD::reorder(); //TODO
        }

        mh.end(); //TODO
        return result;
    }

    template<class DD>
    static DD nHot(int n, std::vector<DD> const& f) {
        return nHot(n, f, 0, f.size());
    }
};

#endif /* DDUTIL_HPP_ */
