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


#include "winprim.h"
#include "bufprim.h" // for getCurBuffer
#include "emacswindow.h"

// #(it,X)
// -------
// Input timed.  Reads a character from the keyboard, waiting for "X"
// milliseconds, or 0 if "X" is null.
// Note: Key names are defined elsewhere.
//
// Returns: The name of the key pressed, or "Timeout" if no key pressed.
class itPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto timeout = args.nextArg(argi).getIntValue();
#ifdef _DEBUG
        for (;;) {
            MintString key = getEmacsWindow().getInput(timeout);
            if (key == MintString("F1")) {
                interp.print();
            } else {
                interp.returnString(is_active, key);
                return;
            }
        }
#else
        interp.returnString(is_active, getEmacsWindow().getInput(timeout));
#endif
    } // operator()
}; // itPrim

// #(ow,X)
// -------
// Overwrite screen.  Write literal string "X" on screen at the current
// cursor position.
//
// Returns: null
class owPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        if (!args.empty()) {
            // We are skipping arg 0, and the MA_END arg at the end
            auto start = ++args.cbegin();
            auto end = --args.cend();
            for (auto i = start; i != end; ++i) {
                getEmacsWindow().overwrite(i->getValue());
            } // while
        } // if
        interp.returnNull(is_active);
    } // operator()
}; // owPrim

// #(an,X,Y,Z)
// -----------
// Announce.  Write on the console after the current window.  If "Y" is not
// null, "X" is displayed after the top window, otherwise "X" and "Z"
// are displayed at the bottom of the screen, with the cursor placed after
// "X".
//
// Returns: null
class anPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto left = args.nextArg(argi).getValue();
        auto flag = args.nextArg(argi).getValue();
        auto right = args.nextArg(argi).getValue();
        if (flag.empty()) {
            getEmacsWindow().announce(left, right);
        } else {
            getEmacsWindow().announceWin(left, right);
        } // if
        interp.returnNull(is_active);
    } // operator()
}; // anPrim

// #(xy,X,Y)
// ---------
// Goto X,Y.  Position the cursor at screen column "X", row "Y".  The top
// row is row 0, and the left column is column 0.
//
// Returns: null
class xyPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto x = args.nextArg(argi).getIntValue();
        auto y = args.nextArg(argi).getIntValue();
        getEmacsWindow().gotoxy(x, y);
        interp.returnNull(is_active);
    } // operator()
}; // xyPrim

// #(bl,X,Y)
// ---------
// Bell.  Ring the bell at frequency "X" for "Y" 18ths of a second.  If "X"
// is 0, then the default frequency is used.  If "X" is less than zero then
// a "visual bell" is rung instead.
//
// Returns: null
class blPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto freq = args.nextArg(argi).getIntValue();
        auto millis = args.nextArg(argi).getIntValue() * 56;
        if (freq < 0) {
            getEmacsWindow().visualBell(millis);
        } else {
            getEmacsWindow().audibleBell(freq, millis);
        } // else
        interp.returnNull(is_active);
    } // operator()
}; // blPrim

// #(rd,X)
// -------
// Redisplay the screen.  If "X" is non-null, the screen is completely
// repainted.
// 
// Returns: null
class rdPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        auto argi = args.cbegin();
        auto force = !args.nextArg(argi).empty();
        getEmacsWindow().redisplay(getCurBuffer(), force);
        interp.returnNull(is_active);
    } // operator()
}; // rdPrim

class bsVar : public MintVar {
    MintString getVal(Mint&) const {
        MintString str;
        return stringAppendNum(str, getEmacsWindow().getBotScrollPercent());
    } // getVal
    void setVal(Mint&, const MintString& val) {
        getEmacsWindow().setBotScrollPercent(getStringIntValue(val));
    } // setVal
}; // bsVar

class tsVar : public MintVar {
    MintString getVal(Mint&) const {
        MintString str;
        return stringAppendNum(str, getEmacsWindow().getTopScrollPercent());
    } // getVal
    void setVal(Mint&, const MintString& val) {
        getEmacsWindow().setTopScrollPercent(getStringIntValue(val));
    } // setVal
}; // tsVar

