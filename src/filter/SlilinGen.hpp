/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: SlilinGen.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef SLILINGEN_HPP_
#define SLILINGEN_HPP_

#include <string>
#include <vector>
#include <unordered_map>

#if __SIZEOF_POINTER__ == 8
#define B_64
#endif
#include <ZBDD.h>

#include "dd/cudd_ZDD.hpp"
#include "dd/sapporo_ZDD.hpp"
#include "graph/SlilinQuiz.hpp"

class SlilinGen {
protected:
    static int const VALUES = 5;

    SlilinQuiz const& quiz;
    int const numVars;

    int var(int hintIndex, int val) const {
        assert(0 <= hintIndex && hintIndex < quiz.hints());
        assert(0 <= val && val < VALUES);
        //int var = numVars - (hintIndex * VALUES + val);
        int var = hintIndex * VALUES + VALUES - 1 - val;
        assert(0 <= var && var < numVars);
        return var;
    }

public:
    SlilinGen(SlilinQuiz const& quiz)
            : quiz(quiz), numVars(quiz.hints() * VALUES) {
    }

    SlilinQuiz const& getQuiz() const {
        return quiz;
    }

    int rows() const {
        return quiz.rows();
    }

    int cols() const {
        return quiz.cols();
    }

    void printQuiz(std::ostream& os, std::vector<std::vector<int>> hint) const {
        quiz.printQuiz(os, hint);
    }

    int variables() const {
        return numVars;
    }

    int var2hint(int var) const {
        assert(0 <= var && var < numVars);
        return var / VALUES;
    }

    int var2row(int var) const {
        assert(0 <= var && var < numVars);
        return var / VALUES / (quiz.cols() - 1);
    }

    int var2col(int var) const {
        assert(0 <= var && var < numVars);
        return var / VALUES % (quiz.cols() - 1);
    }

    int var2val(int var) const {
        assert(0 <= var && var < numVars);
        return VALUES - 1 - var % VALUES;
    }

    std::string var2name(int var) const {
        std::string row = std::to_string(var2row(var));
        std::string col = std::to_string(var2col(var));
        std::string val = std::to_string(var2val(var));
        return "(" + row + "," + col + ") = " + val;
    }
};

/*
 * ZDDベース
 * 基本アルゴリズム
 * dead変数を使用
 */
class SlilinGen1: public SlilinGen {
    std::vector<ZBDD> varDd;
    ZBDD dead;

public:
    typedef ZBDD ValueType;

    SlilinGen1(SlilinQuiz const& slilin)
            : SlilinGen(slilin), varDd(numVars + 1) {
        BDD_Init(256, -1);
        while (BDD_VarUsed() < numVars + 1) {
            BDD_NewVar();
        }
        for (int i = 0; i < numVars; ++i) {
            varDd[i] = ZBDD(1).Change(numVars - i);
        }
        dead = ZBDD(1).Change(numVars + 1);
    }

    ZBDD value0() const {
        return ZBDD(0);
    }

    ZBDD value1() const {
        ZBDD f(1);
        for (int h = quiz.hints() - 1; h >= 0; --h) {
            f += f * varDd[var(h, 0)];
        }
        return f;
    }

    ZBDD value(int e0, ZBDD f0, int e1, ZBDD f1, int e) const {
        if (quiz.arcTaken(e)) {
            f0 *= dead;
            for (int ee = e1 - 1; ee > e; --ee) {
                if (quiz.arcTaken(ee)) {
                    f1 *= dead;
                    break;
                }
            }
        }
        else {
            f1 *= dead;
            for (int ee = e0 - 1; ee > e; --ee) {
                if (quiz.arcTaken(ee)) {
                    f0 *= dead;
                    break;
                }
            }
        }

        auto& acs = quiz.arcConstraints(e);
        for (auto c = acs.begin(); c != acs.end(); ++c) {
            int h = c->index;
            ZBDD ff = f1;
            for (int v = 4; v >= 0; --v) {
                f1 %= varDd[var(h, v)];
            }
            for (int v = 3; v >= 0; --v) {
                f1 += ff / varDd[var(h, v)] * varDd[var(h, v + 1)];
            }
        }

        for (int ee = std::max(e0, e1) - 1; ee >= e; --ee) {
            auto& acs = quiz.arcConstraints(ee);
            for (auto c = acs.begin(); c != acs.end(); ++c) {
                int h = c->index;
                int max = quiz.initialCount(h);
                int min = max - (3 - c->maxCount);
                for (int v = 0; v < min; ++v) {
                    if (ee < e0) f0 %= varDd[var(h, v)];
                    if (ee < e1) f1 %= varDd[var(h, v)];
                }
                for (int v = max + 1; v < 4; ++v) {
                    if (ee < e0) f0 %= varDd[var(h, v)];
                    if (ee < e1) f1 %= varDd[var(h, v)];
                }
            }
        }

        return f0 + f1;
    }

