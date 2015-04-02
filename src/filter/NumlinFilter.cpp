/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: NumlinFilter.cpp 9 2011-11-16 06:38:04Z iwashita $
 */

#include "NumlinFilter.hpp"

#include <cassert>

int NumlinFilter::down(bool take, int fromIndex, int toIndex) {
    Graph::ArcNumber arc = fromIndex;
    Graph::ArcNumber nextArc = toIndex;
    assert(arc < nextArc);
    assert(nextArc <= graph.arcSize());

    for (; arc < nextArc; ++arc) {
        auto const vp = graph.vertexPair(arc);
        auto const v1 = vp.first;
        auto const v2 = vp.second;
        assert(v1 <= v2);
        bool const terminal1 = graph.isTerminal(v1);
        bool const terminal2 = graph.isTerminal(v2);

        mate.setMinIndex(v1);
        Graph::VertexNumber vmax = mate.maxIndex();

        if (take) {
            if (vmax < v2) {
                mate.setMaxIndex(v2);
                for (auto v = vmax + 1; v <= v2; ++v) {
                    mate[v] = v;
                }
                vmax = v2;
            }

            auto const w1 = mate[v1];
            auto const w2 = mate[v2];
            assert(w1 <= vmax);
            assert(w2 <= vmax);

            if (w1 == 0 || w2 == 0) return 0;
            if (terminal1 && w1 != v1) return 0;
            if (terminal2 && w2 != v2) return 0;

            if (w1 == v2) { // loop found
                assert(w2 == v1);
                return 0;
            }

            int const n1 = graph.pathNumber(w1);
            int const n2 = graph.pathNumber(w2);
            if (n1 != 0 && n2 != 0) {
                if (n1 != n2) return 0;

                if (--pathCount == 0) {
                    for (auto v = v1 + 1; v <= vmax; ++v) {
                        if (v == v2) continue;
                        auto const w = mate[v];
                        if (graph.isTerminal(v)) {
                            if (w == v) return 0;
                        }
                        else {
                            if (w != 0 && w != v) return 0;
                        }
                    }
                    return -1;
                }
            }

            if (w1 != v1) mate[v1] = 0;
            if (w2 != v2) mate[v2] = 0;
            if (w1 >= v1) mate[w1] = w2;
            if (w2 >= v1) mate[w2] = w1;

            take = false;
        }

        if (arc == graph.theLastArc(v1)) {
            if (v1 <= vmax) {
                auto const w1 = mate[v1];
                if (terminal1) {
                    if (w1 == v1) return 0;
                }
                else {
                    if (w1 != 0 && w1 != v1) return 0;
                }
            }
            else {
                if (terminal1) return 0;
            }
        }

        if (arc == graph.theLastArc(v2)) {
            if (v2 <= vmax) {
                auto const w2 = mate[v2];
                if (terminal2) {
                    if (w2 == v2) return 0;
                }
                else {
                    if (w2 != 0 && w2 != v2) return 0;
                }
            }
            else {
                if (terminal2) return 0;
            }
        }
    }

    if (arc < graph.arcSize()) {
        mate.setMinIndex(graph.vertexPair(arc).first);
    }
    return toIndex;
}

int NumlinFilter2::down(bool take, int fromIndex, int toIndex) {
    Graph::ArcNumber arc = fromIndex;
    Graph::ArcNumber nextArc = toIndex;
    assert(arc < nextArc);
    assert(nextArc <= graph.arcSize());

    for (; arc < nextArc; ++arc) {
        auto const vp = graph.vertexPair(arc);
        auto const v1 = vp.first;
        auto const v2 = vp.second;
        assert(v1 <= v2);
        bool const terminal1 = graph.isTerminal(v1);
        bool const terminal2 = graph.isTerminal(v2);

        mate.setMinIndex(v1);
        Graph::VertexNumber vmax = mate.maxIndex();

        if (take) {
            if (vmax < v2) {
                mate.setMaxIndex(v2);
                for (auto v = vmax + 1; v <= v2; ++v) {
                    mate[v] = v;
                }
                vmax = v2;
            }

            auto const w1 = mate[v1];
            auto const w2 = mate[v2];
            assert(w1 <= vmax);
            assert(w2 <= vmax);

            if (w1 == 0 || w2 == 0) return 0;
            if (terminal1 && w1 != v1) return 0;
            if (terminal2 && w2 != v2) return 0;

            if (w1 == v2) { // loop found
                assert(w2 == v1);
                return 0;
            }

            int const n1 = graph.pathNumber(w1);
            int const n2 = graph.pathNumber(w2);
            if (n1 != 0 && n2 != 0) {
                if (n1 != n2) return 0;

                if (--pathCount == 0) {
                    if (vmax < graph.vertexSize() - 1) return 0; // all cells must be used
                    for (auto v = v1 + 1; v <= vmax; ++v) {
                        if (v == v2) continue;
                        auto const w = mate[v];
                        if (graph.isTerminal(v)) {
                            if (w == v) return 0;
                        }
                        else {
                            if (w != 0) return 0; // cell must be used
                        }
                    }
                    return -1;
                }
            }

            if (w1 != v1) mate[v1] = 0;
            if (w2 != v2) mate[v2] = 0;
            if (w1 >= v1) mate[w1] = w2;
            if (w2 >= v1) mate[w2] = w1;

            take = false;
        }

        if (arc == graph.theLastArc(v1)) {
            if (v1 <= vmax) {
                auto const w1 = mate[v1];
                if (terminal1) {
                    if (w1 == v1) return 0;
                }
                else {
                    if (w1 != 0) return 0; // cell must be used
                }
            }
            else {
                if (terminal1) return 0;
            }
        }

        if (arc == graph.theLastArc(v2)) {
            if (v2 <= vmax) {
                auto const w2 = mate[v2];
                if (terminal2) {
                    if (w2 == v2) return 0;
                }
                else {
                    if (w2 != 0) return 0; // cell must be used
                }
            }
            else {
                if (terminal2) return 0;
            }
        }
    }

    if (arc < graph.arcSize()) {
        mate.setMinIndex(graph.vertexPair(arc).first);
    }
    return toIndex;
}
