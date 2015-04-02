/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: TdZddHash.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef TDZDDHASH_HPP_
#define TDZDDHASH_HPP_

#include <cassert>
#include <iostream>
#include <stdexcept>

namespace {

static unsigned long long const primes[] = { (1ULL << 3) + 3, (1ULL << 4) + 3,
        (1ULL << 5) + 5, (1ULL << 6) + 3, (1ULL << 7) + 3, (1ULL << 8) + 7,
        (1ULL << 9) + 9, (1ULL << 10) + 7, (1ULL << 11) + 5, (1ULL << 12) + 3,
        (1ULL << 13) + 17, (1ULL << 14) + 27, (1ULL << 15) + 3, (1ULL << 16)
                + 3, (1ULL << 17) + 29, (1ULL << 18) + 3, (1ULL << 19) + 21,
        (1ULL << 20) + 7, (1ULL << 21) + 17, (1ULL << 22) + 15, (1ULL << 23)
                + 9, (1ULL << 24) + 43, (1ULL << 25) + 35, (1ULL << 26) + 15,
        (1ULL << 27) + 29, (1ULL << 28) + 3, (1ULL << 29) + 11, (1ULL << 30)
                + 3, (1ULL << 31) + 11, (1ULL << 32) + 15, (1ULL << 33) + 17,
        (1ULL << 34) + 25, (1ULL << 35) + 53, (1ULL << 36) + 31, (1ULL << 37)
                + 9, (1ULL << 38) + 7, (1ULL << 39) + 23, (1ULL << 40) + 15 };

inline size_t tableSize(size_t n) {
    n *= 2;

    int lo = 0;
    int hi = sizeof(primes) / sizeof(primes[0]) - 1;

    if (n > primes[hi]) return n + 1;

    int i = (lo + hi) / 2;
    while (lo < hi) {
        if (n <= primes[i]) {
            hi = i;
        }
        else {
            lo = i + 1;
        }
        i = (lo + hi) / 2;
    }

    assert(i == lo && i == hi);
    return primes[i];
}

}

template<class E> class TdZddHashSet;

template<class E>
class TdZddHashSet<E*> {
    static int const MAX_RATIO = 90;

    size_t size_;
    size_t capacity_;
    E** table;
    size_t items_;
    int collisions_;

    TdZddHashSet(TdZddHashSet const&);
    TdZddHashSet& operator=(TdZddHashSet const&);

public:
    TdZddHashSet()
            : size_(0), capacity_(0), table(0), items_(0), collisions_(0) {
    }

    TdZddHashSet(size_t maxItems)
            : size_(tableSize(maxItems)), capacity_(size_),
              table(new E*[capacity_]), items_(0), collisions_(0) {
        for (size_t i = 0; i < size_; ++i) {
            table[i] = 0;
        }
    }

    virtual ~TdZddHashSet() {
        delete[] table;
    }

    void initialize(size_t maxItems) {
        size_ = tableSize(maxItems);
        items_ = 0;
        collisions_ = 0;

        if (size_ > capacity_) {
            capacity_ = size_;
            delete[] table;
            table = new E*[capacity_];
        }

        for (size_t i = 0; i < size_; ++i) {
            table[i] = 0;
        }
    }

    E* add(E* elem) {
        if (items_ * 100 >= size_ * MAX_RATIO) {
            std::cerr << "\nsize = " << size_ << ", items = " << items_
                    << ", collisions = " << collisions_ << "\n";
            throw std::runtime_error("TdZddHashSet: Table overflow");
        }

        size_t col = 0;
        size_t i = elem->hashCode() % size_;
        while (table[i] != 0) {
            if (table[i]->equals(*elem)) return table[i];
            ++col;
            ++i;
            if (i >= size_) i = 0;
        }

        ++items_;
        collisions_ += col;
        return table[i] = elem;
    }

    size_t capacity() const {
        return capacity_;
    }

    size_t size() const {
        return size_;
    }

    size_t items() const {
        return items_;
    }

    size_t collisions() const {
        return collisions_;
    }
};

