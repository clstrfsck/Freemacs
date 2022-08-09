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

#include "strprim.h"

// #(==,X,Y,A,B)
// -------------
// Equals.  Compare "X" and "Y" for equality.  To be equal, strings "X" and
// "Y" must be the same length, and have exactly the same characters.
//
// Returns: "A" if "X" and "Y" are equal, "B" otherwise.
class eqPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto &a1 = args.nextArg(argi).getValue();
        auto &a2 = args.nextArg(argi).getValue();
        auto &equal_string = args.nextArg(argi);
        if (a1 == a2) {
            interp.returnString(is_active, equal_string.getValue());
        } else {
            auto &not_equal_string = args.nextArg(argi);
            interp.returnString(is_active, not_equal_string.getValue());
        } // if
    } // operator()
}; // eqPrim

// #(!=,X,Y,A,B)
// -------------
// Not equals.  Convenience function equivalent to #(==,X,Y,B,A).
//
// Returns: "A" if "X" and "Y" are not equal, "B" otherwise.
class nePrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto &a1 = args.nextArg(argi).getValue();
        auto &a2 = args.nextArg(argi).getValue();
        auto &not_equal_string = args.nextArg(argi);
        if (a1 != a2) {
            interp.returnString(is_active, not_equal_string.getValue());
        } else {
            auto &equal_string = args.nextArg(argi);
            interp.returnString(is_active, equal_string.getValue());
        } // if
    } // operator()
}; // nePrim

// #(nc,X)
// -------
// Number of characters.
//
// Returns: The length of string "X" in characters.
class ncPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto &a1 = args.nextArg(argi).getValue();
        interp.returnInteger(is_active, a1.size());
    } // operator()
}; // ncPrim

// #(a?,X,Y,A,B)
// -------------
// Alphabetically ordered.
//
// Returns: "A" if "X" is lexicographically less that "Y", otherwise
// returns "B".
class aoPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto &a1 = args.nextArg(argi).getValue();
        auto &a2 = args.nextArg(argi).getValue();
        auto &less_or_equal_string = args.nextArg(argi);
        if (a1 <= a2) {
            interp.returnString(is_active, less_or_equal_string.getValue());
        } else {
            auto &not_less_or_equal_string = args.nextArg(argi);
            interp.returnString(is_active, not_less_or_equal_string.getValue());
        } // if
    } // operator()
}; // aoPrim

// #(sa,X1,X2,X3,...,Xn)
// ------------------
// Sort ascending.
//
// Returns: Parameters "X1" through "Xn" sorted lexicographically and
// separated by ",".
class saPrim : public MintPrim {
    static const MintString COMMA;
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        MintString s;
        if (args.size() > 2) {
            // Skip SELF (arg[0]) and end marker (arg[size-1])
            auto first = ++args.cbegin();
            auto last  = --args.cend();
            MintArgList newargs(first, last);
            if (!newargs.empty()) {
                newargs.sort();
                auto i = newargs.cbegin();
                s.append(i->getValue());
                for (++i; i != newargs.end(); ++i) {
                    s.append(COMMA);
                    s.append(i->getValue());
                } // for
            }
        } // if
        interp.returnString(is_active, s);
    } // operator()
}; // saPrim

const MintString saPrim::COMMA(",");

// #(si,X,Y)
// ---------
// String index.  Look up each character of literal string "Y" in form
// "X".  The raw ascii value of each character of "Y" is used as an index
// into form "X".  If "X" does not exist, or if the ordinal of the
// character of "Y" is greater than the number of characters in form "X",
// then the character in question is not modified.  Used to translate from
// lower to upper and vice versa.
//
// Returns: Translated string.
class siPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto &a1 = args.nextArg(argi).getValue();
        auto &orig = args.nextArg(argi).getValue();
        MintString ret;
        const auto &form = interp.getForm(a1);
        for (auto i = orig.cbegin(); i != orig.cend(); ++i) {
            umintchar_t index = static_cast<umintchar_t>(*i);
            if (index >= form.size()) {
                ret.append(static_cast<mintchar_t>(index));
            } else {
                ret.append(form[index]);
            } // else
        } // for
        interp.returnString(is_active, ret);
    } // operator()
}; // siPrim

// #(nl)
// ---------
// Newline.  Returns the newline string.
//
// Returns: The newline string.
class nlPrim : public MintPrim {
    static const MintString NEW_LINE;
    void operator()(Mint& interp, bool is_active, const MintArgList&) {
        interp.returnString(is_active, NEW_LINE);
    } // operator()
}; // nlPrim

const MintString nlPrim::NEW_LINE = "\n";

void registerStrPrims(Mint& interp) {
    interp.addPrim("==", std::make_shared<eqPrim>());
    interp.addPrim("!=", std::make_shared<nePrim>());
    interp.addPrim("nc", std::make_shared<ncPrim>());
    interp.addPrim("a?", std::make_shared<aoPrim>());
    interp.addPrim("sa", std::make_shared<saPrim>());
    interp.addPrim("si", std::make_shared<siPrim>());
    interp.addPrim("nl", std::make_shared<nlPrim>());
} // registerStrPrims

// EOF
