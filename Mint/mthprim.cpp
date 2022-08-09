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

#include <cctype>

#include "mthprim.h"

// #(bc,X,Y,Z)
// -----------
// Base conversion.  Convert "X" from base "Y" to base "Z".  Bases are as
// follows:
//     'a','c' ASCII - converts a single ASCII character to it's ordinal.
//     'd'     Decimal
//     'o'     Octal
//     'h'     Hexadecimal
//     'b'     Binary
//
// Returns: "X" interpreted according to base "Y" in base "Z".
class bcPrim : public MintPrim {
    int getBase(char sbaseChr, int def) {
        switch (std::toupper(sbaseChr)) {
        case 'A':
        case 'C':
            return 0;
        case 'B':
            return 2;
        case 'O':
            return 8;
        case 'D':
            return 10;
        case 'H':
            return 16;
        default:
            return def;
        } // case
    } // getBase

    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto &arg1 = args.nextArg(argi);
        auto &arg2 = args.nextArg(argi);
        auto &arg3 = args.nextArg(argi);

        mintchar_t sbaseChr = arg2.getValue().empty() ? 'a' : arg2.getValue()[0];
        int sbase = getBase(sbaseChr, 0);
        MintString prefix;
        int num;
        if (sbase != 0) {
            prefix = arg1.getIntPrefix(sbase);
            num = arg1.getIntValue(sbase);
        } else {
            num = arg1.getValue().empty() ? 0 : arg1.getValue()[0];
        } // else
        mintchar_t dbaseChr = arg3.getValue().empty() ? 'd' : arg3.getValue()[0];
        int dbase = getBase(dbaseChr, 10);
        if (dbase != 0) {
            interp.returnString(is_active, stringAppendNum(prefix, num, dbase));
        } else {
            interp.returnString(is_active, MintString(static_cast<mintchar_t>(num)));
        } // else
    } // operator()
}; // bcPrim

class binaryOpPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto &arg1 = args.nextArg(argi);
        auto a2 = args.nextArg(argi).getIntValue(10);
        auto prefix(arg1.getIntPrefix(10));
        auto a1 = arg1.getIntValue(10);
        interp.returnInteger(is_active, prefix, perform(a1, a2));
    } // operator()

    virtual int perform(int a1, int a2) = 0;
}; // binaryOpPrim

class addPrim : public binaryOpPrim {
    int perform(int a1, int a2) {
        return a1 + a2;
    } // perform
}; // addPrim

class subPrim : public binaryOpPrim {
    int perform(int a1, int a2) {
        return a1 - a2;
    } // perform
}; // subPrim

class mulPrim : public binaryOpPrim {
    int perform(int a1, int a2) {
        return a1 * a2;
    } // perform
}; // mulPrim

class divPrim : public binaryOpPrim {
    int perform(int a1, int a2) {
        if (a2 == 0)
            return a1;
        return a1 / a2;
    } // perform
}; // divPrim

class modPrim : public binaryOpPrim {
    int perform(int a1, int a2) {
        if (a2 == 0)
            return a1;
        return a1 % a2;
    } // perform
}; // modPrim

class iorPrim : public binaryOpPrim {
    int perform(int a1, int a2) {
        return a1 | a2;
    } // perform
}; // andPrim

class andPrim : public binaryOpPrim {
    int perform(int a1, int a2) {
        return a1 & a2;
    } // perform
}; // andPrim

class xorPrim : public binaryOpPrim {
    int perform(int a1, int a2) {
        return a1 ^ a2;
    } // perform
}; // xorPrim

// #(g?,X,Y,A,B)
// -------------
// Numeric greater than.
//
// Returns: "A" if "X" is greater than "Y" when interpreted as numbers, "B"
// otherwise.
class gtPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto a1 = args.nextArg(argi).getIntValue(10);
        auto a2 = args.nextArg(argi).getIntValue(10);
        auto &greater_string = args.nextArg(argi);
        if (a1 > a2) {
            interp.returnString(is_active, greater_string.getValue());
        } else {
            auto &not_greater_string = args.nextArg(argi);
            interp.returnString(is_active, not_greater_string.getValue());
        } // if
    } // operator()
}; // gtPrim

void registerMthPrims(Mint& interp) {
    interp.addPrim("bc", std::make_shared<bcPrim>());
    interp.addPrim("++", std::make_shared<addPrim>());
    interp.addPrim("--", std::make_shared<subPrim>());
    interp.addPrim("**", std::make_shared<mulPrim>());
    interp.addPrim("//", std::make_shared<divPrim>());
    interp.addPrim("%%", std::make_shared<modPrim>());
    interp.addPrim("||", std::make_shared<iorPrim>());
    interp.addPrim("&&", std::make_shared<andPrim>());
    interp.addPrim("^^", std::make_shared<xorPrim>());
    interp.addPrim("g?", std::make_shared<gtPrim>());
} // registerMthPrims

// EOF
