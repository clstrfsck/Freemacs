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

#include "frmprim.h"

// #(ds,X,Y)
// ---------
// Define string.  A form with name "X" is defined with value "Y". If a
// form named "X" already exists, then it's current value is discarded.
//
// Returns: null
class dsPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto &form_name = args.nextArg(argi).getValue();
        auto &form_value = args.nextArg(argi).getValue();
        interp.setFormValue(form_name, form_value);
        interp.returnNull(is_active);
    } // operator()
}; // dsPrim

// #(gs,X,Y1,Y2,...,Yn)
// --------------------
// Get string.  Form with name "X" is retrieved.  If the form contains any
// parameter markers, P1..Pn, they are replaced with literal strings
// Y1..Yn.
//
// Returns: Form "X" with parameter markers replaced with literal strings
// "Y1".."Yn".
class gsPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        MintString form;
        MintArgList newargs;
        if (args.size() > 2) {
            newargs = args;
            // erase "gs" arg
            newargs.pop_front();
            // Save form name, and erase name
            form = newargs.front().getValue();
            newargs.pop_front();
        } else {
            newargs.push_front(MintArg(MintArg::MA_END));
        } // else
        interp.returnSegString(is_active, interp.getForm(form).get(), newargs);
    } // operator()
}; // gsPrim

// #(go,X,Y)
// ---------
// Get one.  Gets a character from form "X".  If the form cannot be found,
// the null string is returned.  If the form is found, and the form pointer
// is currently at the end of the form, string "Y" is returned in active
// mode.  This is approximately equivalent to the TRAC #(cc,X,Y) primitive,
// only argument markers appear to be returned in MINT.
//
// Returns: The character from the form at the form pointer.
class goPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto &form_name = args.nextArg(argi).getValue();
        auto &error_string = args.nextArg(argi).getValue();
        interp.returnNForm(is_active, form_name, 1, error_string);
    } // operator()
}; // goPrim

// #(gn,X,Y,Z)
// -----------
// Get n.  Gets "Y" characters from form "X".  If form "X" cannot be found,
// then "Z" is returned in active mode.  This differs from the TRAC
// #(cn,...) primitive in that argument markers are returned, and negative
// values of "Y" are not allowed.
//
// Returns: "Y" characters from form "X" at the current form pointer.
class gnPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto &form_name = args.nextArg(argi).getValue();
        auto count = args.nextArg(argi).getIntValue();
        auto &error_string = args.nextArg(argi).getValue();
        interp.returnNForm(is_active, form_name, count, error_string);
    } // operator()
}; // gnPrim

// #(rs,X)
// -------
// Reset string.  Resets the form pointer associated with form "X".
//
// Returns: null
class rsPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto &form_name = args.nextArg(argi).getValue();
        interp.setFormPos(form_name, 0);
        interp.returnNull(is_active);
    } // operator()
}; // rsPrim

// #(fm,X,Y,Z)
// -----------
// First match.  Finds the first match of literal string "Y" in form "X".
// If the string is found, the form pointer is advanced to after the string
// found, and the portion of the form before the matched string is
// returned.  If form "X" cannot be found, null is returned.  If "Y" is
// null, or cannot be found in form "X", then "Z" is returned in active
// mode.
//
// Returns: null if "X" cannot be found, the portion of the form "X" before
// literal string "Y" if form "X" exists, or "Z" in active mode if "Y" is
// null or cannot be found in "X".
class fmPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        bool found;
        auto argi = args.cbegin();
        auto &form_name = args.nextArg(argi).getValue();
        const MintForm& frm = interp.getForm(form_name, &found);
        if (found) {
            const MintString& arg2 = args.nextArg(argi).getValue();
            auto s = frm.cbegin() + frm.getPos();
            auto f = arg2.empty() ? frm.cend() : std::search(s, frm.cend(), arg2.cbegin(), arg2.cend());
            if (f != frm.cend()) {
                interp.setFormPos(form_name, (f - frm.cbegin()) + arg2.size());
                interp.returnString(is_active, MintString(s, f));
            } else {
                auto &item_not_found = args.nextArg(argi).getValue();
                interp.returnString(true, item_not_found);
            } // else
        } else {
            interp.returnNull(is_active);
        } // else
    } // operator()
}; // fmPrim

