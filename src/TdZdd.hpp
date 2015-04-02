/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: TdZdd.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef TDZDD_HPP_
#define TDZDD_HPP_

#include "TdZddHash.hpp"
#include "TdZddNode.hpp"
#include "TdZddPool.hpp"

#include <cassert>
#include <sstream>
#include <vector>

/**
 * トップダウン手法によるZDD構築.
 */
class TdZdd {
    int numVars;                        ///< 変数の数.
    std::vector<TdZddNodeList> table;   ///< ノードテーブル本体.
    std::vector<TdZddPool> nodePool;
    std::vector<TdZddPool> newNodePool;
    std::vector<TdZddPool> workDataPool;
    TdZddNode const0;                   ///< 0終端ノード.
    TdZddNode const1;                   ///< 1終端ノード.
    TdZddNode* top;                     ///< 始点へのポインタ.

public:
    TdZdd()
            : numVars(0), table(numVars), nodePool(numVars),
              newNodePool(numVars), workDataPool(numVars),
              const0(numVars, 0, &const1), const1(numVars, &const0, 0),
              top(&const0) {
    }

    TdZdd(int n)
            : numVars(n), table(numVars), nodePool(numVars),
              newNodePool(numVars), workDataPool(numVars),
              const0(numVars, 0, &const1), const1(numVars, &const0, 0),
              top(&const1) {
        for (int i = numVars - 1; i >= 0; --i) {
            top = new (nodePool[i].allocate<TdZddNode>()) TdZddNode(i, top,
                    top);
            table[i].push_back(top);
        }
    }

    TdZdd(TdZdd const& o) {
        operator=(o);
    }

    TdZdd& operator=(TdZdd const& o) {
        class Subsetter {
            TdZdd const& dd;
            TdZddNode* f;
        public:
            Subsetter(TdZdd const& dd)
                    : dd(dd), f(dd.top) {
            }
            Subsetter(Subsetter const& o, TdZddPool& pool)
                    : dd(o.dd), f(o.f) {
            }
            size_t hashCode() const {
                return reinterpret_cast<size_t>(f);
            }
            bool equals(Subsetter const& o) const {
                return f == o.f;
            }
            int down(bool take, int fromIndex, int toIndex) {
                assert(fromIndex <= f->varIndex);
                if (fromIndex == f->varIndex) {
                    f = take ? f->child1 : f->child0;
                }
                else if (take) {
                    return 0;
                }
                while (f->varIndex < toIndex) {
                    f = f->child0;
                }
                if (f == &dd.const0) return 0;
                return f->varIndex;
            }
        };

        initialize(o.numVars);
        subset(Subsetter(o));
        reduce();
        return *this;
    }

    void initialize(int n) {
        numVars = n;
        table.clear();
        table.resize(n);
        nodePool.clear();
        nodePool.resize(n);
        newNodePool.clear();
        newNodePool.resize(n);
        workDataPool.clear();
        workDataPool.resize(n);
        const0 = TdZddNode(n, 0, &const1);
        const1 = TdZddNode(n, &const0, 0);
        top = &const1;
        for (int i = numVars - 1; i >= 0; --i) {
            top = new (nodePool[i].allocate<TdZddNode>()) TdZddNode(i, top,
                    top);
            table[i].push_back(top);
        }
    }

    int variables() const {
        return numVars;
    }

    TdZddNode const* getTop() const {
        return top;
    }

private:
    template<typename Eval, typename T>
    T doEval(Eval& eval) {
        const0.tmp<T>() = eval.value0();
        const1.tmp<T>() = eval.value1();

        for (int i = numVars - 1; i >= 0; --i) {
            for (TdZddNode* f = table[i].front(); f != 0; f = f->next) {
                f->tmp<T>() = eval.value(f->child0->varIndex,
                        f->child0->tmp<T>(), f->child1->varIndex,
                        f->child1->tmp<T>(), f->varIndex);
            }
        }

        return top->tmp<T>();
    }