    sapporo::ZDD getQuizSet(ZBDD f) const {
        return ((f % dead) - (f / dead)) << 1;
    }
};

/*
 * ZDDベース
 * dead変数ではなく２つのZDDで管理
 * ダイナミックに変数を追加
 * バグあり
 */
class SlilinGen2: public SlilinGen {
    std::vector<ZBDD> varDd;

public:
    typedef struct ZBDDs {
        ZBDD all;
        ZBDD dead;
    } ValueType;

    SlilinGen2(SlilinQuiz const& slilin)
            : SlilinGen(slilin), varDd(numVars) {
        BDD_Init(256, -1);
        while (BDD_VarUsed() < numVars) {
            BDD_NewVar();
        }
        for (int i = 0; i < numVars; ++i) {
            varDd[i] = ZBDD(1).Change(numVars - i);
        }
    }

    ZBDDs value0() const {
        return ZBDDs { ZBDD(0), ZBDD(0) };
    }

    ZBDDs value1() const {
        return ZBDDs { ZBDD(1), ZBDD(0) };
    }

    ZBDDs value(int e0, ZBDDs f0, int e1, ZBDDs f1, int e) const {
        //std::cout << "\n***" << e << "\n";
        if (quiz.arcTaken(e)) {
            f0.dead = f0.all;
            for (int ee = e1 - 1; ee > e; --ee) {
                if (quiz.arcTaken(ee)) {
                    f1.dead = f1.all;
                    break;
                }
            }
        }
        else {
            f1.dead = f1.all;
            for (int ee = e0 - 1; ee > e; --ee) {
                if (quiz.arcTaken(ee)) {
                    f0.dead = f0.all;
                    break;
                }
            }
        }

        auto& acs = quiz.arcConstraints(e);
        int hTop = acs.front().index;
        int h0 =
                (f0.all.Top() == 0) ? quiz.hints() :
                        var2hint(numVars - f0.all.Top());
        int h1 =
                (f1.all.Top() == 0) ? quiz.hints() :
                        var2hint(numVars - f1.all.Top());
        for (int h = h0 - 1; h >= hTop; --h) {
            f0.all += f0.all * varDd[var(h, 0)];
            f0.dead += f0.dead * varDd[var(h, 0)];
        }
        for (int h = h1 - 1; h >= hTop; --h) {
            f1.all += f1.all * varDd[var(h, 0)];
            f1.dead += f1.dead * varDd[var(h, 0)];
        }

        for (auto c = acs.begin(); c != acs.end(); ++c) {
            int h = c->index;
            ZBDDs ff = f1;
            for (int v = 4; v >= 0; --v) {
                ZBDD x = varDd[var(h, v)];
                f1.all %= x;
                f1.dead %= x;
            }
            for (int v = 3; v >= 0; --v) {
                ZBDD x = varDd[var(h, v)];
                ZBDD y = varDd[var(h, v + 1)];
                f1.all += ff.all / x * y;
                f1.dead += ff.dead / x * y;
            }
        }

        for (int ee = std::max(e0, e1) - 1; ee >= e; --ee) {
            auto& acs = quiz.arcConstraints(ee);
            for (auto c = acs.begin(); c != acs.end(); ++c) {
                if (c->maxCount == 3) { // the last arc
                    int h = c->index;
                    int target = quiz.initialCount(h);
                    for (int v = 0; v <= 4; ++v) {
                        if (v == target) continue;
                        ZBDD x = varDd[var(h, v)];
                        if (ee <= e0) {
                            f0.all %= x;
                            f0.dead %= x;
                        }
                        if (ee <= e1) {
                            f1.all %= x;
                            f1.dead %= x;
                        }
                    }
                }
            }
        }
        //sapporo::ZDD(f1.all).dump(std::cout); //TODO

        return ZBDDs { f0.all + f1.all, f0.dead + f1.dead };
    }

    sapporo::ZDD getQuizSet(ZBDDs f) const {
        return f.all - f.dead;
    }
};

/*
 * BDDベース
 * 正解以外のサイクルを持つヒント集合を計算
 * ダイナミックに変数を追加
 * 省メモリだが遅いみたい
 */
class SlilinGen3: public SlilinGen {
    typedef cudd::ZDD ZDD;

public:
    struct ValueType {
        ZDD zdd;
        int topHint;
    };

    SlilinGen3(SlilinQuiz const& slilin)
            : SlilinGen(slilin) {
    }

    ValueType value0() const {
        return {ZDD(0), quiz.hints()};
    }

