/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: TdZddPool.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef TDZDDPOOL_HPP_
#define TDZDDPOOL_HPP_

#include <cassert>
#include <iostream>
#include <stdexcept>

/**
 * メモリプールの実装.
 * オブジェクトの消滅とともにプールしたメモリを解放する.
 */
class TdZddPool {
    struct Unit {
        Unit* next;
    };

    static size_t const UNIT_SIZE = sizeof(Unit);
    static size_t const BLOCK_UNITS = 1000000 / UNIT_SIZE;
    static size_t const MAX_ELEMENT_UNIS = BLOCK_UNITS / 10;

    Unit* blockList;
    size_t nextUnit;

public:
    TdZddPool()
            : blockList(0), nextUnit(BLOCK_UNITS) {
    }

    TdZddPool(TdZddPool const& o)
            : blockList(0), nextUnit(BLOCK_UNITS) {
        if (o.blockList != 0) throw std::runtime_error(
                "Can't copy non-empty object");
    }

    TdZddPool& operator=(TdZddPool const& o) {
        if (o.blockList != 0) throw std::runtime_error(
                "Can't copy non-empty object");
        clear();
        return *this;
    }

    virtual ~TdZddPool() {
        clear();
    }

    void clear() {
        while (blockList != 0) {
            Unit* block = blockList;
            blockList = blockList->next;
            delete[] block;
        }
        nextUnit = BLOCK_UNITS;
    }

    void splice(TdZddPool& o) {
        if (blockList != 0) {
            Unit** rear = &o.blockList;
            while (*rear != 0) {
                rear = &(*rear)->next;
            }
            *rear = blockList;
        }

        blockList = o.blockList;
        nextUnit = o.nextUnit;

        o.blockList = 0;
        o.nextUnit = BLOCK_UNITS;
    }

private:
    void* allocate_(size_t n) {
        size_t const elementUnits = (n + UNIT_SIZE - 1) / UNIT_SIZE;

        if (elementUnits > MAX_ELEMENT_UNIS) {
            size_t m = elementUnits + 1;
            Unit* block = new Unit[m];
            if (blockList == 0) {
                block->next = 0;
                blockList = block;
            }
            else {
                block->next = blockList->next;
                blockList->next = block;
            }
            return block + 1;
        }

        if (nextUnit + elementUnits > BLOCK_UNITS) {
            Unit* block = new Unit[BLOCK_UNITS];
            block->next = blockList;
            blockList = block;
            nextUnit = 1;
            assert(nextUnit + elementUnits <= BLOCK_UNITS);
        }

        Unit* p = blockList + nextUnit;
        nextUnit += elementUnits;
        return p;
    }

public:
    template<typename T>
    T* allocate(size_t n = 1) {
        return reinterpret_cast<T*>(allocate_(sizeof(T) * n));
    }

    template<typename T>
    class Allocator: public std::allocator<T> {
        TdZddPool& pool;

    public:
        template<typename U>
        struct rebind {
            typedef Allocator<U> other;
        };

        Allocator(TdZddPool& pool) throw ()
                : pool(pool) {
        }

        Allocator(const Allocator& o) throw ()
                : pool(o.pool) {
        }

        template<typename U>
        Allocator(const Allocator<U>& o) throw ()
                : pool(o.pool) {
        }

        ~Allocator() throw () {
        }

        T* allocate(size_t n, const void* = 0) {
            return pool.allocate<T>(n);
        }

        void deallocate(T*, size_t) {
        }
    };

    template<typename T>
    Allocator<T> allocator() {
        return Allocator<T>(*this);
    }
};

#endif /* TDZDDPOOL_HPP_ */
