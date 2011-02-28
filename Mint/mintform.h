#ifndef _MINTFORM_H
#define _MINTFORM_H

#include <iostream>
#include <algorithm>

#include "minttype.h"

#undef max
#undef min

class MintForm : public MintString {
public:
    MintForm() : _index(0) { }
    MintForm(const MintString& str) : MintString(str), _index(0) { }

    void setPos(mintcount_t n) {
        _index = std::min(n, size());
    } // setPos

    mintcount_t getPos() const {
        return std::min(_index, size());
    } // getPos

    bool atEnd() const {
        return (_index >= size());
    }

    MintString getN(int n) {
        _index = std::min(_index, size());
        mintcount_t len = std::min(size() - _index, static_cast<mintcount_t>(std::max(0, n)));
        MintString s(begin() + _index, begin() + _index + len);
        _index += len;
        return s;
    } // getN

    MintString get() {
        _index = std::min(_index, size());
        MintString s(begin() + _index, end());
        return s;
    } // get

private:
    mintcount_t _index;
}; // MintForm

#endif // _MINTFORM_H

// EOF
