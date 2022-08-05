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

class eqPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        const MintString& a1 = args[1].getValue();
        const MintString& a2 = args[2].getValue();
        if (a1 == a2) {
            interp.returnString(is_active, args[3].getValue());
        } else {
            interp.returnString(is_active, args[4].getValue());
        } // if
    } // operator()
}; // eqPrim

class nePrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        const MintString& a1 = args[1].getValue();
        const MintString& a2 = args[2].getValue();
        if (a1 != a2) {
            interp.returnString(is_active, args[3].getValue());
        } else {
            interp.returnString(is_active, args[4].getValue());
        } // if
    } // operator()
}; // nePrim

class ncPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        const MintString& a1 = args[1].getValue();
        interp.returnInteger(is_active, a1.size());
    } // operator()
}; // ncPrim

class aoPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        const MintString& a1 = args[1].getValue();
        const MintString& a2 = args[2].getValue();
        if (a1 <= a2) {
            interp.returnString(is_active, args[3].getValue());
        } else {
            interp.returnString(is_active, args[4].getValue());
        } // if
    } // operator()
}; // aoPrim

class saPrim : public MintPrim {
public:
    saPrim() : _comma(",") { }
private:
    const MintString _comma;
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        MintString s;
        if (args.size() > 2) {
            // Skip SELF (arg[0]) and end marker (arg[size-1])
            MintArgList::const_iterator first = args.begin();
            std::advance(first, 1);
            MintArgList::const_iterator last  = args.begin();
            std::advance(last, args.size() - 1);
            MintArgList newargs;
            // Note that this introduces sort instability as items are reversed here.
            // We use front_inserter as this is way cheaper if we are using an slist.
            // Fortunately this sort instability does not matter, as mint does
            // not distinguish between string instances.
            std::copy(first, last, std::front_inserter(newargs));
            if (!newargs.empty()) {
                newargs.sort();
                MintArgList::const_iterator i = newargs.begin();
                s.append(i->getValue());
                for (++i; i != newargs.end(); ++i) {
                    s.append(_comma);
                    s.append(i->getValue());
                } // for
            }
        } // if
        interp.returnString(is_active, s);
    } // operator()
}; // saPrim

class siPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        MintString ret;
        const MintString& form = interp.getForm(args[1].getValue());
        const MintString& orig = args[2].getValue();
        for (MintString::const_iterator i = orig.cbegin(); i != orig.cend(); ++i) {
            umintchar_t index = static_cast<umintchar_t>(*i);
            if (index >= form.size()) {
                ret.append(1, static_cast<mintchar_t>(index));
            } else {
                ret.append(1, form[index]);
            } // else
        } // for
        interp.returnString(is_active, ret);
    } // operator()
}; // siPrim

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
