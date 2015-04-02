/*
 * Top-Down ZDD Builder
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: TdZddList.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef TDZDDLIST_HPP_
#define TDZDDLIST_HPP_

#include <cassert>
#include <iostream>
#include <stdexcept>

template<class Element>
class TdZddList {
public:
    class const_iterator;

    class iterator {
        Element** ptrptr;

    public:
        iterator() {
        }

        explicit iterator(Element*& ptr)
                : ptrptr(&ptr) {
        }

        iterator(iterator const& o)
                : ptrptr(o.ptrptr) {
        }

        Element*& operator*() const {
            return *ptrptr;
        }

        Element** operator->() const {
            return ptrptr;
        }

        iterator& operator++() {
            ptrptr = &(*ptrptr)->next;
            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ptrptr = &(*ptrptr)->next;
            return tmp;
        }

        bool operator==(iterator const& o) const {
            return ptrptr == o.ptrptr;
        }

        bool operator!=(iterator const& o) const {
            return ptrptr != o.ptrptr;
        }
    };

    class const_iterator {
        Element const* ptr;

    public:
        const_iterator() {
        }

        explicit const_iterator(Element const* ptr)
                : ptr(ptr) {
        }

        explicit const_iterator(iterator const& o)
                : ptr(*o) {
        }

        const_iterator(const_iterator const& o)
                : ptr(o.ptr) {
        }

        Element const* const & operator*() const {
            return ptr;
        }

        Element const* const * operator->() const {
            return &ptr;
        }

        const_iterator& operator++() {
            ptr = ptr->next;
            return *this;
        }

        const_iterator operator++(int) {
            const_iterator tmp = *this;
            ptr = ptr->next;
            return tmp;
        }

        bool operator==(const_iterator const& o) const {
            return ptr == o.ptr;
        }

        bool operator!=(const_iterator const& o) const {
            return ptr != o.ptr;
        }
    };

private:
    Element* front_;
    iterator end_;
    size_t size_;

public:
    TdZddList()
            : front_(0), end_(front_), size_(0) {
    }

    TdZddList(TdZddList const& o)
            : front_(0), end_(front_), size_(0) {
        if (!o.empty()) throw std::runtime_error("Can't copy non-empty object");
    }

    TdZddList& operator=(TdZddList const& o) {
        if (!o.empty()) throw std::runtime_error("Can't copy non-empty object");
        front_ = 0;
        end_ = iterator(front_);
        size_ = 0;
        return *this;
    }

    iterator begin() {
        return iterator(front_);
    }

    const_iterator begin() const {
        return const_iterator(front_);
    }

    iterator end() {
        return end_;
    }

    const_iterator end() const {
        return const_iterator(end_);
    }

    bool empty() const {
        return front_ == 0;
    }

    size_t size() const {
        return size_;
    }

    Element* front() const {
        return front_;
    }

    void push_front(Element* p) {
        assert(p != 0);
        if (front_ == 0) end_ = iterator(p->next);
        p->next = front_;
        front_ = p;
        ++size_;
    }

    Element* pop_front() {
        Element* p = front_;
        if (p == 0) return 0;
        front_ = p->next;
        if (front_ == 0) end_ = iterator(front_);
        --size_;
        return p;
    }

    void push_back(Element* p) {
        assert(p != 0);
        p->next = 0;
        *end_ = p;
        end_ = iterator(p->next);
        ++size_;
    }

    Element* erase(iterator pos) {
        Element* p = *pos;
        if (p == 0) return 0;
        *pos = p->next;
        if (*pos == 0) end_ = pos;
        --size_;
        return p;
    }

    void clear() {
        front_ = 0;
        end_ = iterator(front_);
        size_ = 0;
    }

    void splice(iterator pos, TdZddList& x) {
        assert(this != &x);
        if (x.empty()) return;
        *(x.end_) = *pos;
        *pos = x.front_;
        if (end_ == pos) end_ = x.end_;
        size_ += x.size_;
        x.front_ = 0;
        x.end_ = iterator(x.front_);
        x.size_ = 0;
    }

    void splice(TdZddList& x) {
        assert(this != &x);
        if (x.empty()) return;
        *end_ = x.front_;
        end_ = x.end_;
        size_ += x.size_;
        x.front_ = 0;
        x.end_ = iterator(x.front_);
        x.size_ = 0;
    }

    template<class E>
    friend std::ostream& operator<<(std::ostream& os, TdZddList<E> const& x);
};

template<class Element>
std::ostream& operator<<(std::ostream& os, TdZddList<Element> const& x) {
    os << "[";
#ifdef DEBUG
    Element* const * pp = &x._front;
    size_t n = 0;
#endif
    bool first = true;
    for (Element* p = x._front; p != 0; p = p->next) {
        if (first) {
            first = false;
        }
        else {
            os << ",";
        }
        os << *p;
#ifdef DEBUG
        pp = &p->next;
        ++n;
#endif
    }
#ifdef DEBUG
    if (n != x._size) os << "***ERROR:size()==" << x._size;
    if (pp != &*x._end) os << "***ERROR:end()";
#endif
    return os << "]";
}

#endif /* TDZDDLIST_HPP_ */
