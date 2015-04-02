/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: Graph.cpp 9 2011-11-16 06:38:04Z iwashita $
 */

#include "Graph.hpp"

#include <stdexcept>

void Graph::reset() {
    elements.clear();
    arcIndex.clear();
    connectedVertices_.clear();
    leavingVertices_.clear();
    theLastArc_.clear();
    vMax = 0;
}

void Graph::setup() {
    int const n = elements.size();
    theLastArc_.resize(vMax + 1);
    for (int i = 0; i < n; ++i) {
        VertexNumberPair vp = elements[i];
        theLastArc_[vp.first] = i;
        theLastArc_[vp.second] = i;
    }

    leavingVertices_.resize(vMax + 1);
    for (VertexNumber v = 1; v <= vMax; ++v) {
        ArcNumber a = theLastArc_[v];
        VertexNumberPair vp = elements[a];
        leavingVertices_[vp.second].push_back(v); // v == vp.second included
    }
    std::vector<bool> left(vMax + 1);
    VertexNumber tail = 1;
    for (VertexNumber v = 1; v <= vMax; ++v) {
        auto& leaving = leavingVertices_[v];
        for (auto p = leaving.begin(); p != leaving.end(); ++p) {
            left[*p] = true;
        }
        leaving.clear();
        while (tail <= vMax && left[tail]) {
            leaving.push_back(tail);
            ++tail;
        }
    }

    initialMate_.resize(vMax + 1);
//    initialDegree_.resize(vMax + 1);
    pathNumber_.resize(vMax + 1);
    for (VertexNumber v = 1; v <= vMax; ++v) {
        initialMate_[v] = v;
//        initialDegree_[v] = 0;
        pathNumber_[v] = 0;
    }
    numPath_ = 0;
}

void Graph::addArc(VertexNumber v1, VertexNumber v2) {
    if (v1 > v2) std::swap(v1, v2);
    VertexNumberPair vp(v1, v2);
    ArcNumber a = elements.size();
    elements.push_back(vp);
    arcIndex.insert(std::make_pair(vp, a));
    if (vMax < v2) {
        vMax = v2;
        connectedVertices_.resize(vMax + 1);
    }
    connectedVertices_[v1].push_back(v2);
    connectedVertices_[v2].push_back(v1);
}

Graph::ArcNumber Graph::getArc(VertexNumber v1, VertexNumber v2) const {
    assert(1 <= v1 && v1 <= vMax);
    assert(1 <= v2 && v2 <= vMax);
    if (v1 > v2) std::swap(v1, v2);
    VertexNumberPair vp(v1, v2);
    auto found = arcIndex.find(vp);
    if (found == arcIndex.end()) throw std::runtime_error(
            "(" + std::to_string(v1) + "," + std::to_string(v2)
                    + "): No such arc");
    return found->second;
}

void Graph::addTargetPath(VertexNumber v1, VertexNumber v2) {
//    if (v1 > v2) std::swap(v1, v2);
    initialMate_[v1] = v2;
    initialMate_[v2] = v1;
//    initialDegree_[v1] = 1;
//    initialDegree_[v2] = 1;
    ++numPath_;
    pathNumber_[v1] = numPath_;
    pathNumber_[v2] = numPath_;
}

void Graph::readAdjacencyList(std::istream& is) {
    reset();
    VertexNumber v1 = 1;
    VertexNumber v2;
    vMax = v1;

    while (is) {
        char c;
        while (isspace(c = is.get())) {
            if (c == '\n') ++v1;
        }
        if (!is) break;
        is.unget();
        is >> v2;
        if (v1 < v2) addArc(v1, v2);
    }

    setup();
}

Graph::VertexNumber Graph::maxFrontierSize() const {
    VertexNumber n = 0;
    for (ArcNumber a = 0; a < elements.size(); ++a) {
        auto vp = elements[a];
        VertexNumber m = vp.second - vp.first + 1;
        if (n < m) n = m;
    }
    return n;
}

std::ostream& operator<<(std::ostream& os, Graph const& g) {
    os << "graph {\n";
    for (auto a = g.elements.begin(); a != g.elements.end(); ++a) {
        os << "  " << a->first << " -- " << a->second << ";\n";
    }
    os << "}\n";
    os.flush();
    return os;
}
