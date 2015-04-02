/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: SlilinFilter.cpp 9 2011-11-16 06:38:04Z iwashita $
 */

#include "SlilinFilter.hpp"

#include <cassert>

int SlilinFilter::down(bool take, int fromIndex, int toIndex) {
    Graph::ArcNumber arc = fromIndex;
    Graph::ArcNumber nextArc = toIndex;
    assert(arc < nextArc);
    assert(nextArc <= graph.arcSize());
    SlilinQuiz::HintIndex smax = sc.maxIndex();

    for (auto a = arc; a < nextArc; ++a) {
        auto cnstr = graph.arcConstraints(a);

        if (take) {
            for (auto p = cnstr.begin(); p != cnstr.end(); ++p) {
                SlilinQuiz::HintIndex i = p->index;
                if (smax < i) {
                    sc.setMaxIndex(i);
                    for (auto j = smax + 1; j <= i; ++j) {
                        sc[j] = graph.initialCount(j);
                    }
                    smax = i;
                }
                int c = --sc[i];
                if (c < 0 || c > p->maxCount) return 0;
                if (p->maxCount == 0) sc.setMinIndex(i + 1);
            }

            take = false;
        }
        else {
            for (auto p = cnstr.begin(); p != cnstr.end(); ++p) {
                SlilinQuiz::HintIndex i = p->index;
                int c = (i <= smax) ? sc[i] : graph.initialCount(i);
                if (c > p->maxCount) return 0;
                if (p->maxCount == 0) sc.setMinIndex(i + 1);
            }
        }
    }

    return toIndex;
}
