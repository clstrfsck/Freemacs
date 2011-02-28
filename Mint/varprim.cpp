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
