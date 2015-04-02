/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: Simpath.cpp 9 2011-11-16 06:38:04Z iwashita $
 */

#include "Simpath.hpp"

#include <cassert>

int Simpath::down(bool take, int fromIndex, int toIndex) {
    Graph::ArcNumber arc = fromIndex;
    Graph::ArcNumber nextArc = toIndex;
    assert(arc < nextArc);
    assert(nextArc <= graph.arcSize());

    for (; arc < nextArc; ++arc) {
        Graph::VertexNumberPair const vp = graph.vertexPair(arc);
        Graph::VertexNumber const v1 = vp.first;
        Graph::VertexNumber const v2 = vp.second;
        assert(v1 <= v2);

        mate.setMinIndex(v1);
        Graph::VertexNumber vmax = mate.maxIndex();

        if (take) {
            if (vmax < v2) {
                mate.setMaxIndex(v2);
                for (Graph::VertexNumber v = vmax + 1; v <= v2; ++v) {
                    Graph::VertexNumber w = graph.initialMate(v);
                    if (w <= vmax) { // implies w != v (v is the terminal)
                        // find the current mate of v
                        for (Graph::VertexNumber x = v1; x <= vmax; ++x) {
                            if (mate[x] == v) {
                                w = x;
                                break;
                            }
                        }
                    }
                    mate[v] = w;
                }
                vmax = v2;
            }

            Graph::VertexNumber w1 = mate[v1];
            Graph::VertexNumber w2 = mate[v2];

            if (w1 == 0 || w2 == 0) return 0;

            if (w1 == v2) { // loop found
                assert(w2 == v1);
                for (Graph::VertexNumber v = v1 + 1; v <= vmax; ++v) {
                    if (v == v2) continue;
                    Graph::VertexNumber const w = mate[v];
                    if (w != 0 && w != v) return 0;
                }
                return -1; // jump to 1 terminal
            }

            if (w1 != v1) mate[v1] = 0;
            if (w2 != v2) mate[v2] = 0;
            if (w1 <= vmax) mate[w1] = w2;
            if (w2 <= vmax) mate[w2] = w1;

            take = false;
        }

        if (arc == graph.theLastArc(v1)) {
            Graph::VertexNumber const w1 =
                    (v1 <= vmax) ? mate[v1] : graph.initialMate(v1);
            if (w1 != 0 && w1 != v1) return 0;
        }

        if (arc == graph.theLastArc(v2)) {
            Graph::VertexNumber const w2 =
                    (v2 <= vmax) ? mate[v2] : graph.initialMate(v2);
            if (w2 != 0 && w2 != v2) return 0;
        }
    }

    if (arc < graph.arcSize()) {
        mate.setMinIndex(graph.vertexPair(arc).first);
    }
    return toIndex;
}
