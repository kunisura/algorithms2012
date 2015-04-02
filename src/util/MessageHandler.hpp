/*
 * Message Handler with CPU Status Report
 * Hiroaki Iwashita <iwashita@erato.ist.hokudai.ac.jp>
 * Copyright (c) 2011 Japan Science and Technology Agency
 * $Id: MessageHandler.hpp 9 2011-11-16 06:38:04Z iwashita $
 */

#ifndef MESSAGEHANDLER_HPP_
#define MESSAGEHANDLER_HPP_

#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>

#include "ResourceUsage.hpp"

class MessageHandler: public std::ostream {
    class Buf: public std::streambuf {
        MessageHandler& mh;
        std::ostream& os;
    public:
        Buf(MessageHandler& mh, std::ostream& os);
    protected:
        virtual void imbue(std::locale const& loc);
        virtual int overflow(int c);
    };

    static int const INDENT_SIZE = 2;
    static int level;
    static int lineno;
    static bool running;
    static Buf* lastUser;

    std::string name;
    int indent;
    int beginLine;
    ResourceUsage initialUsage;

public:
    MessageHandler(std::ostream& os = std::cerr);
    virtual ~MessageHandler();
    MessageHandler& begin(std::string const& name = "started");
    MessageHandler& end(std::string const& type = "done");
    MessageHandler& end(size_t n);
};

#endif /* MESSAGEHANDLER_HPP_ */
