/*
 * CUDD Wrapper
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: cudd_BDD.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef CUDD_BDD_HPP_
#define CUDD_BDD_HPP_

#ifdef __SIZEOF_INT__
#undef SIZEOF_INT
#define SIZEOF_INT __SIZEOF_INT__
#endif

#ifdef __SIZEOF_LONG__
#undef SIZEOF_LONG
#define SIZEOF_LONG __SIZEOF_LONG__
#endif

#ifdef __SIZEOF_POINTER__
#undef SIZEOF_VOID_P
#define SIZEOF_VOID_P __SIZEOF_POINTER__
#endif

#include <utility> // pair
#include <cassert>
#include <cstdio>
#include <cuddInt.h>
#include "ddutil.hpp"

namespace cudd {

class BDD: public ddutil {
    DdNode* dd;
    bool doDeref;

    static DdManager* manager;
    static DdNode* one;
    static DdNode* zero;

    static DdNode* ref(DdNode* dd) {
        assert(dd != 0);
        cuddRef(dd);
        return dd;
    }

    static void deref(DdNode* dd) {
        assert(dd != 0);
        Cudd_RecursiveDeref(manager, dd);
    }

    void update(DdNode* rdd) {
        ref(rdd);
        if (doDeref) Cudd_RecursiveDeref(manager, dd);
        dd = rdd;
        doDeref = true;
    }

    static DdNode* child0(DdNode* f) {
        return Cudd_NotCond(Cudd_E(f), Cudd_IsComplement(f));
    }

    static DdNode* child1(DdNode* f) {
        return Cudd_NotCond(Cudd_T(f), Cudd_IsComplement(f));
    }

    static DdNode* getNode(int i, DdNode *t, DdNode *e) {
        assert(0 <= i);
        assert(t != 0);
        assert(e != 0);

        int pi = index2level(i);
        int pt = index2level(Cudd_NodeReadIndex(t));
        int pe = index2level(Cudd_NodeReadIndex(e));
        if (pi < pt && pi < pe) {
            if (t == e) return e;

            if (Cudd_IsComplement(t)) {
                return Cudd_Not(cuddUniqueInter(manager, i, Cudd_Not(t), Cudd_Not(e)));
            }
            else {
                return cuddUniqueInter(manager, i, t, e);
            }
        }

        DdNode* v = Cudd_bddIthVar(manager, i);
        DdNode* tt = ref(Cudd_bddExistAbstract(manager, t, v));
        DdNode* ee = ref(Cudd_bddExistAbstract(manager, e, v));
        DdNode* r = ref(Cudd_bddIte(manager, v, tt, ee));
        deref(tt);
        deref(ee);
        cuddDeref(r);
        return r;
    }

    BDD(DdNode* dd, bool doRef)
            : dd(doRef ? ref(dd) : dd), doDeref(doRef) {
        assert(dd != 0);
    }

    BDD(int i, DdNode* t, DdNode* e)
            : dd(ref(getNode(i, t, e))), doDeref(true) {
    }

public:
    static const bool isZDD = false;

    BDD()
            : dd(0), doDeref(false) {
    }

    BDD(int val)
            : dd(val ? one : zero), doDeref(false) {
    }

    BDD(int i, const BDD& f, const BDD& g)
            : dd(ref(getNode(i, f.dd, g.dd))), doDeref(false) {
    }

    BDD(const BDD& f)
            : dd(f.doDeref ? ref(f.dd) : f.dd), doDeref(f.doDeref) {
    }

    ~BDD() {
        if (doDeref) Cudd_RecursiveDeref(manager, dd);
    }

    BDD& operator=(const BDD& f) {
        if (doDeref) Cudd_RecursiveDeref(manager, dd);
        if (f.doDeref) {
            dd = ref(f.dd);
            doDeref = true;
        }
        else {
            dd = f.dd;
            doDeref = false;
        }
        return *this;
    }

    size_t id() const {
        return reinterpret_cast<size_t>(dd);
    }

//    void ref() {
//        if (doDeref) return;
//
//        cuddRef(dd);
//        doDeref = true;
//    }

    bool operator==(const BDD& f) const {
        return dd == f.dd;
    }

    bool operator!=(const BDD& f) const {
        return dd != f.dd;
    }

    bool operator<(const BDD& f) const {
        return dd < f.dd;
    }

    BDD operator~() const {
        return BDD(Cudd_Not(dd), true);
    }

    BDD operator&(const BDD& f) const {
        return BDD(Cudd_bddAnd(manager, dd, f.dd), true);
    }

    BDD& operator&=(const BDD& f) {
        update(Cudd_bddAnd(manager, dd, f.dd));
        return *this;
    }

    BDD operator|(const BDD& f) const {
        return BDD(Cudd_bddOr(manager, dd, f.dd), true);
    }

    BDD& operator|=(const BDD& f) {
        update(Cudd_bddOr(manager, dd, f.dd));
        return *this;
    }

    BDD operator-(const BDD& f) const {
        return BDD(Cudd_bddAnd(manager, dd, Cudd_Not(f.dd)), true);
    }

    BDD& operator-=(const BDD& f) {
        update(Cudd_bddAnd(manager, dd, Cudd_Not(f.dd)));
        return *this;
    }

    BDD support() const {
        return BDD(Cudd_Support(manager, dd), true);
    }

    bool hasNodeOfIndex(int index) const {
        return hasNodeOfLevel(index2level(index));
    }

    bool hasNodeOfLevel(int level) const {
        BDD s = support();
        while (!s.isConstant()) {
            int k = s.level();
            if (k == level) return true;
            if (k > level) return false;
            s = s.child1();
        }
        return false;
    }

    BDD constrain(const BDD& f) const {
        return BDD(Cudd_bddConstrain(manager, dd, f.dd), true);
    }

    BDD restrict(const BDD& f) const {
        return BDD(Cudd_bddRestrict(manager, dd, f.dd), true);
    }

    BDD existAbstract(const BDD& cube) const {
        return BDD(Cudd_bddExistAbstract(manager, dd, cube.dd), true);
    }

    BDD existAbstractOthers(const BDD& cube) const {
        return existAbstract(support().existAbstract(cube));
    }

    BDD andAbstract(const BDD& f, const BDD& cube) const {
        return BDD(Cudd_bddAndAbstract(manager, dd, f.dd, cube.dd), true);
    }

    int insertNewVar() const {
        DdNode* newDd =
                isConstant() ? Cudd_bddNewVar(manager) :
                        Cudd_bddNewVarAtLevel(manager, level());
        return Cudd_NodeReadIndex(newDd);
    }

    static int index2level(int i) {
        int k = Cudd_ReadPerm(manager, i);
        if (k < 0) {
            Cudd_bddIthVar(manager, i);
            k = Cudd_ReadPerm(manager, i);
        }
        return k;
    }

    static int level2index(int k) {
        int i = Cudd_ReadInvPerm(manager, k);
        if (i < 0) {
            i = Cudd_NodeReadIndex(Cudd_bddNewVarAtLevel(manager, k));
        }
        return i;
    }

    int index() const {
        return Cudd_NodeReadIndex(dd);
    }

    int level() const {
        return index2level(index());
    }

    BDD child0() const {
        return BDD(child0(dd), true);
    }

    BDD child1() const {
        return BDD(child1(dd), true);
    }

    bool isNull() const {
        return dd == 0;
    }

    bool isTrue() const {
        return dd == one;
    }

    bool isFalse() const {
        return dd == zero;
    }

    bool isConstant() const {
        return dd == 0 || Cudd_IsConstant(dd);
    }

    bool isConst0() const {
        return dd == zero;
    }

    bool isConst1() const {
        return dd == one;
    }

    size_t size() const {
        return Cudd_DagSize(dd);
    }

    double countMinterm(int numVars) const {
        return Cudd_CountMinterm(manager, dd, numVars);
    }

    double pathCount() const {
        return Cudd_CountPathsToNonZero(dd);
    }

    static size_t nodeCount() {
        return Cudd_ReadKeys(manager) - Cudd_ReadDead(manager);
    }

    static size_t peakNodeCount() {
        return Cudd_ReadPeakNodeCount(manager);
    }

    static void reorder() {
        MessageHandler mh;
        mh.begin("reordering") << " <" << nodeCount() << "> ...";
        Cudd_ReduceHeap(manager, CUDD_REORDER_SIFT, 0);
        mh.end(nodeCount());
    }

    template<class Labeler = DefaultLabel>
    void dump(std::ostream& os = std::cout, Labeler labeler =
            DefaultLabel()) const {
        dumpDot(*this, os, labeler);
    }

    friend std::ostream& operator<<(std::ostream& os, BDD const& f) {
        f.dump(os);
        return os;
    }
};

} // namespace cudd

#endif /* CUDD_BDD_HPP_ */
