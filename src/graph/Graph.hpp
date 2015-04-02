/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: Graph.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef GRAPH_HPP_
#define GRAPH_HPP_

#include <cassert>
#include <iostream>
#include <map>
#include <vector>

class Graph {
public:
    typedef unsigned int VertexNumber;
    typedef unsigned int ArcNumber;
    typedef unsigned int PathNumber;
    typedef std::pair<VertexNumber,VertexNumber> VertexNumberPair;

private:
    std::vector<VertexNumberPair> elements;
    std::map<VertexNumberPair,ArcNumber> arcIndex;
    std::vector<std::vector<VertexNumber>> connectedVertices_;
    std::vector<std::vector<VertexNumber>> leavingVertices_;
    VertexNumber vMax;
    std::vector<ArcNumber> theLastArc_;
    std::vector<VertexNumber> initialMate_;
    std::vector<PathNumber> pathNumber_;
    PathNumber numPath_;

public:
    VertexNumber vertexSize() const {
        return vMax;
    }

    ArcNumber arcSize() const {
        return elements.size();
    }

    VertexNumberPair const& vertexPair(ArcNumber a) const {
        assert(0 <= a && size_t(a) < elements.size());
        return elements[a];
    }

    VertexNumber nextVertex(ArcNumber a) const {
        assert(0 <= a && size_t(a) < elements.size());
        if (size_t(a + 1) >= elements.size()) return vMax + 1;
        return elements[a + 1].first;
    }

    std::vector<VertexNumber> const& connectedVertices(VertexNumber v) const {
        assert(1 <= v && v <= vMax);
        return connectedVertices_[v];
    }

    std::vector<VertexNumber> const& leavingVertices(VertexNumber v) const {
        assert(1 <= v && v <= vMax);
        return leavingVertices_[v];
    }

    std::string arcName(ArcNumber a) const {
        if (a < 0 || elements.size() <= size_t(a)) return "-";
        VertexNumberPair const& vp = elements[a];
        return std::to_string(vp.first) + "," + std::to_string(vp.second);
    }

    ArcNumber theLastArc(VertexNumber v) const {
        assert(1 <= v && v <= vMax);
        return theLastArc_[v];
    }

    VertexNumber initialMate(VertexNumber v) const {
        assert(1 <= v && v <= vMax);
        return initialMate_[v];
    }

    PathNumber pathNumber(VertexNumber v) const {
        assert(1 <= v && v <= vMax);
        return pathNumber_[v];
    }

    bool isTerminal(VertexNumber v) const {
        assert(1 <= v && v <= vMax);
        return pathNumber_[v] != 0;
    }

    PathNumber numPath() const {
        return numPath_;
    }

    void addArc(VertexNumber v1, VertexNumber v2);
    ArcNumber getArc(VertexNumber v1, VertexNumber v2) const;
    void addTargetPath(VertexNumber v1, VertexNumber v2);
    void readAdjacencyList(std::istream& is);
    VertexNumber maxFrontierSize() const;

    friend std::ostream& operator<<(std::ostream& os, Graph const& g);

protected:
    void reset();
    void setup();
};

#endif /* GRAPH_HPP_ */