    template<typename Eval, typename T>
    T doEval(Eval const& eval) {
        const0.tmp<T>() = eval.value0();
        const1.tmp<T>() = eval.value1();

        for (int i = numVars - 1; i >= 0; --i) {
            for (TdZddNode* f = table[i].front(); f != 0; f = f->next) {
                f->tmp<T>() = eval.value(f->child0->varIndex,
                        f->child0->tmp<T>(), f->child1->varIndex,
                        f->child1->tmp<T>(), f->varIndex);
            }
        }

        return top->tmp<T>();
    }

    template<typename Eval, typename T>
    class EvalRunner {
        T* work;

    public:
        EvalRunner()
                : work(0) {
        }

        virtual ~EvalRunner() {
            delete[] work;
        }

        T operator()(TdZdd& dd, Eval& eval) {
            size_t const size = 2 + dd.size();
            work = new T[size];
            T* p = work;

            dd.const0.tmpPtr = p++;
            dd.const1.tmpPtr = p++;

            for (int i = dd.numVars - 1; i >= 0; --i) {
                for (TdZddNode* f = dd.table[i].front(); f != 0; f = f->next) {
                    f->tmpPtr = p++;
                }
            }

            return dd.doEval<Eval,T>(eval);
        }

        T operator()(TdZdd& dd, Eval const& eval) {
            size_t const size = 2 + dd.size();
            work = new T[size];
            T* p = work;

            dd.const0.tmpPtr = p++;
            dd.const1.tmpPtr = p++;

            for (int i = dd.numVars - 1; i >= 0; --i) {
                for (TdZddNode* f = dd.table[i].front(); f != 0; f = f->next) {
                    f->tmpPtr = p++;
                }
            }

            return dd.doEval<Eval,T>(eval);
        }
    };

    template<typename Eval>
    struct EvalRunner<Eval,bool> {
        int operator()(TdZdd& dd, Eval& eval) {
            return dd.doEval<Eval,bool>(eval);
        }
        int operator()(TdZdd& dd, Eval const& eval) {
            return dd.doEval<Eval,bool>(eval);
        }
    };

    template<typename Eval>
    struct EvalRunner<Eval,int> {
        int operator()(TdZdd& dd, Eval& eval) {
            return dd.doEval<Eval,int>(eval);
        }
        int operator()(TdZdd& dd, Eval const& eval) {
            return dd.doEval<Eval,int>(eval);
        }
    };

    template<typename Eval>
    struct EvalRunner<Eval,double> {
        double operator()(TdZdd& dd, Eval& eval) {
            return dd.doEval<Eval,double>(eval);
        }
        double operator()(TdZdd& dd, Eval const& eval) {
            return dd.doEval<Eval,double>(eval);
        }
    };

public:
    template<typename Eval>
    typename Eval::ValueType evaluate(Eval& eval) {
        return EvalRunner<Eval,typename Eval::ValueType>()(*this, eval);
    }

    template<typename Eval>
    typename Eval::ValueType evaluate(Eval const& eval) {
        return EvalRunner<Eval,typename Eval::ValueType>()(*this, eval);
    }

private:
    template<typename Subsetter>
    Subsetter* makeCopy(Subsetter const& state, TdZddPool& pool) {
        return new (pool.allocate<Subsetter>()) Subsetter(state, pool);
    }

    template<typename Subsetter, typename T>
    struct DownCaller {
        int operator()(Subsetter* s, bool take, int fromIndex, TdZddNode* f) {
            return s->down(take, fromIndex, f->varIndex, f->tmp<T>());
        }
    };

    template<typename Subsetter>
    struct DownCaller<Subsetter,void> {
        int operator()(Subsetter* s, bool take, int fromIndex, TdZddNode* f) {
            return s->down(take, fromIndex, f->varIndex);
        }
    };

    template<typename Subsetter, typename T>
    void doSubsetEdge(Subsetter* s, int poolIndex, bool take, int fromIndex,
            TdZddNode* oldToNode, TdZddNode** newToNodePointer) {
        int toIndex = DownCaller<Subsetter,T>()(s, take, fromIndex, oldToNode);
        if (toIndex == 0) {
            oldToNode = &const0;
        }
        else {
            if (toIndex < 0) {
                toIndex = numVars;
            }
            while (oldToNode->varIndex < toIndex) {
                oldToNode = oldToNode->child0;
            }
        }

        int k = oldToNode->varIndex;
        if (k < numVars) {
            if (poolIndex < k) {
                Subsetter* t = makeCopy(*s, workDataPool[k]);
                s->~Subsetter();
                s = t;
            }
            void* mem = newNodePool[k].allocate<TdZddNode>();
            TdZddNode* newToNode = new (mem) TdZddNode(k, s, newToNodePointer);
            oldToNode->nodeList->push_back(newToNode);
        }
        else { // terminal node
            s->~Subsetter();
            *newToNodePointer = oldToNode;
        }
    }