class bcVar : public MintVar {
    MintString getVal(Mint&) const {
        MintString str;
        return stringAppendNum(str, getEmacsWindow().getBackColour());
    } // getVal
    void setVal(Mint&, const MintString& val) {
        getEmacsWindow().setBackColour(getStringIntValue(val));
    } // setVal
}; // bcVar

class fcVar : public MintVar {
    MintString getVal(Mint&) const {
        MintString str;
        return stringAppendNum(str, getEmacsWindow().getForeColour());
    } // getVal
    void setVal(Mint&, const MintString& val) {
        getEmacsWindow().setForeColour(getStringIntValue(val));
    } // setVal
}; // fcVar

class ccVar : public MintVar {
    MintString getVal(Mint&) const {
        MintString str;
        return stringAppendNum(str, getEmacsWindow().getCtrlForeColour());
    } // getVal
    void setVal(Mint&, const MintString& val) {
        getEmacsWindow().setCtrlForeColour(getStringIntValue(val));
    } // setVal
}; // ccVar

class rcVar : public MintVar {
    MintString getVal(Mint&) const {
        MintString str;
        return stringAppendNum(str, getEmacsWindow().getColumns());
    } // getVal
    void setVal(Mint&, const MintString&) {
        /* Value can't be set */
    } // setVal
}; // rcVar

class blVar : public MintVar {
    MintString getVal(Mint&) const {
        MintString str;
        return stringAppendNum(str, getEmacsWindow().getLines());
    } // getVal
    void setVal(Mint&, const MintString&) {
        /* FIXME: should be able to adjust the size of the window */
    } // setVal
}; // blVar

class tlVar : public MintVar {
    MintString getVal(Mint&) const {
        // FIXME: Placeholder for when windows are implemented
        MintString s;
        return stringAppendNum(s, 0);
    } // getVal
    void setVal(Mint&, const MintString&) {
        // FIXME: Does nothing. Placeholder.
    } // setVal
}; // tlVar

class wcVar : public MintVar {
    MintString getVal(Mint&) const {
        MintString s;
        return stringAppendNum(s, getEmacsWindow().getWhitespaceColour());
    } // getVal
    void setVal(Mint&, const MintString& val) {
        getEmacsWindow().setWhitespaceColour(getStringIntValue(val));
    } // setVal
}; // wcVar

class wsVar : public MintVar {
    MintString getVal(Mint&) const {
        return MintString(getEmacsWindow().getWhitespaceDisplay() ? "1" : "0");
    } // getVal
    void setVal(Mint&, const MintString& val) {
        getEmacsWindow().setWhitespaceDisplay(getStringIntValue(val) == 0 ? false : true);
    } // setVal
}; // wsVar

void registerWinPrims(Mint& interp) {
    interp.addPrim("it", std::make_shared<itPrim>());
    interp.addPrim("ow", std::make_shared<owPrim>());
    interp.addPrim("an", std::make_shared<anPrim>());
    interp.addPrim("xy", std::make_shared<xyPrim>());
    interp.addPrim("bl", std::make_shared<blPrim>());
    interp.addPrim("rd", std::make_shared<rdPrim>());

    interp.addVar("bc", std::make_shared<bcVar>());
    interp.addVar("bl", std::make_shared<blVar>());
    interp.addVar("bs", std::make_shared<bsVar>());
    interp.addVar("cc", std::make_shared<ccVar>());
    interp.addVar("fc", std::make_shared<fcVar>());
    interp.addVar("rc", std::make_shared<rcVar>());
    interp.addVar("tl", std::make_shared<tlVar>());
    interp.addVar("ts", std::make_shared<tsVar>());
    interp.addVar("wc", std::make_shared<wcVar>());
    interp.addVar("ws", std::make_shared<wsVar>());
} // registerWinPrims

bool keyWaiting() {
    return getEmacsWindow().keyWaiting();
} // keyWaiting

// EOF
