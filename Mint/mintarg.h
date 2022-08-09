/*
 * Copyright 2020 Martin Sandiford
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to: Free Software Foundation
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _MINTARG_H
#define _MINTARG_H

#ifdef USE_ARGS_SLIST
# include <ext/slist>
#else
# include <list>
#endif
#include <tuple>
#include <algorithm>

#include "minttype.h"
#include "mintstring.h"
#ifndef FASTER
# include "mintexception.h"
#endif

#ifdef min
#  undef min
#endif

class MintArg {
public:
    typedef enum { 
        MA_NULL    = 0x80,
        MA_ARG     = 0x01,
        MA_ACTIVE  = 0x82,
        MA_NEUTRAL = 0x83,
        MA_END     = 0x04
    } ArgType;

    explicit MintArg(ArgType arg_type) : _type(arg_type) {
        // Nothing to do
    } // MintArg::MintArg

    // Back insertion sequence model
    typedef MintString::reference reference;
    typedef MintString::const_reference const_reference;

    void append(mintchar_t ch) { _value.append(ch); }
    void append(const mintchar_t* s, mintcount_t l) { _value.append(s, l); }
    void append(const MintString& s) { _value.append(s); }
    template <class II> void append(II first, II last) { _value.append(first, last); }

    ArgType getType() const { return _type; }

    bool isTerm() const { return (static_cast<int>(_type) & 0x80) != 0; }

    const MintString& getValue() const { return _value; }
    bool empty() const { return _value.empty(); }

    int getIntValue(int base = 10) const { return getStringIntValue(_value, base); }
    MintString getIntPrefix(int base = 10) const { return getStringIntPrefix(_value, base); }

    bool operator<(const MintArg& arg) const { return _value < arg._value; }
    bool operator==(const MintArg& arg) const { return _value == arg._value; }

private:
    ArgType _type;
    MintString _value;
}; // MintArg


class MintArgList {
public:
    typedef MintArg value_type;
    typedef std::list<value_type>::size_type size_type;
    typedef std::list<value_type>::iterator iterator;
    typedef std::list<value_type>::const_iterator const_iterator;

    MintArgList() { }
    template <typename II>
    MintArgList(II begin, II end) : _args(begin, end) { }

    size_type size() const {
        return _args.size();
    }

    bool empty() const {
        return _args.empty();
    }

    void sort() {
        _args.sort();
    }

    void clear() {
        _args.clear();
    }

    iterator begin() {
        return _args.begin();
    }

    iterator end() {
        return _args.end();
    }

    const_iterator cbegin() const {
        return _args.cbegin();
    }

    const_iterator cend() const {
        return _args.cend();
    }

    const value_type &front() const {
        return *_args.cbegin();
    }

    void pop_front() {
        _args.erase(_args.cbegin());
    }

    iterator erase(const_iterator start, const_iterator end) {
        return _args.erase(start, end);
    }

    void push_front(const MintArg &arg) {
        _args.push_front(arg);
    }

    void push_front(MintArg &&arg) {
        _args.push_front(arg);
    }

    const value_type &nextArg(const_iterator &i) const {
#ifndef FASTER
        if (i == cend()) {
            throw MintException("Badly terminated argument list found");
        } // if
#endif
        if (i->getType() != MintArg::MA_END) {
            ++i;
        }
        return *i;
    }

//     const MintArg& operator[](mintcount_t n) const {
//         // We cheat somewhat, and assume that the last arg
//         // is the null argment.  It should be checked in a
//         // more rigorous implementation
// #ifndef FASTER
//         if (_args.empty()) {
//             throw MintException("Badly terminated argument list found");
//         } // if
// #endif
//         n = std::min(_args.size() - 1, n);
//         auto i = _args.cbegin();
//         std::advance(i, n);
//         return *i;
//     } // getArg

private:
    std::list<value_type> _args;
}; // MintArgList

#endif // _MINTARG_H

// EOF
