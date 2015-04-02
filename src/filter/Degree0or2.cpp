/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: Degree0or2.cpp 9 2011-11-16 06:38:04Z iwashita $
 */

#include "Degree0or2.hpp"

#include <cassert>

int Degree0or2::down(bool take, int fromIndex, int toIndex) {
    Graph::ArcNumber arc = fromIndex;
    Graph::ArcNumber nextArc = toIndex;
    assert(arc < nextArc);
    assert(nextArc <= graph.arcSize());

    for (; arc < nextArc; ++arc) {
        Graph::VertexNumberPair const vp = graph.vertexPair(arc);
        Graph::VertexNumber const v1 = vp.first;
        Graph::VertexNumber const v2 = vp.second;
        assert(v1 <= v2);

        dc.setMinIndex(v1);
        Graph::VertexNumber vmax = dc.maxIndex();

        if (take) {
            if (vmax < v2) {
                dc.setMaxIndex(v2);
                while (vmax < v2) {
                    ++vmax;
                    dc[vmax] = graph.isTerminal(vmax) ? 1 : 0;
                }
            }

            if (++dc[v1] > 2) return 0;
            if (++dc[v2] > 2) return 0;

            take = false;
        }

        if (arc == graph.theLastArc(v1)) {
            if (v1 <= vmax) {
                if (dc[v1] == 1) return 0;
            }
            else {
                if (graph.isTerminal(v1)) return 0;
            }
        }

        if (arc == graph.theLastArc(v2)) {
            if (v2 <= vmax) {
                if (dc[v2] == 1) return 0;
            }
            else {
                if (graph.isTerminal(v2)) return 0;
            }
        }
    }

    if (arc < graph.arcSize()) {
        dc.setMinIndex(graph.vertexPair(arc).first);
    }

    return toIndex;
}
