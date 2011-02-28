#ifndef _MINTPRIM_H
#define _MINTPRIM_H

class Mint;
class MintArgList;

#include "mintstring.h"

class MintPrim {

public:
    virtual ~MintPrim() { }
    virtual void operator()(Mint& interp, bool is_active, const MintArgList& args) = 0;

}; // MintPrim


class MintVar {

public:
    virtual ~MintVar() { }
    virtual MintString getVal(Mint& interp) const = 0;
    virtual void setVal(Mint& interp, const MintString& val) = 0;

}; // MintVar

#endif // _MINTPRIM_H

// EOF