    template<typename Subsetter, typename T>
    void doSubsetNode(Subsetter* s, TdZddNode const* oldNode,
            TdZddNode* newNode) {
        int const i = oldNode->varIndex;
        assert(i == newNode->varIndex);
        assert(i < numVars);

        if (oldNode->child0 == &const0) {
            newNode->child0 = &const0;

            if (oldNode->child1 == &const0) {
                newNode->child1 = &const0;
                s->~Subsetter();
                return;
            }

            doSubsetEdge<Subsetter,T>(s, i, true, i, oldNode->child1,
                    &newNode->child1);
        }
        else if (oldNode->child1 == &const0) {
            newNode->child1 = &const0;
            doSubsetEdge<Subsetter,T>(s, i, false, i, oldNode->child0,
                    &newNode->child0);
        }
        else {
            int j = oldNode->child1->varIndex;
            if (j >= numVars) j = i;
            Subsetter* t = makeCopy(*s, workDataPool[j]);
            doSubsetEdge<Subsetter,T>(s, i, false, i, oldNode->child0,
                    &newNode->child0);
            doSubsetEdge<Subsetter,T>(t, j, true, i, oldNode->child1,
                    &newNode->child1);
        }
    }

    template<typename Subsetter, typename T>
    void doSubset(Subsetter const& state) {
        if (top == &const0 || top == &const1) return;

        for (int i = 0; i < numVars; ++i) {
            for (TdZddNode* f = table[i].front(); f != 0; f = f->next) {
                f->nodeList =
                        new (workDataPool[i].allocate<TdZddNodeList>()) TdZddNodeList();
            }
        }

        Subsetter* s = makeCopy(state, workDataPool[top->varIndex]);
        if (top->varIndex == 0) {
            void* mem = newNodePool[0].allocate<TdZddNode>();
            TdZddNode* newToNode = new (mem) TdZddNode(0, s, &top);
            top->nodeList->push_back(newToNode);
        }
        else {
            doSubsetEdge<Subsetter,T>(s, top->varIndex, false, 0, top, &top);
        }

        //TdZddHashMap<Subsetter const*,TdZddNode*> uniq(1000000);
        TdZddHashMap<Subsetter const*,TdZddNode*> uniq;

        //MessageHandler mh;//TODO
        for (int i = 0; i < numVars; ++i) {
            TdZddNodeList& list = table[i];
            TdZddNodeList newNodeList;
            //mh.begin("Level") << " " << i << " ...";//TODO

            for (TdZddNode* oldNode = list.front(); oldNode != 0; oldNode =
                    oldNode->next) {
                TdZddNodeList* nl = oldNode->nodeList;
                size_t const m = nl->size();

                if (m == 1) {
                    TdZddNode* newNode = nl->front();
                    Subsetter* s = reinterpret_cast<Subsetter*>(newNode->state);
                    *newNode->referrer = newNode;
                    doSubsetNode<Subsetter,T>(s, oldNode, newNode);
                }
                else if (m != 0) {
                    uniq.initialize(m);

                    for (TdZddNodeList::iterator p = nl->begin();
                            p != nl->end();) {
                        TdZddNode* newNode = *p;
                        Subsetter* s =
                                reinterpret_cast<Subsetter*>(newNode->state);

                        TdZddNode* uniqNewNode = uniq.put(s, newNode);
                        *newNode->referrer = uniqNewNode;

                        if (uniqNewNode == newNode) {
                            ++p;
                        }
                        else {
                            nl->erase(p);
                            s->~Subsetter();
                        }
                    }

                    for (TdZddNode* newNode = nl->front(); newNode != 0;
                            newNode = newNode->next) {
                        Subsetter* s =
                                reinterpret_cast<Subsetter*>(newNode->state);
                        doSubsetNode<Subsetter,T>(s, oldNode, newNode);
                    }
                }

                newNodeList.splice(*nl);
                nl->~TdZddNodeList();
            }

            list.clear();
            list.splice(newNodeList);
            nodePool[i].clear();
            nodePool[i].splice(newNodePool[i]);
            workDataPool[i].clear();
            //mh.end(list.size());//TODO
        }
    }

public:
    template<typename Subsetter>
    void subset(Subsetter const& state) {
        doSubset<Subsetter,void>(state);
    }

