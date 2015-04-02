/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: TdZddNode.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef TDZDDNODE_HPP_
#define TDZDDNODE_HPP_

#include "TdZddList.hpp"

struct TdZddNode;
typedef TdZddList<TdZddNode> TdZddNodeList;

/**
 * ZDDノード構造.
 * トップダウン構築と一時変数保持のために拡張されたメンバ変数群を持つ.
 */
class TdZddNode {
    friend class TdZdd;
    friend class TdZddList<TdZddNode>;

    TdZddNode* next;            ///< リスト構造における次の要素へのポインタ.

    int varIndex;               ///< このノードに対応する変数番号.
                                /// 非終端ノードでは自身が持つ変数を示した0以上の値を持つ.
                                /// 変数を持たない終端ノードでは(変数番号の最大値+1)の値を持つ.
    union {
        struct {
            TdZddNode* child0;  ///< 0枝.
            TdZddNode* child1;  ///< 1枝.
        };
        struct {
            void* state;          ///< トップダウン構築時の状態識別子.
            TdZddNode** referrer; ///< トップダウン構築時の逆方向リンク.
        };
    };

    TdZddNodeList* nodeList;    ///< トップダウン構築時に自身から派生したノードのリスト.

    union {
        TdZddNode* tmpNodePtr;  ///< ノードポインタ保持用の一時変数.
        void* tmpPtr;           ///< ポインタ保持用の一時変数.
        bool tmpBool;           ///< bool保持用の一時変数.
        int tmpInt;             ///< int保持用の一時変数.
        double tmpDbl;          ///< double保持用の一時変数.
    };

    TdZddNode() {
    }

    explicit TdZddNode(int varIndex)
            : next(0), varIndex(varIndex), child0(0), child1(0) {
    }

    TdZddNode(int varIndex, TdZddNode* child0, TdZddNode* child1)
            : next(0), varIndex(varIndex), child0(child0), child1(child1) {
    }

    TdZddNode(int varIndex, void* state, TdZddNode** referrer)
            : next(0), varIndex(varIndex), state(state), referrer(referrer) {
    }

//private:
//    TdZddNode(TdZddNode const&);
//    TdZddNode& operator=(TdZddNode const&);

public:
    int getIndex() const {
        return varIndex;
    }

    TdZddNode const* getChild0() const {
        return child0;
    }

    TdZddNode const* getChild1() const {
        return child1;
    }

    bool isConstant() const {
        return isConst0() || isConst1();
    }

    bool isConst0() const {
        return child0 == 0;
    }

    bool isConst1() const {
        return child1 == 0;
    }

    TdZddNode const* getConst0() const {
        TdZddNode const* f = this;
        while (!f->isConstant()) {
            if (f->child1->isConstant()) {
                f = f->child1;
            }
            else {
                f = f->child0;
            }
        }
        if (f->isConst0()) return f;
        return f->child0;
    }

    TdZddNode const* getConst1() const {
        TdZddNode const* f = this;
        while (!f->isConstant()) {
            if (f->child1->isConstant()) {
                f = f->child1;
            }
            else {
                f = f->child0;
            }
        }
        if (f->isConst1()) return f;
        return f->child1;
    }

    template<typename T>
    inline T& tmp() {
        return *reinterpret_cast<T*>(tmpPtr);
    }

    size_t hashCode() const {
        return reinterpret_cast<size_t>(child0) * 31
                + reinterpret_cast<size_t>(child1);
    }

    bool equals(TdZddNode const& o) const {
        if (child0 != o.child0) return false;
        if (child1 != o.child1) return false;
        if (varIndex != o.varIndex) return false;
        return true;
    }

    friend std::ostream& operator<<(std::ostream& os, TdZddNode const& o) {
        return os << "Node(" << &o << ")"; //"={" << o.varIndex << "," << o.child0 << "," << o.child1 << "}";
    }
};

template<>
inline bool& TdZddNode::tmp<bool>() {
    return tmpBool;
}

template<>
inline int& TdZddNode::tmp<int>() {
    return tmpInt;
}

template<>
inline double& TdZddNode::tmp<double>() {
    return tmpDbl;
}

#endif /* TDZDDNODE_HPP_ */
