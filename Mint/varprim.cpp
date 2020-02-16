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

#include "varprim.h"

class lvPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        interp.returnString(is_active, interp.getVar(args[1].getValue()));
    } // operator()
}; // lvPrim

class svPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        interp.setVar(args[1].getValue(), args[2].getValue());
        interp.returnNull(is_active);
    } // operator()
}; // svPrim

class vnVar : public MintVar {
    MintString getVal(Mint& /*interp*/) const {
        return MintString("2.0a");
    } // getVal
    void setVal(Mint& /*interp*/, const MintString& /*val*/) {
        // This variable is not settable
    } // setVal
}; // vnVar

class asVar : public MintVar {
    MintString getVal(Mint& interp) const {
        MintString s;
        return stringAppendNum(s, interp.getIdleMax());
    } // getVal
    void setVal(Mint& interp, const MintString& val) {
        interp.setIdleMax(getStringIntValue(val));
    } // setVal
}; // asVar

void registerVarPrims(Mint& interp) {
    interp.addPrim("lv", new lvPrim);
    interp.addPrim("sv", new svPrim);

    interp.addVar("as", new asVar);
    interp.addVar("vn", new vnVar);
} // registerVarPrims

// EOF