template<class K, class V> class TdZddHashMap {
    static int const MAX_RATIO = 90;

public:
    struct Entry {
        bool used;
        K key;
        V value;
    };

private:
    size_t size_;
    size_t capacity_;
    Entry* table;
    size_t items_;
    int collisions_;

    size_t hashCode(K const& key) const {
        return static_cast<size_t>(key);
    }

    TdZddHashMap(TdZddHashMap const&);
    TdZddHashMap& operator=(TdZddHashMap const&);

public:
    TdZddHashMap()
            : size_(0), capacity_(0), table(0), items_(0), collisions_(0) {
    }

    TdZddHashMap(size_t maxItems)
            : size_(tableSize(maxItems)), capacity_(size_),
              table(new Entry[capacity_]), items_(0), collisions_(0) {
        for (size_t i = 0; i < size_; ++i) {
            table[i].used = false;
        }
    }

    virtual ~TdZddHashMap() {
        delete[] table;
    }

    void initialize(size_t maxItems) {
        size_ = tableSize(maxItems);
        items_ = 0;
        collisions_ = 0;

        if (size_ > capacity_) {
            capacity_ = size_;
            delete[] table;
            table = new Entry[capacity_];
        }

        for (size_t i = 0; i < size_; ++i) {
            table[i].used = false;
        }
    }

    V& put(K const& key, V const& value) {
        if (items_ * 100 >= size_ * MAX_RATIO) {
            std::cerr << "\nsize = " << size_ << ", items = " << items_
                    << ", collisions = " << collisions_ << "\n";
            throw std::runtime_error("TdZddHashMap: Table overflow");
        }

        size_t col = 0;
        size_t i = hashCode(key) % size_;
        while (table[i].used) {
            if (table[i].key == key) return table[i].value;
            ++col;
            ++i;
            if (i >= size_) i = 0;
        }

        ++items_;
        collisions_ += col;
        table[i].used = true;
        table[i].key = key;
        table[i].value = value;
        return table[i].value;
    }

//    V const& get(K const& key) {
//        static V const notFound;
//        size_t i = hashCode(key) % size_;
//        while (table[i].used) {
//            if (table[i].key == key) return table[i].value;
//            ++i;
//        }
//        return notFound;
//    }

    size_t capacity() const {
        return capacity_;
    }

    size_t size() const {
        return size_;
    }

    size_t items() const {
        return items_;
    }

    size_t collisions() const {
        return collisions_;
    }
};

template<class K, class V>
class TdZddHashMap<K*,V> {
    static int const MAX_RATIO = 90;

public:
    struct Entry {
        K* key;
        V value;
    };

private:
    size_t size_;
    size_t capacity_;
    Entry* table;
    size_t items_;
    int collisions_;

    TdZddHashMap(TdZddHashMap const&);
    TdZddHashMap& operator=(TdZddHashMap const&);

public:
    TdZddHashMap()
            : size_(0), capacity_(0), table(0), items_(0), collisions_(0) {
    }

    TdZddHashMap(size_t maxItems)
            : size_(tableSize(maxItems)), capacity_(size_),
              table(new Entry[capacity_]), items_(0), collisions_(0) {
        for (size_t i = 0; i < size_; ++i) {
            table[i].key = 0;
        }
    }

    virtual ~TdZddHashMap() {
        delete[] table;
    }

    void initialize(size_t maxItems) {
        size_ = tableSize(maxItems);
        items_ = 0;
        collisions_ = 0;

        if (size_ > capacity_) {
            capacity_ = size_;
            delete[] table;
            table = new Entry[capacity_];
        }

        for (size_t i = 0; i < size_; ++i) {
            table[i].key = 0;
        }
    }

    V& put(K* key, V const& value) {
        if (items_ * 100 >= size_ * MAX_RATIO) {
            std::cerr << "\nsize = " << size_ << ", items = " << items_
                    << ", collisions = " << collisions_ << "\n";
            throw std::runtime_error("TdZddHashMap: Table overflow");
        }

        size_t col = 0;
        size_t i = key->hashCode() % size_;
        while (table[i].key != 0) {
            if (table[i].key->equals(*key)) return table[i].value;
            ++col;
            ++i;
            if (i >= size_) i = 0;
        }

        ++items_;
        collisions_ += col;
        table[i].key = key;
        table[i].value = value;
        return table[i].value;
    }

//    V const& get(K* key) {
//        static V const notFound;
//        size_t i = key->hashCode() % size_;
//        while (table[i].key != 0) {
//            if (table[i].key->equals(*key)) return table[i].value;
//            ++i;
//        }
//        return notFound;
//    }

    size_t capacity() const {
        return capacity_;
    }

    size_t size() const {
        return size_;
    }

    size_t items() const {
        return items_;
    }

    size_t collisions() const {
        return collisions_;
    }
};

#endif /* TDZDDHASH_HPP_ */
