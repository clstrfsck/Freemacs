#include <cctype>

#include "mthprim.h"

class bcPrim : public MintPrim {
    int getBase(char sbaseChr, int def) {
        switch (std::toupper(sbaseChr)) {
        default:
            return def;
        case 'A':
        case 'C':
            return 0;
        case 'H':
            return 16;
        case 'D':
            return 10;
        case 'O':
            return 8;
        case 'B':
            return 2;
        } // case
    } // getBase

    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        const MintArg& arg1 = args[1];
        const MintArg& arg2 = args[2];
        const MintArg& arg3 = args[3];

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
            interp.returnString(is_active, MintString(1, static_cast<mintchar_t>(num)));
        } // else
    } // operator()
}; // bcPrim

class binaryOpPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        const MintArg& arg1 = args[1];
        MintString prefix(arg1.getIntPrefix(10));
        int a1 = arg1.getIntValue(10);
        int a2 = args[2].getIntValue(10);
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

class gtPrim : public MintPrim {
    void operator()(Mint& interp, bool is_active, const MintArgList& args) {
        int a1 = args[1].getIntValue(10);
        int a2 = args[2].getIntValue(10);
        if (a1 > a2) {
            interp.returnString(is_active, args[3].getValue());
        } else {
            interp.returnString(is_active, args[4].getValue());
        } // if
    } // operator()
}; // gtPrim

void registerMthPrims(Mint& interp) {
    interp.addPrim("bc", new bcPrim);
    interp.addPrim("++", new addPrim);
    interp.addPrim("--", new subPrim);
    interp.addPrim("**", new mulPrim);
    interp.addPrim("//", new divPrim);
    interp.addPrim("%%", new modPrim);
    interp.addPrim("||", new iorPrim);
    interp.addPrim("&&", new andPrim);
    interp.addPrim("^^", new xorPrim);
    interp.addPrim("g?", new gtPrim);
} // registerMthPrims

// EOF