    template<typename Eval, typename Subsetter>
    void evalAndSubset(Eval const& eval, Subsetter const& state) {
        typedef typename Eval::ValueType ValueType;
        EvalRunner<Eval,ValueType> evalContext;
        evalContext(*this, eval);
        doSubset<Subsetter,ValueType>(state);
    }

    template<typename ES>
    void evalAndSubset(ES const& es) {
        evalAndSubset(es, es);
    }

    void reduce() {
        const0.tmpNodePtr = &const0;
        const1.tmpNodePtr = &const1;

        //TdZddHashSet<TdZddNode*> uniq(1000000);
        TdZddHashSet<TdZddNode*> uniq;

        for (int i = numVars - 1; i >= 0; --i) {
            TdZddNodeList& list = table[i];
            if (list.empty()) continue;

            uniq.initialize(list.size());

            for (TdZddNode* f = list.front(); f != 0; f = f->next) {
                f->child1 = f->child1->tmpNodePtr;
                if (f->child1 == &const0) {
                    f->tmpNodePtr = f->child0->tmpNodePtr;
                }
                else {
                    f->child0 = f->child0->tmpNodePtr;
                    f->tmpNodePtr = uniq.add(f);
                }
            }
        }

        top = top->tmpNodePtr;

        for (int i = numVars - 1; i >= 0; --i) {
            TdZddNodeList& list = table[i];
            if (list.empty()) continue;

            for (TdZddNodeList::iterator p = list.begin(); p != list.end();) {
                if ((*p)->tmpNodePtr == (*p)) {
                    ++p;
                }
                else {
                    list.erase(p);
                }
            }
        }
    }

    size_t size() {
        size_t n = 0;
        for (int i = 0; i < numVars; ++i) {
            n += table[i].size();
        }
        return n;
    }

private:
    struct DeadNodeCounter {
        typedef bool ValueType;
        size_t count;

        DeadNodeCounter()
                : count(0) {
        }

        ValueType value0() const {
            return false;
        }

        ValueType value1() const {
            return true;
        }

        ValueType value(int k0, ValueType v0, int k1, ValueType v1, int k) {
            if (v0 || v1) return true;
            ++count;
            return false;
        }
    };

public:
    size_t deadSize() {
        DeadNodeCounter dnc;
        evaluate(dnc);
        return dnc.count;
    }

private:
    struct PathCounter {
        typedef double ValueType;

        ValueType value0() const {
            return 0.0;
        }

        ValueType value1() const {
            return 1.0;
        }

        ValueType value(int k0, ValueType v0, int k1, ValueType v1,
                int k) const {
            return v0 + v1;
        }
    };

public:
    double pathCount() {
        return evaluate(PathCounter());
    }

    class const_iterator {
        struct Selection {
            TdZddNode const* node;
            bool val;
            bool operator==(Selection const& o) const {
                return node == o.node && val == o.val;
            }
        };

        TdZdd const& dd;
        int cursor;
        std::vector<Selection> path;
        std::vector<int> itemSet;

    public:
        const_iterator(TdZdd const& dd, bool begin)
                : dd(dd), cursor(begin ? -1 : -2), path(), itemSet() {
            if (begin) next(dd.top);
        }

        const_iterator& operator++() {
            next(&dd.const0);
            return *this;
        }

        std::vector<int> const& operator*() const {
            return itemSet;
        }

        std::vector<int> const* operator->() const {
            return &itemSet;
        }

        bool operator==(const_iterator const& o) const {
            return cursor == o.cursor && path == o.path;
        }

        bool operator!=(const_iterator const& o) const {
            return !operator==(o);
        }

