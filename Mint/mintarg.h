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
    void push_back(mintchar_t ch) { _value.push_back(ch); }


    void append(mintcount_t n, mintchar_t ch) { _value.append(n, ch); }
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





// std::list is probably not an entirely appropriate choice
// for this.  We really need singly linked container with
// cheap semantics for unlinking part of a chain and
// reversing it.  An slist could be used, but a custom
// container would be able to take advantage of ownership
// and use properties better.
#ifdef USE_ARGS_SLIST
typedef __gnu_cxx::slist<MintArg> MintArgList_base;
#else
typedef std::list<MintArg> MintArgList_base;
#endif
class MintArgList : public MintArgList_base {
public:
    MintArgList() { }

    const MintArg& operator[](mintcount_t n) const {
        // We cheat somewhat, and assume that the last arg
        // is the null argment.  It should be checked in a
        // more rigorous implementation
#ifndef FASTER
        if (size() == 0) {
            throw MintException("Badly terminated argument list found");
        } // if
#endif
        n = std::min(size() - 1, n);
        const_iterator i = begin();
        std::advance(i, n);
        return *i;
    } // getArg
}; // MintArgList

#endif // _MINTARG_H

// EOF
