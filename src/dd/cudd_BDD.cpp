/*
 * CUDD Wrapper
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: cudd_BDD.cpp 9 2011-11-16 06:38:04Z iwashita $
 */

#include "cudd_BDD.hpp"

namespace cudd {

DdManager* BDD::manager = Cudd_Init(0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0);
DdNode* BDD::one = Cudd_ReadOne(manager);
DdNode* BDD::zero = Cudd_ReadLogicZero(manager);

} // namespace cudd