    ValueType value1() const {
        return {ZDD(1), quiz.hints()};
    }

    ValueType value(int e0, ValueType v0, int e1, ValueType v1, int e) const {
        ZDD f0 = v0.zdd;
        ZDD f1 = v1.zdd;
        //std::cerr << "\n*** " << e0 << "," << e1 << " -> " << e << "\n";//TODO
        //f1.dump(std::cerr); //TODO
        auto& acs = quiz.arcConstraints(e);
        int topHint = acs.front().index;
        if (v0.topHint < topHint) topHint = v0.topHint;
        if (v1.topHint < topHint) topHint = v1.topHint;
        for (int h = v0.topHint - 1; h >= topHint; --h) {
            f0 = ZDD(var(h, 0), f0, f0);
        }
        for (int h = v1.topHint - 1; h >= topHint; --h) {
            //std::cerr << var(h, 0) << "\n";//TODO
            f1 = ZDD(var(h, 0), f1, f1);
        }
        //f1.dump(std::cerr); //TODO

        for (auto c = acs.rbegin(); c != acs.rend(); ++c) {
            int h = c->index;
            f1 = f1.shift(-1, var(h, 3), 4);
        }
        //f1.dump(std::cerr); //TODO

        for (int ee = std::max(e0, e1) - 1; ee >= e; --ee) {
            auto& acs = quiz.arcConstraints(ee);
            for (auto c = acs.begin(); c != acs.end(); ++c) {
                int h = c->index;
                int max = quiz.initialCount(h);
                int min = max - (3 - c->maxCount);
                int vals[5];
                for (int v = 0; v <= 4; ++v) {
                    vals[4 - v] = (min <= v && v <= max) ? 2 : 0;
                }
                if (ee < e0) f0 = f0.assign(vals, var(h, 4), 5);
                if (ee < e1) f1 = f1.assign(vals, var(h, 4), 5);
            }
        }
        //f1.dump(std::cerr); //TODO

        return {f0 | f1, topHint};
    }

    ZDD getQuizSet(ValueType v) const {
        ZDD g(1);
        for (int h = quiz.hints() - 1; h >= 0; --h) {
            int c = quiz.initialCount(h);
            g = ZDD(var(h, c), g, g);
        }
        return g - v.zdd;
    }
};

/*
 * ZDDベース
 * 正解以外のサイクルを持つヒント集合を計算
 */
class SlilinGen4: public SlilinGen {
    std::vector<ZBDD> varDd;

public:
    typedef ZBDD ValueType;

    SlilinGen4(SlilinQuiz const& slilin)
            : SlilinGen(slilin), varDd(numVars) {
        BDD_Init(256, -1);
        while (BDD_VarUsed() < numVars) {
            BDD_NewVar();
        }
        for (int i = 0; i < numVars; ++i) {
            varDd[i] = ZBDD(1).Change(numVars - i);
        }
    }

    ZBDD value0() const {
        return ZBDD(0);
    }

    ZBDD value1() const {
        ZBDD f(1);
        for (int h = quiz.hints() - 1; h >= 0; --h) {
            f += f * varDd[var(h, 0)];
        }
        return f;
    }

    ZBDD value(int e0, ZBDD f0, int e1, ZBDD f1, int e) const {
        auto& acs = quiz.arcConstraints(e);
        for (auto c = acs.begin(); c != acs.end(); ++c) {
            int h = c->index;
            ZBDD ff = f1;
            for (int v = 4; v >= 0; --v) {
                f1 %= varDd[var(h, v)];
            }
            for (int v = 3; v >= 0; --v) {
                f1 += ff / varDd[var(h, v)] * varDd[var(h, v + 1)];
            }
        }

        for (int ee = std::max(e0, e1) - 1; ee >= e; --ee) {
            auto& acs = quiz.arcConstraints(ee);
            for (auto c = acs.begin(); c != acs.end(); ++c) {
                int h = c->index;
                int max = quiz.initialCount(h);
                int min = max - (3 - c->maxCount);
                for (int v = 0; v < min; ++v) {
                    if (ee < e0) f0 %= varDd[var(h, v)];
                    if (ee < e1) f1 %= varDd[var(h, v)];
                }
                for (int v = max + 1; v < 4; ++v) {
                    if (ee < e0) f0 %= varDd[var(h, v)];
                    if (ee < e1) f1 %= varDd[var(h, v)];
                }
            }
        }

        return f0 + f1;
    }

    sapporo::ZDD getQuizSet(ZBDD f) const {
        ZBDD g(1);
        for (int h = quiz.hints() - 1; h >= 0; --h) {
            int v = quiz.initialCount(h);
            g += g * varDd[var(h, v)];
        }
        return g - f;
    }
};

#endif /* SLILINGEN_HPP_ */
