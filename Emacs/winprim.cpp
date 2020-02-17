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

class itPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
#ifdef _DEBUG
        for (;;) {
            MintString key = getEmacsWindow().getInput(args[1].getIntValue());
            if (key == MintString("F1")) {
                interp.print();
            } else {
                interp.returnString(is_active, key);
                return;
            }
        }
#else
        interp.returnString(is_active, getEmacsWindow().getInput(args[1].getIntValue()));
#endif
    } // operator()
}; // itPrim

class owPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        MintArgList::const_iterator i = args.begin();
        if (i != args.end()) {
            // We are skipping arg 0
            ++i;
            while (i != args.end()) {
                getEmacsWindow().overwrite(i->getValue());
                ++i;
            } // while
        } // if
        interp.returnNull(is_active);
    } // operator()
}; // owPrim

class anPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        if (args[2].getValue().empty()) {
            getEmacsWindow().announce(args[1].getValue(), args[3].getValue());
        } else {
            getEmacsWindow().announceWin(args[1].getValue(), args[3].getValue());
        } // if
        interp.returnNull(is_active);
    } // operator()
}; // anPrim

class xyPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        getEmacsWindow().gotoxy(args[1].getIntValue(), args[2].getIntValue());
        interp.returnNull(is_active);
    } // operator()
}; // xyPrim

class blPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        int freq = args[1].getIntValue();
        int millis = args[2].getIntValue();
        if (freq < 0) {
            getEmacsWindow().visualBell(millis);
        } else {
            getEmacsWindow().audibleBell(freq, millis);
        } // else
        interp.returnNull(is_active);
    } // operator()
}; // blPrim

class rdPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        getEmacsWindow().redisplay(getCurBuffer(), !args[1].empty());
        interp.returnNull(is_active);
    } // operator()
}; // rdPrim

class bsVar : public MintVar {
    MintString getVal(Mint& interp) const {
        MintString str;
        return stringAppendNum(str, getEmacsWindow().getBotScrollPercent());
    } // getVal
    void setVal(Mint& interp, const MintString& val) {
        getEmacsWindow().setBotScrollPercent(getStringIntValue(val));
    } // setVal
}; // bsVar

class tsVar : public MintVar {
    MintString getVal(Mint& interp) const {
        MintString str;
        return stringAppendNum(str, getEmacsWindow().getTopScrollPercent());
    } // getVal
    void setVal(Mint& interp, const MintString& val) {
        getEmacsWindow().setTopScrollPercent(getStringIntValue(val));
    } // setVal
}; // tsVar

class bcVar : public MintVar {
    MintString getVal(Mint& interp) const {
        MintString str;
        return stringAppendNum(str, getEmacsWindow().getBackColour());
    } // getVal
    void setVal(Mint& interp, const MintString& val) {
        getEmacsWindow().setBackColour(getStringIntValue(val));
    } // setVal
}; // bcVar

class fcVar : public MintVar {
    MintString getVal(Mint& interp) const {
        MintString str;
        return stringAppendNum(str, getEmacsWindow().getForeColour());
    } // getVal
    void setVal(Mint& interp, const MintString& val) {
        getEmacsWindow().setForeColour(getStringIntValue(val));
    } // setVal
}; // fcVar

class ccVar : public MintVar {
    MintString getVal(Mint& interp) const {
        MintString str;
        return stringAppendNum(str, getEmacsWindow().getCtrlForeColour());
    } // getVal
    void setVal(Mint& interp, const MintString& val) {
        getEmacsWindow().setCtrlForeColour(getStringIntValue(val));
    } // setVal
}; // ccVar

class rcVar : public MintVar {
    MintString getVal(Mint& interp) const {
        MintString str;
        return stringAppendNum(str, getEmacsWindow().getColumns());
    } // getVal
    void setVal(Mint& interp, const MintString& val) {
        /* Value can't be set */
    } // setVal
}; // rcVar

class blVar : public MintVar {
    MintString getVal(Mint& interp) const {
        MintString str;
        return stringAppendNum(str, getEmacsWindow().getLines());
    } // getVal
    void setVal(Mint& interp, const MintString& val) {
        /* FIXME: should be able to adjust the size of the window */
    } // setVal
}; // blVar

class tlVar : public MintVar {
    MintString getVal(Mint& interp) const {
        // FIXME: Placeholder for when windows are implemented
        MintString s;
        return stringAppendNum(s, 0);
    } // getVal
    void setVal(Mint& interp, const MintString& val) {
        // FIXME: Does nothing. Placeholder.
    } // setVal
}; // tlVar

class wcVar : public MintVar {
    MintString getVal(Mint& interp) const {
        MintString s;
        return stringAppendNum(s, getEmacsWindow().getWhitespaceColour());
    } // getVal
    void setVal(Mint& interp, const MintString& val) {
        getEmacsWindow().setWhitespaceColour(getStringIntValue(val));
    } // setVal
}; // wcVar

class wsVar : public MintVar {
    MintString getVal(Mint& interp) const {
        return MintString(getEmacsWindow().getWhitespaceDisplay() ? "1" : "0");
    } // getVal
    void setVal(Mint& interp, const MintString& val) {
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
