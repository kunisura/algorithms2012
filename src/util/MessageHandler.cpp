/*
 * Message Handler with CPU Status Report
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: MessageHandler.cpp 9 2011-11-16 06:38:04Z iwashita $
 */

#include "MessageHandler.hpp"

#include <cctype>
#include <cstdio>

using std::string;
using std::ostream;
using std::locale;

int MessageHandler::level = 0;
int MessageHandler::lineno = 1;
bool MessageHandler::running = false;
MessageHandler::Buf* MessageHandler::lastUser = 0;

static string capitalize(string const& s) {
    string t = s;
    if (t.size() >= 1) {
        t[0] = toupper(s[0]);
    }
    return t;
}

MessageHandler::Buf::Buf(MessageHandler& mh, ostream& os) :
    mh(mh), os(os) {
}

void MessageHandler::Buf::imbue(locale const& loc) {
    os.imbue(loc);
}

int MessageHandler::Buf::overflow(int c) {
    if (lastUser != this) {
        if (running) {
            running = false;
            ++lineno;
            os.put('\n');
        }
        lastUser = this;
    }

    if (c == EOF) return EOF;

    if (!running) {
        if (isspace(c)) return c;
        for (int i = mh.indent; i > 0; --i) {
            os.put(' ');
        }
    }

    if (c == '\n') {
        running = false;
        ++lineno;
    }
    else {
        running = true;
    }

    os.put(c);
    return c;
}

MessageHandler::MessageHandler(ostream& os) :
    ostream(new Buf(*this, os)) {
    flags(os.flags());
    precision(os.precision());
    width(os.width());
    indent = level * INDENT_SIZE;
}

MessageHandler::~MessageHandler() {
    if (!name.empty()) end("aborted");
    delete rdbuf();
}

MessageHandler& MessageHandler::begin(string const& s) {
    if (!name.empty()) end("aborted");
    name = s.empty() ? "level-" + level : s;
    indent = level * INDENT_SIZE;
    *this << capitalize(name);
    indent = ++level * INDENT_SIZE;
    beginLine = lineno;
    initialUsage.update();
    return *this;
}

MessageHandler& MessageHandler::end(string const& type) {
    if (name.empty()) return *this;
    indent = --level * INDENT_SIZE;
    ResourceUsage u = ResourceUsage() - initialUsage;
    if (beginLine == lineno) {
        *this << " " << type;
    }
    else {
        *this << "\n" << capitalize(type);
        if (type == "done") *this << " " << name;
    }
    *this << " in " << u << ".\n";
    name = "";
    return *this;
}

MessageHandler& MessageHandler::end(size_t n) {
    return end("<" + std::to_string(n) + ">");
}