// #(n?,X,A,B)
// -----------
// Name exists?  Check to see if form given by literal string "X" exists.
//
// Returns: "A" if form exists, "B" otherwise.
class nxPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto &form_name = args.nextArg(argi).getValue();
        auto &exists_string = args.nextArg(argi).getValue();
        bool found;
        interp.getForm(form_name, &found);
        if (found) {
            interp.returnString(is_active, exists_string);
        } else {
            auto &not_exists_string = args.nextArg(argi).getValue();
            interp.returnString(is_active, not_exists_string);
        } // if
    } // operator()
}; // nxPrim

// #(ls,X,Y)
// ---------
// List strings.
//
// Returns: A list of forms separated by literal string "X" that match
// prefix "Y".
class lsPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto &separator = args.nextArg(argi).getValue();
        auto &prefix = args.nextArg(argi).getValue();
        interp.returnFormList(is_active, separator, prefix);
    } // operator()
}; // lsPrim

// #(es,X1,X2,...,Xn)
// ------------------
// Erase strings.  Remove all forms with names "X1", "X2", ..., "Xn".
//
// Returns: null
class esPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        // We ned args[1] and args[args.size()-1] as our bounds
        if (args.size() > 2) {
            auto start = ++args.cbegin();
            auto finish = --args.cend();
            // std::for_each would be good here, but we run into some
            // bustage with standard libraries trying to create
            // references to references.
            for (auto i = start; i != finish; ++i) {
                interp.delForm(i->getValue());
            } // for
        }
        interp.returnNull(is_active);
    } // operator()
}; // esPrim

// #(mp,X,Y1,Y2,...,Yn)
// --------------------
// Make parameters.  Form with name "X" is scanned for occurrences of the
// literal sub-string Y1.  If any are found, they are replaced by special
// parameter markers P1.  This process is repeated for Y2 through Yn,
// replacing with parameter markers P2 through Pn.
// Corresponds to the TRAC primitive #(ss,X,Y1,...,Yn).
//
// Returns: null
class mpPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        if (args.size() > 3) {
            bool found;
            auto argi = args.cbegin();
            auto &form_name = args.nextArg(argi).getValue();
            MintString fv(interp.getForm(form_name, &found));
            if (found) {
                ++argi; // Point to parameter 1
                auto last = --args.cend();
                // FIXME: Dodgy magic numbers and hardcoded types
                umintchar_t insch = 0x80;
                for (MintArgList::const_iterator i = argi; i != last; ++i, ++insch) {
                    const MintString& av = i->getValue();
                    if (!av.empty()) {
                        for (MintString::size_type pos = fv.find(av, 0);
                             pos != MintString::npos; pos = fv.find(av, pos + 1)) {
                            fv.replace(pos, av.size(), static_cast<char>(insch));
                        } // for
                    } // if
                } // for
                interp.setFormValue(form_name, fv);
            } // if
        } // if
        interp.returnNull(is_active);
    } // operator()
}; // mpPrim

// #(hk,X1,X2,X3,...,Xn)
// ---------
// Hook string.  Searches for forms named "X1", through "Xn".  If a form
// that exists is found, evaluates using #(gs,...) using the remainder of
// the arguments.  For example: #(hk,f1,f2,f3,f4) if form "f1" does not
// exist, but form "f2" does, is equivalent to #(gs,f2,f3,f4).
//
// Returns: Expanded version of first of form X1..Xn found, or null if no
// form found.
class hkPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        if (args.size() > 2) {
            auto first = ++args.cbegin();
            auto last = --args.cend();
            for (auto i = first; i != last; ++i) {
                bool found;
                const auto &form_value = interp.getForm(i->getValue(), &found);
                if (found) {
                    MintArgList newargs(i, args.cend());
                    interp.returnSegString(is_active, form_value, newargs);
                    return;
                } // if
            } // for
        } // if
        interp.returnNull(is_active);
    } // operator()
}; // hkPrim


void registerFrmPrims(Mint& interp) {
    interp.addPrim("ds", std::make_shared<dsPrim>());
    interp.addPrim("gs", std::make_shared<gsPrim>());
    interp.addPrim("go", std::make_shared<goPrim>());
    interp.addPrim("gn", std::make_shared<gnPrim>());
    interp.addPrim("rs", std::make_shared<rsPrim>());
    interp.addPrim("fm", std::make_shared<fmPrim>());
    interp.addPrim("n?", std::make_shared<nxPrim>());
    interp.addPrim("ls", std::make_shared<lsPrim>());
    interp.addPrim("es", std::make_shared<esPrim>());
    interp.addPrim("mp", std::make_shared<mpPrim>());
    interp.addPrim("hk", std::make_shared<hkPrim>());
} // registerFrmPrims

// EOF