    private:
        void next(TdZddNode const* f) {
            for (;;) {
                while (f != &dd.const0) { // down
                    if (f == &dd.const1) return;

                    if (f->child0 != &dd.const0) {
                        cursor = path.size();
                        Selection sel = { f, false };
                        path.push_back(sel);
                        f = f->child0;
                    }
                    else {
                        Selection sel = { f, true };
                        path.push_back(sel);
                        itemSet.push_back(f->varIndex);
                        f = f->child1;
                    }
                }

                for (; cursor >= 0; --cursor) { // up
                    Selection& s = path[cursor];
                    if (s.val == false && s.node->child1 != &dd.const0) {
                        f = s.node;
                        s.val = true;
                        path.resize(cursor + 1);
                        while (!itemSet.empty() && itemSet.back() >= f->varIndex) {
                            itemSet.pop_back();
                        }
                        itemSet.push_back(f->varIndex);
                        f = f->child1;
                        break;
                    }
                }

                if (cursor < 0) { // end() state
                    cursor = -2;
                    path.clear();
                    itemSet.clear();
                    return;
                }
            }
        }
    };

    const_iterator begin() const {
        return const_iterator(*this, true);
    }

    const_iterator end() const {
        return const_iterator(*this, false);
    }

private:
    struct DumpLabeler {
        int operator()(int i) {
            return i;
        }
    };

public:
    void dump(std::ostream& os) const {
        dump(os, DumpLabeler());
    }

    template<typename L>
    void dump(std::ostream& os, L labeler) const {
        os << "digraph {\n";

        int j = 0;
        int k = 0;

        for (int i = 0; i < numVars; ++i) {
            if (table[i].size() == 0) continue;

            for (TdZddNode const* f = table[i].front(); f != 0;
                    f = f->next, ++k) {
                os << "  \"" << f << "\" [label=\"" << labeler(i) << "\"];\n";
                if (f->child0 != &const0) {
                    os << "  \"" << f << "\" -> \"" << f->child0
                            << "\" [style=dashed];\n";
                    if (f->child0 == &const1) ++j;
                }
                if (f->child1 != &const0) {
                    os << "  \"" << f << "\" -> \"" << f->child1
                            << "\" [style=solid];\n";
                    if (f->child1 == &const1) ++j;
                }
            }

            os << "  {rank=same";
            for (TdZddNode const* f = table[i].front(); f != 0;
                    f = f->next, ++k) {
                os << "; \"" << f << "\"";
            }
            os << "}\n";
        }

        if (k == 0) {
            os << "  \"" << &const0 << "\" [shape=square,label=\"0\"];\n";
        }
        else if (j != 0) {
            os << "  \"" << &const1 << "\" [shape=square,label=\"1\"];\n";
        }

        os << "}\n";
        os.flush();
    }

    void selfTest(std::ostream& os) {
        int nodeCount = 0;
        top->tmpInt = 0;

        for (int i = 0; i < numVars; ++i) {
            for (TdZddNode* f = table[i].front(); f != 0; f = f->next) {
                ++nodeCount;
                f->child0->tmpInt = 0;
                f->child1->tmpInt = 0;
            }
        }

        const0.tmpInt = 1;
        const1.tmpInt = 1;

        for (int i = numVars - 1; i >= 0; --i) {
            for (TdZddNode* f = table[i].front(); f != 0; f = f->next) {
                if (f->child0->tmpInt != 1) os << "\nOnly in DAG: "
                        << *f->child0 << "\n";
                if (f->child1->tmpInt != 1) os << "\nOnly in DAG: "
                        << *f->child1 << "\n";
                f->tmpInt = 1;
            }
        }

        if (top->tmpInt != 1) os << "\nOnly in DAG: " << *top << "\n";
        top->tmpInt = 0;

        for (int i = 0; i < numVars; ++i) {
            for (TdZddNode* f = table[i].front(); f != 0; f = f->next) {
                if (f->tmpInt != 0) os << "\nOnly in table: " << *f << "\n";
                f->child0->tmpInt = 0;
                f->child1->tmpInt = 0;
            }
        }
    }

    void printDebugInfo(std::ostream& os) {
        selfTest(os);
    }
};

#endif /* TDZDD_HPP_ */
